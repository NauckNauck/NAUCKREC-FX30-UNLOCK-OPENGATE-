/* FX30 Open Gate Phase 3c:
 * - Mode 2 records (0xc79440 / 0xc79ff8) with correct height fields & v-scaler (no h-phase corruption)
 * - Scalares S1 (0xe02888), S3 (0xe03298), S9 (0xe03290), S7 (0xe0311a)
 * - Outsz (0x155d446 = 2560) & DMM buffers (29.29 MB)
 * - IMX684 Hardware Sensor: Node 0 continuous clock + 6 physical height registers
 * - LCD patch disabled (isolated display)
 * 100% Volatile RAM.
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

static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/p3_start","/tmp_wbshd/p3_ok","/tmp_wbshd/p3_bad",
                   "/tmp_wbshd/p3_rec_mismatch","/tmp_wbshd/p3_sc_mismatch",
                   "/tmp_wbshd/p3_pb_mismatch","/tmp_wbshd/p3_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

struct p16 { u32 addr; u16 oldv; u16 newv; };
static const struct p16 REC[] = {
  /* Mode 2 Left (0xc79440) */
  {0x00c79446UL, 3382, 4002},   /* bounding height */
  {0x00c7944eUL, 3356, 3976},   /* guardband height */
  {0x00c79456UL, 3348, 3968},   /* active height */
  {0x00c79bbcUL, 3356, 3968},   /* scaler in-h */
  {0x00c79bc4UL, 5625, 6655},   /* luma vertical step (3968/2442 * 4096) */
  {0x00c79d9eUL,  406,    0},   /* crop V offset */

  /* Mode 2 Right (0xc79ff8) */
  {0x00c79ffeUL, 3382, 4002},   /* bounding height */
  {0x00c7a006UL, 3356, 3976},   /* guardband height */
  {0x00c7a00eUL, 3348, 3968},   /* active height */
  {0x00c7a774UL, 3356, 3968},   /* scaler in-h */
  {0x00c7a77cUL, 5625, 6655},   /* luma vertical step */
  {0x00c7a956UL,  406,    0},   /* crop V offset */
};
#define NREC (sizeof(REC)/sizeof(REC[0]))

static const struct p16 SC[] = {
  {0x00e02888UL, 3348, 3968},     /* S1 active height of mode 6 */
  {0x00e03290UL,   41,   47},     /* S9 sensor reg 0x19 (column 11 value) */
  {0x00e03298UL, 1704, 2020},     /* S3 line periods of mode 6 */
  {0x00e0311aUL,  406,    0},     /* S7 lens shading V-offset of mode 6 */
  {0x0155d446UL, 0x0870, 0x0a00}, /* outsz height 2160 -> 2560 */
};
#define NSC (sizeof(SC)/sizeof(SC[0]))

static const struct p32_st { u32 addr; u32 oldv; u32 newv; } DMM[] = {
  {0x02080860UL, 0x01788700UL, 0x01bf0000UL},
  {0x02080874UL, 0x01788700UL, 0x01bf0000UL},
};
#define NDMM (sizeof(DMM)/sizeof(DMM[0]))

/* Hardware sensor registers (Node 0 continuous streaming + physical line readout) */
struct pb { u32 addr; u8 oldv; u8 newv; };
static const struct pb PB[] = {
  {0x01038d2bUL, 0x01, 0x0f},     /* Node 0: continuous video streaming */
  {0x01123f7aUL, 0x02, 0x82},     /* IMX684 Node 40 (Reg 0x00d7) */
  {0x01123f86UL, 0x55, 0x22},     /* IMX684 Node 42 (Reg 0x00da) */
  {0x01123faaUL, 0x2a, 0x12},     /* IMX684 Node 44 (Reg 0x00e3) */
  {0x01123fe6UL, 0x55, 0x6a},     /* IMX684 Node 51 (Reg 0x0438) */
  {0x011241daUL, 0x88, 0x22},     /* IMX684 Node 143 (Reg 0x068a) */
  {0x011241deUL, 0x68, 0xda},     /* IMX684 Node 144 (Reg 0x0a80) */
};
#define NPB (sizeof(PB)/sizeof(PB[0]))

static u64 pages[32]; static u32 npages;
static void add_page(u64 a){ u64 pg=a&~0xfffUL; for(u32 i=0;i<npages;i++) if(pages[i]==pg) return; if(npages<32) pages[npages++]=pg; }

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/p3_start","S");

  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/p3_alloc","A"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/p3_devmem","D"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 mp=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(mp<0){ wr("/tmp_wbshd/p3_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
  u8 *b=(u8*)mp+lead;

  for(u32 i=0;i<NREC;i++) add_page(REC[i].addr);
  for(u32 i=0;i<NSC;i++)  add_page(SC[i].addr);
  for(u32 i=0;i<NDMM;i++) add_page(DMM[i].addr);
  for(u32 i=0;i<NPB;i++)  add_page(PB[i].addr);

  int ok=1;
  /* Phase 1: Verify stock checks across all pages */
  for(u32 p=0;p<npages && ok;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=REC[i].oldv && cv!=REC[i].newv){ ok=0; wr("/tmp_wbshd/p3_rec_mismatch","!"); break; } }
    for(u32 i=0;i<NSC;i++) if((SC[i].addr&~0xfffUL)==pages[p]){
      u32 o=SC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=SC[i].oldv && cv!=SC[i].newv){ ok=0; wr("/tmp_wbshd/p3_sc_mismatch","!"); break; } }
    for(u32 i=0;i<NPB;i++) if((PB[i].addr&~0xfffUL)==pages[p]){
      u32 o=PB[i].addr&0xfffUL; u8 cv=b[o];
      if(cv!=PB[i].oldv && cv!=PB[i].newv){ ok=0; wr("/tmp_wbshd/p3_pb_mismatch","!"); break; } }
  }

  /* Phase 2: Apply patches if all checks passed */
  if(ok){
    for(u32 p=0;p<npages;p++){
      if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
        u32 o=REC[i].addr&0xfffUL, v=REC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
      for(u32 i=0;i<NSC;i++) if((SC[i].addr&~0xfffUL)==pages[p]){
        u32 o=SC[i].addr&0xfffUL, v=SC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
      for(u32 i=0;i<NDMM;i++) if((DMM[i].addr&~0xfffUL)==pages[p]){
        u32 o=DMM[i].addr&0xfffUL, v=DMM[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
      for(u32 i=0;i<NPB;i++) if((PB[i].addr&~0xfffUL)==pages[p]){
        u32 o=PB[i].addr&0xfffUL; b[o]=PB[i].newv; }
      if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
    }
  }

  wr(ok?"/tmp_wbshd/p3_ok":"/tmp_wbshd/p3_bad", ok?"OK":"BAD");
  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/p3_done","D");
}
