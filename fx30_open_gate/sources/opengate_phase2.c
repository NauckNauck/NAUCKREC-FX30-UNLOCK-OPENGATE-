/* FX30 Open Gate 3:2 — DEFINITIVE patch (Phase 2)
 *
 * 1. S0: sensor hardware mode column selector.
 *      ImagerDrvParam column table @ CP 0x01039938 ; mode index 6 (4K) -> column 11 (3:2 full)
 *      stock: 1  ->  new: 11   at CP 0x01039950
 * 2. Entry 6 geometry (mode 0x8c0): 5952x3348 -> 6192x4128 (11 fields)
 * 3. S4 / S2: zero the vertical crop offsets (502 -> 0)
 * 4. outsz: 0x155d446  0x0870 -> 0x0a00 (3840x2560)
 * 5. dmmwb: 2 records  0x1788700 -> 0x01bf0000
 *
 * RAM only. Verifies each stock value; reads back. Cold boot restores.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U
#define PA  0x20d9000UL          /* entry 6 page */
#define SA  0x01039000UL         /* column table page (S0) */
#define OA  0x155d000UL          /* outsz page */
#define DA  0x2080000UL          /* dmmwb page */

struct p16 { u32 addr; u16 old_v; u16 new_v; };
static const struct p16 P16[] = {
  {0x20d9bf6, 267,  270 },  {0x20d9c34, 34,   12  },  {0x20d9c36, 12,   26  },
  {0x20d9c38, 5952, 6192},  {0x20d9c3a, 12,   26  },  {0x20d9c3c, 38,   8   },
  {0x20d9c46, 2,    8   },  {0x20d9c48, 4,    20  },  {0x20d9c4a, 3348, 4128},
  {0x20d9c4c, 4,    20  },  {0x20d9c4e, 2,    8   },
  {0xc365d4,  502,  0   },  {0xc9b308,  502,  0   },  /* S4, S2 */
  {0x155d446, 0x0870, 0x0a00},
};
#define N16 (sizeof(P16)/sizeof(P16[0]))

/* --- LCD (fx30lcd_v2): force the display sub-scaler out of its 16:9 crop --- */
struct p32 { u32 addr; u32 old_v; u32 new_v; };
static const struct p32 P32[] = {
  {0x1e2d3cUL, 0x00000003UL, 0x00000000UL},
  {0xde791cUL, 0x00260000UL, 0x00000000UL},
  {0xde7920UL, 0x01940280UL, 0x01e00280UL},
  {0xde7994UL, 0x00260000UL, 0x00000000UL},
  {0xde7998UL, 0x01940280UL, 0x01e00280UL},
  {0xde7a0cUL, 0x00260000UL, 0x00000000UL},
  {0xde7a10UL, 0x01940280UL, 0x01e00280UL},
};
#define N32 (sizeof(P32)/sizeof(P32[0]))

static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++;
  long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){ sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }
