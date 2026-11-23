/* FX30 Open Gate — T3: Tradução de Dois Eixos (Coluna 11 + Mode 2 L/R 6192x4128)
 * Baseado no GEMINI_PROMPT_OPEN_GATE_FINAL.md (23/11).
 * 100% Volátil em RAM. Cold boot (bateria 10s) restaura de fábrica.
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
  const char *m[]={"/tmp_wbshd/ta_start","/tmp_wbshd/ta_ok","/tmp_wbshd/ta_bad",
                   "/tmp_wbshd/ta_mismatch","/tmp_wbshd/ta_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

struct p8 { u32 addr; u8 oldv; u8 newv; };
struct p16 { u32 addr; u16 oldv; u16 newv; };
struct p32 { u32 addr; u32 oldv; u32 newv; };

/* 1. S0 & Node 0 Streaming */
static const struct p32 P_S0[] = {
  {0x01039950UL, 1, 11}, /* S0: Mode 6 column 1 -> 11 */
};
static const struct p8 P_N0[] = {
  {0x01038d2bUL, 0x01, 0x0f}, /* Node 0 c11: single-shot -> continuous streaming */
};

/* 2. ImagerDrvParam (Entry 2 Preview e Entry 6 Rec 24p) */
static const struct p16 P_DRV[] = {
  /* Entry 2 (0x020d9664) */
  {0x020d968cUL,  340, 1040}, /* line timing 1 */
  {0x020d9698UL,  340, 1040}, /* line timing 2 */
  {0x020d96b8UL, 5952, 6192}, /* width */
  {0x020d96caUL, 3348, 4128}, /* height */
  {0x020d975aUL,  406,    0}, /* crop V 1 */
  {0x020d9766UL,  406,    0}, /* crop V 2 */
  {0x020d97a6UL, 1704, 2100}, /* S3 line periods */

  /* Entry 6 (0x020d9be4) */
  {0x020d9c0cUL,  340, 1040}, /* line timing 1 */
  {0x020d9c18UL,  340, 1040}, /* line timing 2 */
  {0x020d9c38UL, 5952, 6192}, /* width */
  {0x020d9c4aUL, 3348, 4128}, /* height */
  {0x020d9cdaUL,  406,    0}, /* crop V 1 */
  {0x020d9ce6UL,  406,    0}, /* crop V 2 */
  {0x020d9d26UL, 1704, 2100}, /* S3 line periods */
};

/* 3. Scalars */
static const struct p16 P_SC[] = {
  {0x00e02888UL, 3348, 4128},     /* S1 active height mode 6 */
  {0x00e03298UL, 1704, 2100},     /* S3 row 0 (24p) */
  {0x00e032a6UL, 1609, 2100},     /* S3 row 1 (25p) */
  {0x00e032b4UL, 1704, 2100},     /* S3 row 2 (30p) */
  {0x0155d446UL, 0x0870, 0x0a00}, /* outsz height 2160 -> 2560 */
};

/* 4. DMM Buffers (29.29 MB) */
static const struct p32 P_DMM[] = {
  {0x02080860UL, 0x01788700UL, 0x01bf0000UL},
  {0x02080874UL, 0x01788700UL, 0x01bf0000UL},
};

