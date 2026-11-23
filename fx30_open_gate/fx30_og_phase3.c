/* FX30 OPEN GATE 3:2 — FASE 3 DEFINITIVA
 * 100% Volatile RAM injection via USB Service Mode (Jiritsu protocol)
 * Zero writes to flash. 10-second battery pull completely restores camera to factory stock.
 */
#include "records_180_data.h"

typedef unsigned long u64;
typedef long i64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U

static long sc6(long n, long a, long b, long c, long d, long e, long f) {
    register long x8 asm("x8") = n;
    register long x0 asm("x0") = a;
    register long x1 asm("x1") = b;
    register long x2 asm("x2") = c;
    register long x3 asm("x3") = d;
    register long x4 asm("x4") = e;
    register long x5 asm("x5") = f;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
    return x0;
}

static void wr(const char *p, const char *m) {
    long n = 0;
    const char *tmp = m;
    while (*tmp++) n++;
    long fd = sc6(56, -100, (long)p, 0x241, 0644, 0, 0);
    if (fd >= 0) {
        sc6(64, fd, (long)m, n, 0, 0, 0);
        sc6(57, fd, 0, 0, 0, 0, 0);
    }
}

static inline void st16(u8 *b, u32 o, u16 v) {
    b[o] = (u8)v;
    b[o + 1] = (u8)(v >> 8);
}

static inline void st32(u8 *b, u32 o, u32 v) {
    b[o]     = (u8)v;
    b[o + 1] = (u8)(v >> 8);
    b[o + 2] = (u8)(v >> 16);
    b[o + 3] = (u8)(v >> 24);
}

