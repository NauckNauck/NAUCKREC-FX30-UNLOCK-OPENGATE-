/* FX30 Open Gate Phase 3: correct Mode 2 records + corrected S1 + scalars. RAM only. */
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
  {0x00c79bc8UL, 3963, 4104},
  {0x00c7a780UL, 31591, 4104},
  {0x00c79d9eUL, 406, 0},
  {0x00c7a956UL, 406, 0},
};
#define NREC (sizeof(REC)/sizeof(REC[0]))
static const struct p16 SC[] = {
  {0xe02888, 3348, 3968},   /* S1 active height of mode 6 (corrected!) */
  {0xe03298, 1704, 2020},   /* S3 line periods */
  {0xe030f0, 502, 0},       /* S7 */
  {0xc365d4, 502, 0},       /* S4 */
  {0xc9b308, 502, 0},       /* S2 */
  {0x155d446, 0x0870, 0x0a00}, /* outsz */
};
#define NSC (sizeof(SC)/sizeof(SC[0]))
static const struct p32_st { u32 addr; u32 oldv; u32 newv; } DMM[] = {
  {0x2080860,0x1788700,0x01bf0000},{0x2080874,0x1788700,0x01bf0000},
};
#define NDMM (sizeof(DMM)/sizeof(DMM[0]))
static const struct p32_st LCD[] = {
  {0x1e2d3c,3,0},{0xde791c,0x00260000,0},{0xde7920,0x01940280,0x01e00280},
  {0xde7994,0x00260000,0},{0xde7998,0x01940280,0x01e00280},
  {0xde7a0c,0x00260000,0},{0xde7a10,0x01940280,0x01e00280},
};
#define NLCD (sizeof(LCD)/sizeof(LCD[0]))
static u64 pages[32]; static u32 npages;
static void add_page(u64 a){ u64 pg=a&~0xfffUL; for(u32 i=0;i<npages;i++) if(pages[i]==pg) return; if(npages<32) pages[npages++]=pg; }
__attribute__((constructor)) static void go(void){
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
  for(u32 i=0;i<NSC;i++) add_page(SC[i].addr);
  for(u32 i=0;i<NDMM;i++) add_page(DMM[i].addr);
  for(u32 i=0;i<NLCD;i++) add_page(LCD[i].addr);
  int ok=1;
  for(u32 p=0;p<npages && ok;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=REC[i].oldv && cv!=REC[i].newv){ ok=0; wr("/tmp_wbshd/p3_rec_mismatch","!"); break; } }
    if(!ok) break;
    for(u32 i=0;i<NREC;i++) if((REC[i].addr&~0xfffUL)==pages[p]){
      u32 o=REC[i].addr&0xfffUL, v=REC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
    for(u32 i=0;i<NSC;i++) if((SC[i].addr&~0xfffUL)==pages[p]){
      u32 o=SC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=SC[i].oldv && cv!=SC[i].newv){ ok=0; wr("/tmp_wbshd/p3_sc_mismatch","!"); break; }
      b[o]=(u8)SC[i].newv; b[o+1]=(u8)(SC[i].newv>>8); }
    if(!ok) break;
    for(u32 i=0;i<NDMM;i++) if((DMM[i].addr&~0xfffUL)==pages[p]){
      u32 o=DMM[i].addr&0xfffUL, v=DMM[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
    for(u32 i=0;i<NLCD;i++) if((LCD[i].addr&~0xfffUL)==pages[p]){
      u32 o=LCD[i].addr&0xfffUL, v=LCD[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
    if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
  }
  wr(ok?"/tmp_wbshd/p3_ok":"/tmp_wbshd/p3_bad", ok?"OK":"BAD");
  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/p3_done","D");
}
