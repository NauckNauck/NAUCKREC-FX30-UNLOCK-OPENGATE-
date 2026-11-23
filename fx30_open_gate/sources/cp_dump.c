/* Minimal read-only coprocessor dumper for the Sony FX30.
 * Uses the camera's own libsencore.so: dmm_alloc/dmm_free/sen_dma_transfer.
 * Link: zig cc -target aarch64-linux-gnu ... cp_dump.c <nflasha15/lib/libsencore.so>
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned char u8;

extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U
#define START   0x00C0000UL
#define END     0x2100000UL

static long sc6(long n, long a, long b, long c, long d, long e, long f) {
    register long x8 asm("x8") = n; register long x0 asm("x0") = a; register long x1 asm("x1") = b;
    register long x2 asm("x2") = c; register long x3 asm("x3") = d; register long x4 asm("x4") = e; register long x5 asm("x5") = f;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
    return x0;
}
static void mark(const char *p, const char *m, unsigned long n) {
    long fd = sc6(56, -100, (long)p, 0x241, 0644, 0, 0);
    if (fd >= 0) { sc6(64, fd, (long)m, (long)n, 0, 0, 0); sc6(57, fd, 0,0,0,0,0); }
}

__attribute__((constructor)) static void go(void) {
    mark("/tmp_wbshd/e_start", "S", 1);
    u32 handle = 0;
    u64 phys = cam_dmm_alloc(CP_PAGE, &handle);
    if (!phys) { mark("/tmp_wbshd/e_alloc", "A", 1); return; }
    if ((phys & 0x3f) || phys < 0x400000000UL || phys + CP_PAGE > 0x500000000UL) {
        mark("/tmp_wbshd/e_phys", "P", 1); cam_dmm_free(handle); return;
    }
    i64 fd = sc6(56, -100, (long)"/dev/mem", 0x101000, 0, 0, 0);
    if (fd < 0) { mark("/tmp_wbshd/e_devmem", "D", 1); cam_dmm_free(handle); return; }
    u64 lead = phys & 0xfff, maplen = (lead + CP_PAGE + 0xfff) & ~0xfffUL;
    i64 m = sc6(222, 0, (long)maplen, 1 /*PROT_READ*/, 1 /*MAP_SHARED*/, fd, (long)(phys - lead));
    if (m < 0) { mark("/tmp_wbshd/e_mmap", "M", 1); sc6(57, fd,0,0,0,0,0); cam_dmm_free(handle); return; }
    long out = sc6(56, -100, (long)"/tmp_wbshd/dump.bin", 0x241, 0644, 0, 0);
    if (out < 0) { mark("/tmp_wbshd/e_out", "O", 1); sc6(215, m, (long)maplen, 0,0,0,0); sc6(57, fd,0,0,0,0,0); cam_dmm_free(handle); return; }
    u8 zero[CP_PAGE]; for (u32 k = 0; k < CP_PAGE; k++) zero[k] = 0;
    u64 ok = 0, bad = 0;
    for (u64 a = START; a < END; a += CP_PAGE) {
        u32 r = sen_dma_transfer(0, a, phys, CP_PAGE);
        const u8 *src = (const u8 *)m + lead;
        if (r) { src = zero; bad++; } else ok++;
        long off = 0;
        while (off < (long)CP_PAGE) { long w = sc6(64, out, (long)(src + off), (long)CP_PAGE - off, 0,0,0); if (w <= 0) break; off += w; }
    }
    mark("/tmp_wbshd/e_done", "D", 1);
    sc6(215, m, (long)maplen, 0,0,0,0);
    sc6(57, out,0,0,0,0,0); sc6(57, fd,0,0,0,0,0);
    cam_dmm_free(handle);
    (void)ok; (void)bad;
}
