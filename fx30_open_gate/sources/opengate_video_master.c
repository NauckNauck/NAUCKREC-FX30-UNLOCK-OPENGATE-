/* FX30 OPEN GATE 3:2 — MASTER VIDEO CALIBRATION
 *
 * 1. Node 0 (Clock): 0x01038d2b = 0x0F (Continuous Video Streaming)
 * 2. S0 (Hardware Matrix Column): 0x01039950 = 11 (3:2 Sensor Readout)
 * 3. Mode 3 (Video Records) Height-Only Stretched to 3:2:
 *      Left  (0xc7abb0): w3=4200, w7=4168, w11=4128, w943=4168, w1199=0
 *      Right (0xc7b768): w3=4200, w7=4168, w11=4128, w943=4168, w1199=0
 *    -> Preserves 100% of 4K Video Gain, Black Level, White Balance & Stride!
 * 4. S4 & S2 V-offsets: 0xc365d4 = 0, 0xc9b308 = 0
 * 5. ISP Scaler Outsz: 0x155d446 = 0x0a00 (2560 lines)
 * 6. DMM Buffers: 29.29 MB (0x01bf0000)
 * 7. LCD Display Driver: 7 sites unlocked (preview 3:2)
 *
 * RAM ONLY. Volatile.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U

struct p16 { u32 addr; u16 old_v; u16 new_v; };
static const struct p16 P16[] = {
  /* Mode 3 Left Height Fields */
  {0xc7abb6, 3190, 4200}, {0xc7abbe, 3164, 4168}, {0xc7abc6, 3156, 4128},
  {0xc7b30e, 3164, 4168}, {0xc7b50e, 502,  0   },
  /* Mode 3 Right Height Fields */
  {0xc7b76e, 3190, 4200}, {0xc7b776, 3164, 4168}, {0xc7b77e, 3156, 4128},
  {0xc7bec6, 3164, 4168}, {0xc7c0c6, 502,  0   },
  /* Global V-Offsets & ISP Outsz */
  {0xc365d4,  502,  0   }, {0xc9b308,  502,  0   },
  {0x155d446, 0x0870, 0x0a00},
};
#define N16 (sizeof(P16)/sizeof(P16[0]))

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

__attribute__((constructor)) static void go(void){
  wr("/tmp_wbshd/vm_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/vm_alloc","A"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  /* 1. S0 (0x01039950 = 11) & Node 0 Streaming (0x01038d2b = 0x0f) */
  { u32 s0_pg = 0x01039000UL;
    if(sen_dma_transfer(0, s0_pg, phys, CP_PAGE)==0){
      u32 o = (u32)(0x01039950UL - s0_pg);
      b[o] = 11; b[o+1] = 0; b[o+2] = 0; b[o+3] = 0;
      sen_dma_transfer(1, phys, s0_pg, CP_PAGE);
    }
    u32 n0_pg = 0x01038000UL;
    if(sen_dma_transfer(0, n0_pg, phys, CP_PAGE)==0){
      u32 o = (u32)(0x01038d2bUL - n0_pg);
      b[o] = 0x0f;
      sen_dma_transfer(1, phys, n0_pg, CP_PAGE);
      wr("/tmp_wbshd/vm_s0_ok","S0_OK");
    }
  }

  /* 2. Mode 3 Height Fields + V-offsets + Outsz */
  { int ok=1;
    u64 pages[16]; u32 np=0;
    for(u32 i=0;i<N16;i++){
      u64 pg = P16[i].addr & ~0xfffUL;
      int found=0;
      for(u32 k=0;k<np;k++) if(pages[k]==pg) { found=1; break; }
      if(!found && np<16) pages[np++]=pg;
    }
    for(u32 p=0;p<np;p++){
      if(sen_dma_transfer(0, pages[p], phys, CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<N16;i++){
        if((P16[i].addr & ~0xfffUL) != pages[p]) continue;
        st16(b, (u32)(P16[i].addr & 0xfffUL), P16[i].new_v);
      }
      if(sen_dma_transfer(1, phys, pages[p], CP_PAGE)){ ok=0; break; }
    }
    wr(ok?"/tmp_wbshd/vm_height_ok":"/tmp_wbshd/vm_height_bad", ok?"OK":"BAD");
  }

  /* 3. DMM Buffers: 2 records to 29.29 MB */
  { u32 da = 0x2080000UL;
    if(sen_dma_transfer(0, da, phys, CP_PAGE)==0){
      u32 off1=(u32)(0x2080860UL-da), off2=(u32)(0x2080874UL-da);
      u32 nv=0x01bf0000;
      for(int k=0;k<2;k++){
        u32 o=k?off2:off1;
        b[o]=(u8)nv; b[o+1]=(u8)(nv>>8); b[o+2]=(u8)(nv>>16); b[o+3]=(u8)(nv>>24);
      }
      sen_dma_transfer(1, phys, da, CP_PAGE);
      wr("/tmp_wbshd/vm_dmm_ok","DMM_OK");
    }
  }

  /* 4. LCD Display 3:2 Preview */
  { u64 lp[2]={0x1e2d000UL, 0x00de7000UL};
    for(u32 q=0;q<2;q++){
      if(sen_dma_transfer(0,lp[q],phys,CP_PAGE)==0){
        for(u32 i=0;i<N32;i++){
          if((P32[i].addr & ~0xfffUL)!=lp[q]) continue;
          u32 o=(u32)(P32[i].addr & 0xfffUL), v=P32[i].new_v;
          b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24);
        }
        sen_dma_transfer(1,phys,lp[q],CP_PAGE);
      }
    }
    wr("/tmp_wbshd/vm_lcd_ok","LCD_OK");
  }

  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle);
  wr("/tmp_wbshd/vm_done","D");
}