/* 5. Mode 2 ISP Front-End Record (Left: 0x00c79440, Right: 0x00c79ff8) */
static const struct p16 P_M2[] = {
  /* Verticais Left (0x00c79440) */
  {0x00c79446UL, 3382, 4162},   /* bounding height */
  {0x00c7944eUL, 3356, 4136},   /* guardband height */
  {0x00c79456UL, 3348, 4128},   /* active height */
  {0x00c79bbcUL, 3356, 4128},   /* scaler in-h */
  {0x00c79bc4UL, 5625, 6912},   /* luma v-step: round(4128/2442 * 4096) */
  {0x00c79bc6UL, 5623, 6911},   /* chroma v-step */
  {0x00c79d9eUL,  406,    0},   /* crop V offset */

  /* Verticais Right (0x00c79ff8) */
  {0x00c79ffeUL, 3382, 4162},   /* bounding height */
  {0x00c7a006UL, 3356, 4136},   /* guardband height */
  {0x00c7a00eUL, 3348, 4128},   /* active height */
  {0x00c7a774UL, 3356, 4128},   /* scaler in-h */
  {0x00c7a77cUL, 5625, 6912},   /* luma v-step */
  {0x00c7a77eUL, 5623, 6911},   /* chroma v-step */
  {0x00c7a956UL,  406,    0},   /* crop V offset */

  /* Horizontais Left (0x00c79440) — larguras */
  {0x00c79460UL, 3008, 3136},
  {0x00c79468UL, 3016, 3144},
  {0x00c79470UL, 3016, 3144},
  {0x00c79478UL, 3004, 3118},
  {0x00c79484UL, 3008, 3136},
  {0x00c7948cUL, 3008, 3136},
  {0x00c79494UL, 3008, 3136},
  {0x00c7949cUL, 2996, 3110},
  {0x00c794a4UL, 3008, 3136},
  {0x00c794acUL, 3008, 3136},
  {0x00c794b4UL, 2996, 3110},
  {0x00c794c0UL, 3008, 3136},
  {0x00c794c8UL, 3024, 3168},
  {0x00c794d0UL, 3008, 3136},
  {0x00c794d8UL, 2996, 3110},
  {0x00c794e4UL, 3008, 3136},

  /* Horizontais Right (0x00c79ff8) — X offsets e larguras */
  {0x00c79ff8UL, 2664, 3160}, /* X */
  {0x00c7a000UL, 2664, 3160}, /* X */
  {0x00c7a008UL, 2664, 3160}, /* X */
  {0x00c7a018UL, 3008, 3136}, /* W */
  {0x00c7a01cUL, 2648, 3128}, /* X */
  {0x00c7a020UL, 3016, 3144}, /* W */
  {0x00c7a024UL, 2648, 3128}, /* X */
  {0x00c7a028UL, 2976, 3116}, /* W */
  {0x00c7a02cUL, 2648, 3128}, /* X */
  {0x00c7a030UL, 2964, 3090}, /* W */
  {0x00c7a03cUL, 3008, 3136}, /* W */
  {0x00c7a040UL, 3008, 3136}, /* W */
  {0x00c7a044UL, 2968, 3112}, /* W */
  {0x00c7a048UL, 3008, 3136}, /* W */
  {0x00c7a04cUL, 2968, 3108}, /* W */
  {0x00c7a050UL, 3008, 3136}, /* W */
  {0x00c7a054UL, 2956, 3082}, /* W */
  {0x00c7a058UL, 3008, 3136}, /* W */
  {0x00c7a05cUL, 3008, 3136}, /* W */
  {0x00c7a060UL, 3008, 3136}, /* W */
  {0x00c7a064UL, 2968, 3108}, /* W */
  {0x00c7a068UL, 3008, 3136}, /* W */
  {0x00c7a06cUL, 2956, 3082}, /* W */
  {0x00c7a078UL, 3008, 3136}, /* W */
  {0x00c7a07cUL, 2664, 3160}, /* X */
  {0x00c7a080UL, 3024, 3168}, /* W */
  {0x00c7a084UL, 2664, 3160}, /* X */
  {0x00c7a088UL, 2968, 3108}, /* W */
  {0x00c7a08cUL, 2664, 3160}, /* X */
  {0x00c7a090UL, 2956, 3082}, /* W */
  {0x00c7a09cUL, 3008, 3136}, /* W */
};

#define NS0  (sizeof(P_S0)/sizeof(P_S0[0]))
#define NN0  (sizeof(P_N0)/sizeof(P_N0[0]))
#define NDRV (sizeof(P_DRV)/sizeof(P_DRV[0]))
#define NSC  (sizeof(P_SC)/sizeof(P_SC[0]))
#define NDMM (sizeof(P_DMM)/sizeof(P_DMM[0]))
#define NM2  (sizeof(P_M2)/sizeof(P_M2[0]))

