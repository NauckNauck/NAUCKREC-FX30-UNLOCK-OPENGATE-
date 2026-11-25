/* SONY FX30 (FW 6.01) — ISP OUTSZ 3840x2560 + VDF NATIVE 3:2 ASPECT (12 u32 sites, 0% LCD stretch) */
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
struct p32_entry { u32 addr; u32 oldv; u32 newv; };
static const struct p32_entry P32[] = {
  {0x0155d43cUL, 0x08700f00UL, 0x0a000f00UL}, /* ISP outsz alt: 3840x2160 -> 3840x2560 */
  {0x0155d444UL, 0x08700f00UL, 0x0a000f00UL}, /* ISP outsz: 3840x2160 -> 3840x2560 */
  {0x001e395cUL, 0x20024601UL, 0x20014601UL}, /* VDF Movie aspect 0x1e395e: movs r0, #2 (16:9) -> #1 (3:2) */
  {0x001e3970UL, 0xbdd02002UL, 0xbdd02001UL}, /* VDF Movie aspect 0x1e3970: movs r0, #2 (16:9) -> #1 (3:2) */
  {0x001e3650UL, 0xbd802002UL, 0xbd802001UL}, /* VDF Movie/SQ aspect 0x1e3650: movs r0, #2 (16:9) -> #1 (3:2) */
  {0x006618a4UL, 0x2002b6c9UL, 0x2001b6c9UL}, /* VDF fallback aspect 0x6618a6: movs r0, #2 (16:9) -> #1 (3:2) */
  {0x001e3654UL, 0x00000002UL, 0x00000001UL}, /* VDF 0x1e3654[0]: 16:9 -> 3:2 */
  {0x001e365cUL, 0x00000002UL, 0x00000001UL}, /* VDF 0x1e3654[2]: 16:9 -> 3:2 */
  {0x001e3660UL, 0x00000002UL, 0x00000001UL}, /* VDF 0x1e3654[3]: 16:9 -> 3:2 */
  {0x001e3668UL, 0x00000002UL, 0x00000001UL}, /* VDF 0x1e3654[5]: 16:9 -> 3:2 */
  {0x001e397cUL, 0x00000001UL, 0x00000002UL}, /* VDF 0x1e3978[1]: idx 1 (16:9) -> idx 2 (3:2) */
  {0x001e3988UL, 0x00000001UL, 0x00000002UL}, /* VDF 0x1e3978[4]: idx 1 (16:9) -> idx 2 (3:2) */
};
#define NP32 (sizeof(P32)/sizeof(P32[0]))
__attribute__((constructor)) static void go(void){
  const char *mk[]={"/tmp_wbshd/ol_start","/tmp_wbshd/ol_ok","/tmp_wbshd/ol_mismatch","/tmp_wbshd/ol_bad","/tmp_wbshd/ol_done"};
  for(unsigned i=0;i<5;i++) sc6(35,-100,(long)mk[i],0,0,0,0);
  wr("/tmp_wbshd/ol_start","S");
  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/ol_bad","ALLOC"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/ol_bad","DEVMEM"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/ol_bad","MMAP"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
  u8 *b=(u8*)m+lead;
  u64 pages[8]; u32 np=0;
  for(u32 i=0;i<NP32;i++){ u64 pg=P32[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<8) pages[np++]=pg; }
  int ok=1;
  for(u32 p=0;p<np;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NP32;i++) if((P32[i].addr&~0xfffUL)==pages[p]){
      u32 o=P32[i].addr&0xfffUL;
      u32 cv=(u32)b[o]|((u32)b[o+1]<<8)|((u32)b[o+2]<<16)|((u32)b[o+3]<<24);
      if(cv!=P32[i].oldv && cv!=P32[i].newv){ ok=0; break; }
    }
    if(!ok) break;
  }
  if(!ok){
    wr("/tmp_wbshd/ol_mismatch","MISMATCH");
    sc6(215,m,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
    wr("/tmp_wbshd/ol_done","D"); return;
  }
  for(u32 p=0;p<np;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NP32;i++) if((P32[i].addr&~0xfffUL)==pages[p]){
      u32 o=P32[i].addr&0xfffUL, v=P32[i].newv;
      b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24);
    }
    if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NP32;i++) if((P32[i].addr&~0xfffUL)==pages[p]){
      u32 o=P32[i].addr&0xfffUL;
      u32 cv=(u32)b[o]|((u32)b[o+1]<<8)|((u32)b[o+2]<<16)|((u32)b[o+3]<<24);
      if(cv!=P32[i].newv){ ok=0; break; }
    }
    if(!ok) break;
  }
  wr(ok?"/tmp_wbshd/ol_ok":"/tmp_wbshd/ol_bad", ok?"OK":"BAD");
  sc6(215,m,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/ol_done","D");
}
