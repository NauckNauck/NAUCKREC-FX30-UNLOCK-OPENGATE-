/* FX30 Open Gate — DCI column 9 stretched to 5952x3968. Width stays 5952 (video-native).
 * Column 9 differs from column 1 ONLY in vertical registers (VBLANK/active/prescaler/S9). */
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
  {0x00c79446UL, 3382, 4002},
  {0x00c79ffeUL, 3382, 4002},
  {0x00c7944eUL, 3356, 3976},
  {0x00c7a006UL, 3356, 3976},
  {0x00c79456UL, 3348, 3968},
  {0x00c7a00eUL, 3348, 3968},
  {0x00c79bbcUL, 3356, 3968},
  {0x00c7a774UL, 3356, 3968},
  {0x00c79bc4UL, 5625, 6655},
  {0x00c7a77cUL, 5625, 6655},
  {0x00c79bc6UL, 5623, 6654},
  {0x00c7a77eUL, 5623, 6654},
  {0x00c79d9eUL, 406, 0},
  {0x00c7a956UL, 406, 0},
};
#define NREC (sizeof(REC)/sizeof(REC[0]))
static const struct p16 IP[] = {
  {0x020d96b8UL, 5952, 5952},
  {0x020d96caUL, 3348, 3968},
  {0x020d975aUL, 406, 0},
  {0x020d97a6UL, 1704, 2020},
  {0x020d9c38UL, 5952, 5952},
  {0x020d9c4aUL, 3348, 3968},
  {0x020d9cdaUL, 406, 0},
  {0x020d9d26UL, 1704, 2020},
  {0x020db238UL, 5952, 5952},
  {0x020db24aUL, 3140, 3968},
  {0x020db2daUL, 0, 0},
  {0x020db326UL, 0, 2020},
};
#define NIP (sizeof(IP)/sizeof(IP[0]))
static const struct p16 SC[] = {
  {0x00e02888, 3348, 3968},   /* S1 active height mode 6 */
  {0x00e03298, 1704, 2020},   /* S3 row 0 */
  {0x00e032a6, 1609, 2020},   /* S3 row 1 */
  {0x00e032b4, 1704, 2020},   /* S3 row 2 */
  {0x0155d446, 0x0870, 0x0a00}, /* outsz 2560 */
};
#define NSC (sizeof(SC)/sizeof(SC[0]))
static const struct p32_st { u32 addr; u32 oldv; u32 newv; } DMM[] = {
  {0x2080860,0x1788700,0x01bf0000},{0x2080874,0x1788700,0x01bf0000},
};
#define NDMM (sizeof(DMM)/sizeof(DMM[0]))
struct p8 { u32 addr; u8 oldv; u8 newv; };
static const struct p8 PB[] = {
  {0x01039950, 1, 9},    /* S0 -> column 9 (DCI) */
  {0x01123dfe, 0xe7, 0xb1},  /* n3 VBLANK lo */
  {0x01123e02, 0x01, 0x00},  /* n4 VBLANK hi */
  {0x01123e06, 0x90, 0xc6},  /* n5 active lo */
  {0x01123e0a, 0x06, 0x07},  /* n6 active hi */
  {0x01123e1e, 0x09, 0x03},  /* n7 prescaler */
  {0x01123e26, 0x22, 0x28},  /* n8 prescaler comp */
  {0x01123e4a, 0x29, 0x2f},  /* n10 S9 */
};
#define NPB (sizeof(PB)/sizeof(PB[0]))
static u64 pages[32]; static u32 npages;
static void add_page(u64 a){ u64 pg=a&~0xfffUL; for(u32 i=0;i<npages;i++) if(pages[i]==pg) return; if(npages<32) pages[npages++]=pg; }
__attribute__((constructor)) static void go(void){
  const char *mk[]={"/tmp_wbshd/dci_ok","/tmp_wbshd/dci_bad","/tmp_wbshd/dci_done","/tmp_wbshd/dci_start","/tmp_wbshd/dci_mis"};
  for(unsigned i=0;i<5;i++) sc6(35,-100,(long)mk[i],0,0,0,0);
  wr("/tmp_wbshd/dci_start","S");
  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/dci_alloc","A"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/dci_devmem","D"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 mp=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(mp<0){ wr("/tmp_wbshd/dci_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
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
      if(cv!=REC[i].oldv && cv!=REC[i].newv){ ok=0; wr("/tmp_wbshd/dci_mis","!"); break; } }
    if(!ok) break;
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL, v=REC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NIP;i++) if((IP[i].addr&~0xfffUL)==pages[p]){
      u32 o=IP[i].addr&0xfffUL, v=IP[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NSC;i++) if((SC[i].addr&~0xfffUL)==pages[p]){
      u32 o=SC[i].addr&0xfffUL, v=SC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NDMM;i++) if((DMM[i].addr&~0xfffUL)==pages[p]){
      u32 o=DMM[i].addr&0xfffUL, v=DMM[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
    for(u32 i=0;i<NPB;i++) if((PB[i].addr&~0xfffUL)==pages[p]){
      u32 o=PB[i].addr&0xfffUL; b[o]=PB[i].newv; }
    if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
  }
  wr(ok?"/tmp_wbshd/dci_ok":"/tmp_wbshd/dci_bad", ok?"OK":"BAD");
  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/dci_done","D");
}