__attribute__((constructor)) static void run_phase3(void) {
    wr("/tmp_wbshd/og3_start", "START");

    u32 handle = 0;
    u64 phys = cam_dmm_alloc(CP_PAGE, &handle);
    if (!phys) {
        wr("/tmp_wbshd/og3_err_alloc", "ALLOC_FAIL");
        return;
    }
    if ((phys & 0x3f) || phys < 0x400000000UL || phys + CP_PAGE > 0x500000000UL) {
        wr("/tmp_wbshd/og3_err_phys", "PHYS_FAIL");
        cam_dmm_free(handle);
        return;
    }

    i64 fd = sc6(56, -100, (long)"/dev/mem", 0x101002, 0, 0, 0);
    if (fd < 0) {
        wr("/tmp_wbshd/og3_err_mem", "MEM_FAIL");
        cam_dmm_free(handle);
        return;
    }

    u64 lead = phys & 0xfff;
    u64 maplen = (lead + CP_PAGE + 0xfffUL) & ~0xfffUL;
    i64 m = sc6(222, 0, (long)maplen, 3, 1, fd, (long)(phys - lead));
    if (m < 0) {
        wr("/tmp_wbshd/og3_err_mmap", "MMAP_FAIL");
        sc6(57, fd, 0, 0, 0, 0, 0);
        cam_dmm_free(handle);
        return;
    }
    u8 *b = (u8 *)m + lead;

    /* 1. S0: Coluna 1 (Vídeo 4K Nativo) + Nó 0 Streaming Contínuo */
    {
        /* Coluna 1 do Modo 6 */
        u32 s0_pg = 0x01039000UL;
        if (sen_dma_transfer(0, s0_pg, phys, CP_PAGE) == 0) {
            u32 o = (u32)(0x01039950UL - s0_pg);
            b[o] = 1; b[o + 1] = 0; b[o + 2] = 0; b[o + 3] = 0;
            sen_dma_transfer(1, phys, s0_pg, CP_PAGE);
        }
        /* Nó 0: Clock de Streaming Contínuo (0x0F) */
        u32 n0_pg = 0x01038000UL;
        if (sen_dma_transfer(0, n0_pg, phys, CP_PAGE) == 0) {
            u32 o = (u32)(0x01038d2bUL - n0_pg);
            b[o] = 0x0f;
            sen_dma_transfer(1, phys, n0_pg, CP_PAGE);
        }
        wr("/tmp_wbshd/og3_s0_ok", "S0_OK");
    }

    /* 2. S3: Temporizador Vertical de Readout (1704 -> 2020) + S7 (Crop V offset -> 0) */
    {
        u32 s3_pg = 0x00e03000UL;
        if (sen_dma_transfer(0, s3_pg, phys, CP_PAGE) == 0) {
            /* S7: LC Table V Crop Offset (502 -> 0) */
            st16(b, 0x0f0, 0);

            /* S3: Line Periods / Readout Length (1704 -> 2020) */
            st16(b, 0x298, 2020); /* 24p NTSC principal */
            st16(b, 0x2b4, 2020);
            st16(b, 0x2c2, 2020);
            st16(b, 0x2d0, 2020);
            st16(b, 0x2de, 2020);

            sen_dma_transfer(1, phys, s3_pg, CP_PAGE);
            wr("/tmp_wbshd/og3_s3_ok", "S3_OK");
        }
    }

    /* 3. S1: Stream 0 Active Height (3348 -> 3968) */
    {
        u32 s1_pg = 0x00e02000UL;
        if (sen_dma_transfer(0, s1_pg, phys, CP_PAGE) == 0) {
            st16(b, 0x7f0, 3968);
            sen_dma_transfer(1, phys, s1_pg, CP_PAGE);
            wr("/tmp_wbshd/og3_s1_ok", "S1_OK");
        }
    }

    /* 4. S2 & S4: Offsets Globais de Crop e Geometria (502 -> 0) */
    {
        /* S4: 0xc365d4 */
        u32 s4_pg = 0x00c36000UL;
        if (sen_dma_transfer(0, s4_pg, phys, CP_PAGE) == 0) {
            st16(b, 0x5d4, 0);
            sen_dma_transfer(1, phys, s4_pg, CP_PAGE);
        }
        /* S2: 0xc9b308 */
        u32 s2_pg = 0x00c9b000UL;
        if (sen_dma_transfer(0, s2_pg, phys, CP_PAGE) == 0) {
            st16(b, 0x308, 0);
            sen_dma_transfer(1, phys, s2_pg, CP_PAGE);
        }
        wr("/tmp_wbshd/og3_s2s4_ok", "S2S4_OK");
    }

    /* 5. Os 180 Campos dos Records de 3000 Bytes (Modo 3 Left e Right) */
    {
        u64 base_l = 0x00c7abb0UL;
        u64 base_r = 0x00c7b768UL;
        u64 rec_pages[3] = {0x00c7a000UL, 0x00c7b000UL, 0x00c7c000UL};

        int ok = 1;
        for (int p = 0; p < 3; p++) {
            u64 pg = rec_pages[p];
            if (sen_dma_transfer(0, pg, phys, CP_PAGE) != 0) {
                ok = 0;
                break;
            }

            /* Aplica os campos correspondentes para Left e Right */
            for (int i = 0; i < 180; i++) {
                u16 off = DIFFS_180[i].offset;

                /* Canal Left */
                u64 addr_l = base_l + off;
                if (addr_l >= pg && addr_l + 2 <= pg + CP_PAGE) {
                    st16(b, (u32)(addr_l - pg), DIFFS_180[i].val_left);
                }

                /* Canal Right */
                u64 addr_r = base_r + off;
                if (addr_r >= pg && addr_r + 2 <= pg + CP_PAGE) {
                    st16(b, (u32)(addr_r - pg), DIFFS_180[i].val_right);
                }
            }

            if (sen_dma_transfer(1, phys, pg, CP_PAGE) != 0) {
                ok = 0;
                break;
            }
        }
        wr(ok ? "/tmp_wbshd/og3_records_ok" : "/tmp_wbshd/og3_records_fail", ok ? "REC_OK" : "REC_FAIL");
    }

    /* 6. ISP Scaler Outsz: 3840 x 2560 (0x155d446 = 0x0a00) */
    {
        u32 outsz_pg = 0x0155d000UL;
        if (sen_dma_transfer(0, outsz_pg, phys, CP_PAGE) == 0) {
            st16(b, 0x446, 0x0a00);
            sen_dma_transfer(1, phys, outsz_pg, CP_PAGE);
            wr("/tmp_wbshd/og3_outsz_ok", "OUTSZ_OK");
        }
    }

    /* 7. DMM Buffers: 29.29 MB (0x01bf0000) */
    {
        u32 dmm_pg = 0x02080000UL;
        if (sen_dma_transfer(0, dmm_pg, phys, CP_PAGE) == 0) {
            u32 nv = 0x01bf0000;
            st32(b, 0x860, nv);
            st32(b, 0x874, nv);
            sen_dma_transfer(1, phys, dmm_pg, CP_PAGE);
            wr("/tmp_wbshd/og3_dmm_ok", "DMM_OK");
        }
    }

    /* 8. Display Driver (LCD Preview 3:2 sem letterbox) */
    {
        /* 0x1e2d3c = 0 */
        u32 lcd1_pg = 0x001e2000UL;
        if (sen_dma_transfer(0, lcd1_pg, phys, CP_PAGE) == 0) {
            st32(b, 0xd3c, 0);
            sen_dma_transfer(1, phys, lcd1_pg, CP_PAGE);
        }

        /* Página 0x00de7000: pares de geometria */
        u32 lcd2_pg = 0x00de7000UL;
        if (sen_dma_transfer(0, lcd2_pg, phys, CP_PAGE) == 0) {
            st32(b, 0x91c, 0x00000000UL);
            st32(b, 0x920, 0x01e00280UL);
            st32(b, 0x994, 0x00000000UL);
            st32(b, 0x998, 0x01e00280UL);
            st32(b, 0xa0c, 0x00000000UL);
            st32(b, 0xa10, 0x01e00280UL);
            sen_dma_transfer(1, phys, lcd2_pg, CP_PAGE);
        }
        wr("/tmp_wbshd/og3_lcd_ok", "LCD_OK");
    }

    sc6(215, m, (long)maplen, 0, 0, 0, 0);
    sc6(57, fd, 0, 0, 0, 0, 0);
    cam_dmm_free(handle);

    wr("/tmp_wbshd/og3_done", "DONE");
}