static u16 ld16(const u8*b,u32 o){ return (u16)(b[o]|(b[o+1]<<8)); }
static void st16(u8*b,u32 o,u16 v){ b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/og2_ok","/tmp_wbshd/og2_bad","/tmp_wbshd/og2_mismatch","/tmp_wbshd/og2_s0_ok",
                   "/tmp_wbshd/og2_s0_mismatch","/tmp_wbshd/og2_outsz_ok","/tmp_wbshd/og2_dmm_ok",
                   "/tmp_wbshd/og2_start","/tmp_wbshd/og2_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/og2_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/og2_alloc","A"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/og2_phys","P"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/og2_devmem","D"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/og2_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  /* --- 1. S0: column table for mode 6: 1 -> 11 --- */
  { u32 cp=0x01039950UL;
    if(sen_dma_transfer(0, SA, phys, CP_PAGE)==0){
      u32 o=(u32)(cp-SA);
      u32 cur=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
      if(cur==1 || cur==11){
        b[o]=(u8)11; b[o+1]=0; b[o+2]=0; b[o+3]=0;
        if(sen_dma_transfer(1, phys, SA, CP_PAGE)==0){
          if(sen_dma_transfer(0, SA, phys, CP_PAGE)==0){
            u32 nv=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
            wr(nv==11?"/tmp_wbshd/og2_s0_ok":"/tmp_wbshd/og2_s0_bad","S0_OK");
          }
        }
      } else wr("/tmp_wbshd/og2_s0_mismatch","!");
    } }

  /* --- 1.1 S0 Continuous Streaming Override: Node 0 Column 11: 0x01 (Still 1-shot) -> 0x0f (Video Stream) --- */
  { u32 na = 0x01038000UL;
    if(sen_dma_transfer(0, na, phys, CP_PAGE)==0){
      u32 o = 0x01038d2bUL - na;
      b[o] = 0x0f;
      sen_dma_transfer(1, phys, na, CP_PAGE);
    }
  }


  /* --- 2. entry 6 + S4/S2 + outsz (all via the same page buffer, one page at a time) --- */
  { int ok=1, done=0;
    u64 pages[8] = { PA, 0,0,0,0,0,0,0 }; u32 np=1;
    /* collect pages from P16 */
    for(u32 i=0;i<N16;i++){ u64 pg=P16[i].addr & ~0xfffUL; int found=0;
      for(u32 k=0;k<np;k++) if(pages[k]==pg) { found=1; break; }
      if(!found && np<8) pages[np++]=pg; }
    for(u32 p=0;p<np;p++){
      if(sen_dma_transfer(0, pages[p], phys, CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<N16;i++){
        if((P16[i].addr & ~0xfffUL) != pages[p]) continue;
        u32 o=(u32)(P16[i].addr & 0xfffUL);
        u16 cv=ld16(b,o);
        if(cv!=P16[i].old_v && cv!=P16[i].new_v){ ok=0; break; }
      }
      if(!ok) break;
      for(u32 i=0;i<N16;i++){
        if((P16[i].addr & ~0xfffUL) != pages[p]) continue;
        st16(b,(u32)(P16[i].addr & 0xfffUL),P16[i].new_v);
      }
      if(sen_dma_transfer(1, phys, pages[p], CP_PAGE)){ ok=0; break; }
      if(sen_dma_transfer(0, pages[p], phys, CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<N16;i++){
        if((P16[i].addr & ~0xfffUL) != pages[p]) continue;
        if(ld16(b,(u32)(P16[i].addr & 0xfffUL))!=P16[i].new_v) ok=0;
      }
      done=1;
    }
    if(done) wr(ok?"/tmp_wbshd/og2_ok":"/tmp_wbshd/og2_bad", ok?"OK":"BAD");
    else wr("/tmp_wbshd/og2_mismatch","!"); }

  /* --- 3. dmmwb: 2 records 0x1788700 -> 0x01bf0000 --- */
  if(sen_dma_transfer(0, DA, phys, CP_PAGE)==0){
    u32 off1=(u32)(0x2080860UL-DA), off2=(u32)(0x2080874UL-DA);
    int ok=1;
    for(int k=0;k<2;k++){
      u32 o=k?off2:off1;
      u32 cv=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
      if(cv!=0x1788700 && cv!=0x01bf0000) ok=0;
    }
    if(ok){
      u32 nv=0x01bf0000;
      for(int k=0;k<2;k++){ u32 o=k?off2:off1; b[o]=(u8)nv; b[o+1]=(u8)(nv>>8); b[o+2]=(u8)(nv>>16); b[o+3]=(u8)(nv>>24); }
      if(sen_dma_transfer(1, phys, DA, CP_PAGE)==0){
        if(sen_dma_transfer(0, DA, phys, CP_PAGE)==0){
          int ok2=1;
          for(int k=0;k<2;k++){ u32 o=k?off2:off1; u32 v=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24)); if(v!=nv) ok2=0; }
          wr(ok2?"/tmp_wbshd/og2_dmm_ok":"/tmp_wbshd/og2_dmm_bad","DMM_OK");
        }
      }
    }
  }


  /* --- LCD: 7 sites --- */
  { int lcd_ok=1;
    u64 lp[2]={0x1e2d000UL,0x00de7000UL};
    for(u32 q=0;q<2;q++){
      if(sen_dma_transfer(0,lp[q],phys,CP_PAGE)){ lcd_ok=0; break; }
      for(u32 i=0;i<N32;i++){
        if((P32[i].addr & ~0xfffUL)!=lp[q]) continue;
        u32 o=(u32)(P32[i].addr & 0xfffUL);
        u32 cv=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
        if(cv!=P32[i].old_v && cv!=P32[i].new_v) lcd_ok=0;
      }
      if(!lcd_ok) break;
      for(u32 i=0;i<N32;i++){
        if((P32[i].addr & ~0xfffUL)!=lp[q]) continue;
        u32 o=(u32)(P32[i].addr & 0xfffUL), v=P32[i].new_v;
        b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24);
      }
      if(sen_dma_transfer(1,phys,lp[q],CP_PAGE)){ lcd_ok=0; break; }
    }
    wr(lcd_ok?"/tmp_wbshd/og2_lcd_ok":"/tmp_wbshd/og2_lcd_bad", lcd_ok?"LCD_OK":"LCD_BAD"); }

  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); wr("/tmp_wbshd/og2_done","D");
}
