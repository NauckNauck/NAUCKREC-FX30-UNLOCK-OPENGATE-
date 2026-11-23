/* FX30 Open Gate — NATIVE DCI stretched to 5952x3968.
 * Camera must ALREADY be in DCI (modo 16) before injection. S0 untouched (mode 16 already = column 9).
 * Only the HEIGHT changes: 3140 -> 3968. RAM only. */
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
struct p16 { u32 addr; u16 oldv; u16 newv; };
static const struct p16 REC[] = {
  {0x00c76566UL, 3174, 4002},
  {0x00c7711eUL, 3174, 4002},
  {0x00c7656eUL, 3148, 3976},
  {0x00c77126UL, 3148, 3976},
  {0x00c76576UL, 3140, 3968},
  {0x00c7712eUL, 3140, 3968},
  {0x00c76cdcUL, 3148, 3968},
  {0x00c77894UL, 3148, 3968},
  {0x00c76ce4UL, 5275, 6655},
  {0x00c7789cUL, 5275, 6655},
  {0x00c76ce6UL, 5272, 6654},
  {0x00c7789eUL, 5272, 6654},
  {0x00c76ebeUL, 510, 0},
  {0x00c77a76UL, 510, 0},
};
#define NREC (sizeof(REC)/sizeof(REC[0]))
static const struct p16 IP[] = {
  /* Entry 16 (DCI 24p) */
  {0x020daa0aUL, 3140, 3968}, {0x020daa9aUL, 510, 0}, {0x020daae6UL, 1600, 2020},
  /* Entry 17 (DCI 25/30p) */
  {0x020dab6aUL, 3140, 3968}, {0x020dabfaUL, 510, 0}, {0x020dac46UL, 1600, 2020},
};
#define NIP (sizeof(IP)/sizeof(IP[0]))
static const struct p16 SC[] = {
  {0x0155d446, 0x0870, 0x0a00},   /* outsz 2560 */
};
#define NSC (sizeof(SC)/sizeof(SC[0]))
static const struct p32_st { u32 addr; u32 oldv; u32 newv; } DMM[] = {
  {0x2080860,0x1788700,0x01bf0000},{0x2080874,0x1788700,0x01bf0000},
};
#define NDMM (sizeof(DMM)/sizeof(DMM[0]))
struct p8 { u32 addr; u8 oldv; u8 newv; };
static const struct p8 PB[] = {
  /* matrix column 9 -> 3968 active (1990 = 0x07c6), VBLANK 177 = 0x00b1 */
  {0x01038d65UL, 0x4f, 0xb1}, {0x01038d79UL, 0x02, 0x00},
  {0x01038d8dUL, 0x28, 0xc6}, {0x01038da1UL, 0x06, 0x07},
  {0x01038db5UL, 0x0b, 0x03}, {0x01038dc9UL, 0x20, 0x28},
  {0x01038df1UL, 0x27, 0x2f},
  /* active P2S buffer */
  {0x01123dfeUL, 0xe7, 0xb1}, {0x01123e02UL, 0x01, 0x00},
  {0x01123e06UL, 0x90, 0xc6}, {0x01123e0aUL, 0x06, 0x07},
  {0x01123e1eUL, 0x09, 0x03}, {0x01123e26UL, 0x22, 0x28},
  {0x01123e4aUL, 0x29, 0x2f},
};
#define NPB (sizeof(PB)/sizeof(PB[0]))
static u64 pages[32]; static u32 npages;
static void add_page(u64 a){ u64 pg=a&~0xfffUL; for(u32 i=0;i<npages;i++) if(pages[i]==pg) return; if(npages<32) pages[npages++]=pg; }
__attribute__((constructor)) static void go(void){
  const char *mk[]={"/tmp_wbshd/dci3_ok","/tmp_wbshd/dci3_bad","/tmp_wbshd/dci3_done","/tmp_wbshd/dci3_start","/tmp_wbshd/dci3_mis"};
  for(unsigned i=0;i<5;i++) sc6(35,-100,(long)mk[i],0,0,0,0);
  wr("/tmp_wbshd/dci3_start","S");
  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/dci3_alloc","A"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/dci3_devmem","D"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 mp=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(mp<0){ wr("/tmp_wbshd/dci3_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
  u8 *b=(u8*)mp+lead;
  for(u32 i=0;i<NREC;i++) add_page(REC[i].addr);
  for(u32 i=0;i<NIP;i++) add_page(IP[i].addr);
  for(u32 i=0;i<NSC;i++) add_page(SC[i].addr);
  for(u32 i=0;i<NDMM;i++) add_page(DMM[i].addr);
  for(u32 i=0;i<NPB;i++) add_page(PB[i].addr);
  int ok=1;
  for(u32 p=0;p<npages && ok;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=REC[i].oldv && cv!=REC[i].newv){ ok=0; wr("/tmp_wbshd/dci3_mis","!"); break; } }
    if(!ok) break;
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL, v=REC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NIP;i++) if((IP[i].addr&~0xfffUL)==pages[p]){
      u32 o=IP[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=IP[i].oldv && cv!=IP[i].newv){ ok=0; wr("/tmp_wbshd/dci3_ipmis","!"); break; }
      b[o]=(u8)IP[i].newv; b[o+1]=(u8)(IP[i].newv>>8); }
    if(!ok) break;
    for(u32 i=0;i<NSC;i++) if((SC[i].addr&~0xfffUL)==pages[p]){
      u32 o=SC[i].addr&0xfffUL, v=SC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NDMM;i++) if((DMM[i].addr&~0xfffUL)==pages[p]){
      u32 o=DMM[i].addr&0xfffUL, v=DMM[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
    for(u32 i=0;i<NPB;i++) if((PB[i].addr&~0xfffUL)==pages[p]){
      u32 o=PB[i].addr&0xfffUL; b[o]=PB[i].newv; }
    if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
  }
  wr(ok?"/tmp_wbshd/dci3_ok":"/tmp_wbshd/dci3_bad", ok?"OK":"BAD");
  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/dci3_done","D");
}
