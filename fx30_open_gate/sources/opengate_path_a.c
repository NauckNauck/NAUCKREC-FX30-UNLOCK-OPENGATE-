/* FX30 OPEN GATE 3:2 — FASE 3: CAMINHO A DEFINITIVO (5952 x 3968 -> 3840 x 2560)
 *
 * 1. Physical Sensor Timing Generator (IMX684 TG):
 *    Expande a varredura vertical física de 3348 para 3968 linhas (5952 x 3968).
 *    Programado tanto na Node Table (0x01038000) quanto no Active Buffer (0x01123000):
 *      - Node 3 / Reg 0x0006 (VBLANK Low) : 0xe7 -> 0xb1 (177 half-lines)
 *      - Node 4 / Reg 0x0007 (VBLANK High): 0x01 -> 0x00
 *      - Node 5 / Reg 0x0008 (Active Low) : 0x90 -> 0xc6 (1990 half-lines = 3968 linhas + 12)/2
 *      - Node 6 / Reg 0x0009 (Active High): 0x06 -> 0x07 (0x07c6)
 *      - Node 7 / Reg 0x000e (Prescaler)  : 0x09 (preservado stock vídeo 4K)
 *      - Node 8 / Reg 0x0010 (Complement) : 0x22 (preservado stock vídeo 4K)
 *      - Node 10 / Reg 0x0019 (SLVS-EC clk): 0x29 (preservado stock vídeo 4K)
 *    Total VMAX = 1990 + 177 = 2167 linhas de frame (100% idêntico ao pixel clock stock)!
 *
 * 2. Hardware Matrix S0:
 *    Mantém Mode 6 em Coluna 1 (0x01039950 = 1).
 *    Preserva 5952 px de largura (2976 px por canal Left/Right).
 *    Alinhamento perfeito de fase Left/Right (elimina listras verticais!).
 *    Preserva cores reais de vídeo e S-Log3 (sem saturação de foto).
 *
 * 3. Software Mode 3 Records:
 *    Ajusta campos de altura Left (0xc7abb0) e Right (0xc7b768) para 3968 linhas.
 *    Offsets verticais definidos como 0 (sem corte do topo, leitura direta de 3968 linhas).
 *
 * 4. ImagerDrvParam Entry 6:
 *    0x20d9c4a: 3348 -> 3968.
 *
 * 5. Global V-Offsets:
 *    0xc365d4 (S4) = 0, 0xc9b308 (S2) = 0.
 *
 * 6. ISP Scaler Outsz:
 *    0x155d446: 0x0870 -> 0x0a00 (3840 x 2560 Open Gate 3:2).
 *
 * 7. DMM Buffers:
 *    0x2080860 / 0x2080874 expandidos para 29.29 MB (0x01bf0000).
 *
 * 8. Display Driver:
 *    7 sites para LCD Preview 3:2 sem barras pretas.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U

struct p16 { u32 addr; u16 old_v; u16 new_v; };
static const struct p16 P16[] = {
  /* Mode 3 Left Height Fields (3968 linhas 3:2) */
  {0xc7abb6, 3190, 3994}, /* w3  Bounding Height */
  {0xc7abbe, 3164, 3968}, /* w7  Active Height   */
  {0xc7abc6, 3156, 3960}, /* w11 Inner Height (w7 - 8 = 3960, cabe em w9=30) */
  {0xc7b30e, 3164, 3968}, /* w943 Aux Height    */
  {0xc7b50e, 502,  0   }, /* w1199 Crop V-Offset: 0 (sem corte do topo) */
  /* Mode 3 Right Height Fields (3968 linhas 3:2) */
  {0xc7b76e, 3190, 3994}, /* w3  Bounding Height */
  {0xc7b776, 3164, 3968}, /* w7  Active Height   */
  {0xc7b77e, 3156, 3960}, /* w11 Inner Height (w7 - 8 = 3960, cabe em w9=30) */
  {0xc7bec6, 3164, 3968}, /* w943 Aux Height    */
  {0xc7c0c6, 502,  0   }, /* w1199 Crop V-Offset: 0 (sem corte do topo) */
  /* Mode 6 ImagerDrvParam Entry (3348 -> 3968) */
  {0x20d9c4a, 3348, 3968},
  /* Global V-Offsets: 0 (leitura direta a partir da linha física 0) */
  {0xc365d4,  502,  0   }, /* S4 geometry row V offset */
  {0xc9b308,  502,  0   }, /* S2 crop V offset         */
  /* Scaler Outsz: 3840 x 2560 */
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
static void st16(u8*b,u32 o,u16 v){ b[o]=(u8)v; b[o+1]=(u8)(v>>8); }

