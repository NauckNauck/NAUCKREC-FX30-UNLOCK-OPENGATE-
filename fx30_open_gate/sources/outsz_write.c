/* FX30 outsz helper: changes the movie-size-table entry for the stock 4K class
 * from 3840x2160 (000f7008) to 3840x2560 (000f000a), at CP 0x155d444.
 * RAM-only, volatile. Verifies the stock bytes first; writes only if they match. */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);
#define CP_PAGE 4096U
#define SITE    0x155d444UL
#define PAGE    (SITE & ~0xfffUL)
#define OFF     (SITE & 0xfffUL)
static long sc6(long n,long a,long b,long c,long d,long e,long f){
    register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
    register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
    asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++; long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }
static const u8 OLD[4]={0x00,0x0f,0x70,0x08};
static const u8 NEW[4]={0x00,0x0f,0x00,0x0a};
__attribute__((constructor)) static void go(void){
    wr("/tmp_wbshd/osz_start","S");
    u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
    if(!phys){wr("/tmp_wbshd/osz_alloc","A");return;}
    if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){wr("/tmp_wbshd/osz_phys","P");cam_dmm_free(handle);return;}
    i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0); /* O_RDWR|O_SYNC */
    if(fd<0){wr("/tmp_wbshd/osz_devmem","D");cam_dmm_free(handle);return;}
    u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
    i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead)); /* PROT_READ|WRITE, MAP_SHARED */
    if(m<0){wr("/tmp_wbshd/osz_mmap","M");sc6(57,fd,0,0,0,0,0);cam_dmm_free(handle);return;}
    u8 *b=(u8*)m+lead;
    /* read the page */
    if(sen_dma_transfer(0,PAGE,phys,CP_PAGE)){wr("/tmp_wbshd/osz_read","R");goto out;}
    for(int k=0;k<4;k++) if(b[OFF+k]!=OLD[k]){wr("/tmp_wbshd/osz_mismatch","!");
        /* dump the actual 4 bytes for the log */
        long o=sc6(56,-100,(long)"/tmp_wbshd/osz_actual",0x241,0644,0,0); if(o>=0){sc6(64,o,(long)(b+OFF),4,0,0,0);sc6(57,o,0,0,0,0,0);}
        goto out;}
    for(int k=0;k<4;k++) b[OFF+k]=NEW[k];
    if(sen_dma_transfer(1,phys,PAGE,CP_PAGE)){wr("/tmp_wbshd/osz_write","W");goto out;}
    /* read back */
    if(sen_dma_transfer(0,PAGE,phys,CP_PAGE)){wr("/tmp_wbshd/osz_rb","B");goto out;}
    int ok=1; for(int k=0;k<4;k++) if(b[OFF+k]!=NEW[k]) ok=0;
    wr(ok?"/tmp_wbshd/osz_ok":"/tmp_wbshd/osz_bad", ok?"OK":"BAD");
out:
    sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); wr("/tmp_wbshd/osz_done","D");
}
