/* FX30 Open Gate — "height only, keep video column" attempt.
 *
 * Keeps the 4K video column (S0 stays 1) so clock/gain/timing remain the video ones,
 * and only pushes the physical height registers (nodes 40/42/44/51/143/144) to the
 * 3:2 values. Also: Node 0 column 11 -> continuous, outsz -> 2560, LCD 3:2.
 *
 * RAM only. Cold boot restores.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);
#define CP_PAGE 4096U

static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++;
  long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){ sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }

struct p16 { u32 addr; u16 oldv; u16 newv; };
static const struct p16 P16[] = {
  {0xc7abbe, 3164, 4168},   /* S1 L: active height */
  {0xc7b776, 3164, 4168},   /* S1 R: active height */
  {0xc9b308, 502,  0   },   /* S2: crop V offset */
  {0xc365d4, 502,  0   },   /* S4: geometry V offset */
};
#define NP16 (sizeof(P16)/sizeof(P16[0]))

/* byte patches: addr, old, new */
struct pb { u32 addr; u8 oldv; u8 newv; };
static const struct pb PB[] = {
  /* Node 0: clock/mode register, column 11 currently single-shot */
  {0x01038d2b, 0x01, 0x0f},
  /* height registers (3:2 values, taken from the Column 11 matrix) */
  {0x01123f7a, 0x02, 0x82},
  {0x01123f86, 0x55, 0x22},
  {0x01123faa, 0x2a, 0x12},
  {0x01123fe6, 0x55, 0x6a},
  {0x011241da, 0x88, 0x22},
  {0x011241de, 0x68, 0xda},
  /* outsz height */
  {0x155d447, 0x08, 0x0a},
};
#define NPB (sizeof(PB)/sizeof(PB[0]))

static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/hg_start","/tmp_wbshd/hg_ok","/tmp_wbshd/hg_bad",
                   "/tmp_wbshd/hg_mismatch","/tmp_wbshd/hg_outsz_ok","/tmp_wbshd/hg_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/hg_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/hg_alloc","A"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/hg_phys","P"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/hg_devmem","D"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/hg_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  /* collect pages */
  u64 pages[8]={0,0,0,0,0,0,0,0}; u32 np=0;
  for(u32 i=0;i<NPB;i++){ u64 pg=PB[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<8) pages[np++]=pg; }

  int ok=1;
  for(u32 p=0;p<np;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NPB;i++){
      if((PB[i].addr & ~0xfffUL)!=pages[p]) continue;
      u8 cv=b[PB[i].addr & 0xfffUL];
      if(cv!=PB[i].oldv && cv!=PB[i].newv){ ok=0; break; }
    }
    if(!ok) break;
    for(u32 i=0;i<NPB;i++){
      if((PB[i].addr & ~0xfffUL)!=pages[p]) continue;
      b[PB[i].addr & 0xfffUL]=PB[i].newv;
    }
    if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NPB;i++){
      if((PB[i].addr & ~0xfffUL)!=pages[p]) continue;
      if(b[PB[i].addr & 0xfffUL]!=PB[i].newv) ok=0;
    }
  }
  /* 16-bit scalars */
  { u64 pages16[8]={0,0,0,0,0,0,0,0}; u32 np16=0;
    for(u32 i=0;i<NP16;i++){ u64 pg=P16[i].addr & ~0xfffUL; int f=0;
      for(u32 k=0;k<np16;k++) if(pages16[k]==pg){f=1;break;}
      if(!f && np16<8) pages16[np16++]=pg; }
    for(u32 p=0;p<np16;p++){
      if(sen_dma_transfer(0,pages16[p],phys,CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<NP16;i++){
        if((P16[i].addr & ~0xfffUL)!=pages16[p]) continue;
        u32 o=P16[i].addr & 0xfffUL;
        u16 cv=(u16)(b[o]|(b[o+1]<<8));
        if(cv!=P16[i].oldv && cv!=P16[i].newv){ ok=0; break; }
      }
      if(!ok) break;
      for(u32 i=0;i<NP16;i++){
        if((P16[i].addr & ~0xfffUL)!=pages16[p]) continue;
        u32 o=P16[i].addr & 0xfffUL, v=P16[i].newv;
        b[o]=(u8)v; b[o+1]=(u8)(v>>8);
      }
      if(sen_dma_transfer(1,phys,pages16[p],CP_PAGE)){ ok=0; break; }
    } }
  wr(ok?"/tmp_wbshd/hg_ok":"/tmp_wbshd/hg_bad", ok?"OK":"BAD");
  /* outsz already in PB; report separately if it was reached */
  wr("/tmp_wbshd/hg_outsz_ok","OUTSZ_OK");
  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); wr("/tmp_wbshd/hg_done","D");
}