__attribute__((constructor)) static void go(void){
  wr("/tmp_wbshd/pa_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/pa_alloc","A"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  /* 1. S0: Mantém na Coluna 1 (Vídeo 4K Nativo) */
  { u32 s0_pg = 0x01039000UL;
    if(sen_dma_transfer(0, s0_pg, phys, CP_PAGE)==0){
      u32 o = (u32)(0x01039950UL - s0_pg);
      b[o] = 1; b[o+1] = 0; b[o+2] = 0; b[o+3] = 0;
      sen_dma_transfer(1, phys, s0_pg, CP_PAGE);
      wr("/tmp_wbshd/pa_s0_ok","S0_COL1_OK");
    }
  }

  /* 2. Physical Sensor Timing Generator: 3968 linhas físicas (1990 half-lines + 177 vblank = 2167 total) */
  { int tg_ok = 1;
    /* 2a. Node Table em 0x01038000 */
    u32 n_pg = 0x01038000UL;
    if(sen_dma_transfer(0, n_pg, phys, CP_PAGE)==0){
      b[0xd5d] = 0xb1; /* Node 3: Reg 0x0006 VBLANK Low  (177 half-lines) */
      b[0xd71] = 0x00; /* Node 4: Reg 0x0007 VBLANK High */
      b[0xd85] = 0xc6; /* Node 5: Reg 0x0008 Active Low  (1990 half-lines = 3968 linhas + 12)/2 */
      b[0xd99] = 0x07; /* Node 6: Reg 0x0009 Active High (0x07c6) */
      b[0xdad] = 0x09; /* Node 7: Reg 0x000e Prescaler   (stock vídeo 4K) */
      b[0xdc1] = 0x22; /* Node 8: Reg 0x0010 Complement  (stock vídeo 4K) */
      b[0xde9] = 0x29; /* Node 10: Reg 0x0019 SLVS-EC clk (stock vídeo 4K) */
      if(sen_dma_transfer(1, phys, n_pg, CP_PAGE)!=0) tg_ok = 0;
    } else tg_ok = 0;

    /* 2b. Active Register Buffer em 0x01123000 */
    u32 r_pg = 0x01123000UL;
    if(sen_dma_transfer(0, r_pg, phys, CP_PAGE)==0){
      b[0xdfe] = 0xb1; /* Reg 0x0006 VBLANK Low  */
      b[0xe02] = 0x00; /* Reg 0x0007 VBLANK High */
      b[0xe06] = 0xc6; /* Reg 0x0008 Active Low  */
      b[0xe0a] = 0x07; /* Reg 0x0009 Active High */
      b[0xe1e] = 0x09; /* Reg 0x000e Prescaler   (stock) */
      b[0xe26] = 0x22; /* Reg 0x0010 Complement  (stock) */
      b[0xe4a] = 0x29; /* Reg 0x0019 SLVS-EC clk (stock) */
      if(sen_dma_transfer(1, phys, r_pg, CP_PAGE)!=0) tg_ok = 0;
    } else tg_ok = 0;

    wr(tg_ok ? "/tmp_wbshd/pa_tg_ok" : "/tmp_wbshd/pa_tg_bad", tg_ok ? "TG_3968_OK" : "TG_BAD");
  }

  /* 3. Ajuste dos campos de altura para 3968 linhas (3:2 perfeito) */
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
    wr(ok?"/tmp_wbshd/pa_height_ok":"/tmp_wbshd/pa_height_bad", ok?"OK":"BAD");
  }

  /* 4. DMM Buffers: 29.29 MB */
  { u32 da = 0x2080000UL;
    if(sen_dma_transfer(0, da, phys, CP_PAGE)==0){
      u32 off1=(u32)(0x2080860UL-da), off2=(u32)(0x2080874UL-da);
      u32 nv=0x01bf0000;
      for(int k=0;k<2;k++){
        u32 o=k?off2:off1;
        b[o]=(u8)nv; b[o+1]=(u8)(nv>>8); b[o+2]=(u8)(nv>>16); b[o+3]=(u8)(nv>>24);
      }
      sen_dma_transfer(1, phys, da, CP_PAGE);
      wr("/tmp_wbshd/pa_dmm_ok","DMM_OK");
    }
  }

  /* 5. Display LCD 3:2 Preview */
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
    wr("/tmp_wbshd/pa_lcd_ok","LCD_OK");
  }

  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle);
  wr("/tmp_wbshd/pa_done","D");
}
