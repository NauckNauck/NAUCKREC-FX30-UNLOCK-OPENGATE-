/* FX30 READ PROBE v2 — 100% Read-Only Live CP State Inspector + USB-in-Movie IMG INIT Cure
 * Output layout in /tmp_wbshd/probe_dump.bin (1024 bytes):
 *   [  0..351] 352 B: ImagerDrvParam Mode 6 (0x020d9be4)
 *   [512..639] 128 B: Active P2S Sensor Buffer (0x01123dea)
 *   [640..671]  32 B: Movie Size Table / outsz (0x0155d438)
 *   [672..691]  20 B: S2 Crop Table Mode 6 (0x00d10e5c)
 *   [692..703]  12 B: S4 Geometry Table Mode 6 (0x00cb57f0)
 *   [704..719]  16 B: S5/S1/S6 Stream-0 Window Mode 6 (0x00e02880)
 *   [720..735]  16 B: Mode 2 L scaler block +0x780..+0x78f (0x00c79bc0)
 *   [736..743]   8 B: LCD fit word (0x001e2d38..0x001e2d3f)
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 s, u32 *h) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 h) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);
#define CP_PAGE 4096U

static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++;
  long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){ sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }

static void cp_read_slice(u64 cp_addr, u32 len, u8 *dst, u64 phys, u8 *b){
  u64 pg = cp_addr & ~0xfffUL;
  u32 off = (u32)(cp_addr & 0xfffUL);
  if(sen_dma_transfer(0, pg, phys, CP_PAGE) == 0){
    for(u32 i=0; i<len; i++) dst[i] = b[off + i];
  }
}

__attribute__((constructor)) static void go(void){
  const char *mk[]={"/tmp_wbshd/pb_start","/tmp_wbshd/pb_ok","/tmp_wbshd/pb_bad","/tmp_wbshd/pb_done"};
  for(unsigned i=0;i<4;i++) sc6(35,-100,(long)mk[i],0,0,0,0);
  wr("/tmp_wbshd/pb_start","S");
  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/pb_bad","ALLOC"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/pb_bad","DEVMEM"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 mp=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(mp<0){ wr("/tmp_wbshd/pb_bad","MMAP"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
  u8 *b=(u8*)mp+lead;

  u8 outbuf[1024];
  for(int i=0;i<1024;i++) outbuf[i]=0;

  cp_read_slice(0x020d9be4UL, 352, outbuf + 0,   phys, b);
  cp_read_slice(0x01123deaUL, 128, outbuf + 512, phys, b);
  cp_read_slice(0x0155d438UL,  32, outbuf + 640, phys, b);
  cp_read_slice(0x00d10e5cUL,  20, outbuf + 672, phys, b);
  cp_read_slice(0x00cb57f0UL,  12, outbuf + 692, phys, b);
  cp_read_slice(0x00e02880UL,  16, outbuf + 704, phys, b);
  cp_read_slice(0x00c79bc0UL,  16, outbuf + 720, phys, b);
  cp_read_slice(0x001e2d38UL,   8, outbuf + 736, phys, b);

  long dfd = sc6(56,-100,(long)"/tmp_wbshd/probe_dump.bin",0x241,0644,0,0);
  if(dfd>=0){
    sc6(64,dfd,(long)outbuf,1024,0,0,0);
    sc6(57,dfd,0,0,0,0,0);
    wr("/tmp_wbshd/pb_ok","OK");
  } else {
    wr("/tmp_wbshd/pb_bad","FILE");
  }

  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/pb_done","D");
}
