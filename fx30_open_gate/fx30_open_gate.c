/* FX30 Height-Only ISOLADO (SEM outsz, SEM dmmwb, SEM encoder)
 * Objetivo: Teste de diagnóstico óptico/geométrico puro.
 * 1. Height-Only no Modo 3 (w3=4200, w7=4168, w11=4128, w943=4168, w1199=0)
 * 2. S2 e S4 globais (502 -> 0)
 * 3. outsz: STOCK (00 0f 70 08 = 3840x2160) - INTACTO!
 * 4. dmmwb: STOCK (24.67 MB) - INTACTO!
 * 5. encoder: STOCK (2160) - INTACTO!
 * 100% Volatile RAM injection.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;

extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U

static long sc6(long n, long a, long b, long c, long d, long e, long f) {
    register long x8 asm("x8") = n; register long x0 asm("x0") = a; register long x1 asm("x1") = b;
    register long x2 asm("x2") = c; register long x3 asm("x3") = d; register long x4 asm("x4") = e; register long x5 asm("x5") = f;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
    return x0;
}

static void wr(const char *p, const char *m) {
    long n = 0;
    const char *tmp = m;
    while (*tmp++) n++;
    long fd = sc6(56, -100, (long)p, 0x241, 0644, 0, 0);
    if (fd >= 0) { sc6(64, fd, (long)m, n, 0, 0, 0); sc6(57, fd, 0, 0, 0, 0, 0); }
}

#define MODE3_L 0xc7abb0UL
#define MODE3_R 0xc7b768UL

static void patch_word(u8 *buf, u64 page_base, u64 target_addr, u16 new_val) {
    if (target_addr >= page_base && target_addr + 2 <= page_base + CP_PAGE) {
        u32 off = (u32)(target_addr - page_base);
        buf[off]     = (u8)(new_val & 0xff);
        buf[off + 1] = (u8)((new_val >> 8) & 0xff);
    }
}

__attribute__((constructor)) static void run_fx30_open_gate(void) {
    wr("/tmp_wbshd/og_start", "START");
    u32 handle = 0;
    u64 phys = cam_dmm_alloc(CP_PAGE, &handle);
    if (!phys) { wr("/tmp_wbshd/og_err_alloc", "ALLOC_FAIL"); return; }
    if ((phys & 0x3f) || phys < 0x400000000UL || phys + CP_PAGE > 0x500000000UL) {
        wr("/tmp_wbshd/og_err_phys", "PHYS_FAIL");
        cam_dmm_free(handle);
        return;
    }

    i64 fd = sc6(56, -100, (long)"/dev/mem", 0x101002, 0, 0, 0); /* O_RDWR|O_SYNC */
    if (fd < 0) { wr("/tmp_wbshd/og_err_devmem", "DEVMEM_FAIL"); cam_dmm_free(handle); return; }

    u64 lead = phys & 0xfff, maplen = (lead + CP_PAGE + 0xfffUL) & ~0xfffUL;
    i64 m = sc6(222, 0, (long)maplen, 3, 1, fd, (long)(phys - lead));
    if (m < 0) {
        wr("/tmp_wbshd/og_err_mmap", "MMAP_FAIL");
        sc6(57, fd, 0, 0, 0, 0, 0);
        cam_dmm_free(handle);
        return;
    }
    u8 *buf = (u8 *)m + lead;

    /* A. Height-Only Patch no Modo 3 */
    u64 pages[3] = { 0xc7a000UL, 0xc7b000UL, 0xc7c000UL };
    for (int p = 0; p < 3; p++) {
        u64 paddr = pages[p];
        if (sen_dma_transfer(0, paddr, phys, CP_PAGE) == 0) {
            patch_word(buf, paddr, MODE3_L + 6,    4200);
            patch_word(buf, paddr, MODE3_L + 14,   4168);
            patch_word(buf, paddr, MODE3_L + 22,   4128);
            patch_word(buf, paddr, MODE3_L + 1886, 4168);
            patch_word(buf, paddr, MODE3_L + 2398, 0);

            patch_word(buf, paddr, MODE3_R + 6,    4200);
            patch_word(buf, paddr, MODE3_R + 14,   4168);
            patch_word(buf, paddr, MODE3_R + 22,   4128);
            patch_word(buf, paddr, MODE3_R + 1886, 4168);
            patch_word(buf, paddr, MODE3_R + 2398, 0);

            sen_dma_transfer(1, phys, paddr, CP_PAGE);
        }
    }
    wr("/tmp_wbshd/og_ogsq_ok", "OGSQ_OK");

    /* B. S4 e S2 */
    u64 s4_page = 0xc36000UL;
    u32 s4_off  = 0x5d4;
    if (sen_dma_transfer(0, s4_page, phys, CP_PAGE) == 0) {
        buf[s4_off] = 0x00; buf[s4_off + 1] = 0x00;
        sen_dma_transfer(1, phys, s4_page, CP_PAGE);
    }

    u64 s2_page = 0xc9b000UL;
    u32 s2_off  = 0x308;
    if (sen_dma_transfer(0, s2_page, phys, CP_PAGE) == 0) {
        buf[s2_off] = 0x00; buf[s2_off + 1] = 0x00;
        sen_dma_transfer(1, phys, s2_page, CP_PAGE);
    }

    sc6(215, m, (long)maplen, 0, 0, 0, 0);
    sc6(57, fd, 0, 0, 0, 0, 0);
    cam_dmm_free(handle);
    wr("/tmp_wbshd/og_done", "DONE");
}
