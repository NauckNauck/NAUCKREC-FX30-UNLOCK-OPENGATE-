/*
 * dump.c — minimal physical-memory dumper .so for the Sony FX30 (no libc).
 *
 * Constructor only writes a marker, mkdirs the output dir, then copies
 * /dev/mem[PHY_START .. PHY_START+PHY_LEN) to DUMP_PATH. No stack buffer
 * (an anonymous mmap holds the 16 KiB chunk) so the loader's stack is untouched.
 */

#define PHY_START 0x01000000UL
#define PHY_LEN   0x01000000UL
#define CHUNK     0x00004000UL
#define DUMP_PATH "../../tmp_wbshd/dump.bin"

#define SYS_mkdirat 34
#define SYS_openat  56
#define SYS_close   57
#define SYS_read    63
#define SYS_write   64
#define SYS_lseek   62
#define SYS_mmap    222
#define SYS_munmap  215

#define AT_FDCWD        (-100)
#define O_RDONLY        0
#define O_WRONLY_CREAT  0x241
#define SEEK_SET        0
#define PROT_RW         3
#define MAP_ANON        0x22

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

static void mark(const char *path, const char *msg, unsigned long n) {
    long fd = sc6(SYS_openat, AT_FDCWD, (long)path, O_WRONLY_CREAT, 0644, 0, 0);
    if (fd >= 0) {
        sc6(SYS_write, fd, (long)msg, (long)n, 0, 0, 0);
        sc6(SYS_close, fd, 0, 0, 0, 0, 0);
    }
}

__attribute__((constructor))
static void do_dump(void) {
    char mk = 'R';
    mark("/tmp/mk_root", &mk, 1);
    sc6(SYS_mkdirat, AT_FDCWD, (long)"/tmp/wbshd", 0755, 0, 0, 0);
    mark("../../tmp_wbshd/mk_rel", &mk, 1);

    long fd = sc6(SYS_openat, AT_FDCWD, (long)"/dev/mem", O_RDONLY, 0, 0, 0);
    if (fd < 0) { mark("../../tmp_wbshd/mk_rel", "E1", 2); return; }

    long out = sc6(SYS_openat, AT_FDCWD, (long)DUMP_PATH, O_WRONLY_CREAT, 0644, 0, 0);
    if (out < 0) { mark("../../tmp_wbshd/mk_rel", "E2", 2); sc6(SYS_close, fd, 0, 0, 0, 0, 0); return; }

    long buf = sc6(SYS_mmap, 0, 0x4000, PROT_RW, MAP_ANON, -1, 0);
    if (buf < 0) { mark("../../tmp_wbshd/mk_rel", "E3", 2); sc6(SYS_close, fd, 0, 0, 0, 0, 0); sc6(SYS_close, out, 0, 0, 0, 0, 0); return; }

    sc6(SYS_lseek, fd, (long)PHY_START, SEEK_SET, 0, 0, 0);
    unsigned long done = 0;
    while (done < PHY_LEN) {
        long n = sc6(SYS_read, fd, buf, (long)CHUNK, 0, 0, 0);
        if (n <= 0) break;
        long off = 0;
        while (off < n) {
            long w = sc6(SYS_write, out, buf + off, n - off, 0, 0, 0);
            if (w <= 0) { done = PHY_LEN; break; }
            off += w;
        }
        done += n;
    }
    mark("../../tmp_wbshd/mk_rel", "D", 1);
    sc6(SYS_munmap, buf, 0x4000, 0, 0, 0, 0);
    sc6(SYS_close, out, 0, 0, 0, 0, 0);
    sc6(SYS_close, fd, 0, 0, 0, 0, 0);
}