static u64 pages[32]; static u32 npages;
static void add_page(u64 a){ u64 pg=a&~0xfffUL; for(u32 i=0;i<npages;i++) if(pages[i]==pg) return; if(npages<32) pages[npages++]=pg; }

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/ta_start","S");

  u32 h=0; u64 phys=cam_dmm_alloc(CP_PAGE,&h);
  if(!phys){ wr("/tmp_wbshd/ta_bad","NO_ALLOC"); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/ta_bad","NO_DEVMEM"); cam_dmm_free(h); return; }
  u64 lead=phys&0xfff, ml=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 mp=sc6(222,0,(long)ml,3,1,fd,(long)(phys-lead));
  if(mp<0){ wr("/tmp_wbshd/ta_bad","NO_MMAP"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h); return; }
  u8 *b=(u8*)mp+lead;

  for(u32 i=0;i<NS0;i++)  add_page(P_S0[i].addr);
  for(u32 i=0;i<NN0;i++)  add_page(P_N0[i].addr);
  for(u32 i=0;i<NDRV;i++) add_page(P_DRV[i].addr);
  for(u32 i=0;i<NSC;i++)  add_page(P_SC[i].addr);
  for(u32 i=0;i<NDMM;i++) add_page(P_DMM[i].addr);
  for(u32 i=0;i<NM2;i++)  add_page(P_M2[i].addr);

  int ok=1;
  /* Phase 1: Verify all stock checks across all pages */
  for(u32 p=0;p<npages && ok;p++){
    if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
    for(u32 i=0;i<NS0;i++) if((P_S0[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_S0[i].addr&0xfffUL; u32 cv=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
      if(cv!=P_S0[i].oldv && cv!=P_S0[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","S0_MISMATCH"); break; } }
    for(u32 i=0;i<NN0;i++) if((P_N0[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_N0[i].addr&0xfffUL; u8 cv=b[o];
      if(cv!=P_N0[i].oldv && cv!=P_N0[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","N0_MISMATCH"); break; } }
    for(u32 i=0;i<NDRV;i++) if((P_DRV[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_DRV[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=P_DRV[i].oldv && cv!=P_DRV[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","DRV_MISMATCH"); break; } }
    for(u32 i=0;i<NSC;i++) if((P_SC[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_SC[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=P_SC[i].oldv && cv!=P_SC[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","SC_MISMATCH"); break; } }
    for(u32 i=0;i<NDMM;i++) if((P_DMM[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_DMM[i].addr&0xfffUL; u32 cv=(u32)(b[o]|(b[o+1]<<8)|(b[o+2]<<16)|(b[o+3]<<24));
      if(cv!=P_DMM[i].oldv && cv!=P_DMM[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","DMM_MISMATCH"); break; } }
    for(u32 i=0;i<NM2;i++) if((P_M2[i].addr&~0xfffUL)==pages[p]){
      u32 o=P_M2[i].addr&0xfffUL; u16 cv=(u16)(b[o]|(b[o+1]<<8));
      if(cv!=P_M2[i].oldv && cv!=P_M2[i].newv){ ok=0; wr("/tmp_wbshd/ta_mismatch","M2_MISMATCH"); break; } }
  }

  /* Phase 2: Apply patches if all checks passed */
  if(ok){
    for(u32 p=0;p<npages;p++){
      if(sen_dma_transfer(0,pages[p],phys,CP_PAGE)){ ok=0; break; }
      for(u32 i=0;i<NS0;i++) if((P_S0[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_S0[i].addr&0xfffUL, v=P_S0[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
      for(u32 i=0;i<NN0;i++) if((P_N0[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_N0[i].addr&0xfffUL; b[o]=P_N0[i].newv; }
      for(u32 i=0;i<NDRV;i++) if((P_DRV[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_DRV[i].addr&0xfffUL, v=P_DRV[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
      for(u32 i=0;i<NSC;i++) if((P_SC[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_SC[i].addr&0xfffUL, v=P_SC[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
      for(u32 i=0;i<NDMM;i++) if((P_DMM[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_DMM[i].addr&0xfffUL, v=P_DMM[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); b[o+2]=(u8)(v>>16); b[o+3]=(u8)(v>>24); }
      for(u32 i=0;i<NM2;i++) if((P_M2[i].addr&~0xfffUL)==pages[p]){
        u32 o=P_M2[i].addr&0xfffUL, v=P_M2[i].newv; b[o]=(u8)v; b[o+1]=(u8)(v>>8); }
      if(sen_dma_transfer(1,phys,pages[p],CP_PAGE)){ ok=0; break; }
    }
  }

  wr(ok?"/tmp_wbshd/ta_ok":"/tmp_wbshd/ta_bad", ok?"OK":"BAD");
  sc6(215,mp,(long)ml,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(h);
  wr("/tmp_wbshd/ta_done","D");
}
