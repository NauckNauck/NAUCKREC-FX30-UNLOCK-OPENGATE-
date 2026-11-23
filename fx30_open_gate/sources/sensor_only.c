/* FX30 Open Gate — sensor mode geometry patch (entry 6 = 4K 24p, mode 0x8c0)
 *
 * Changes the 4K mode's readout window from 5952x3348 (16:9) to 6192x4128 (3:2 full),
 * copying the geometry fields of entry 25 (the native 3:2 mode), while KEEPING the
 * mode id 0x8c0 at +0x24 so the 4K mode still exists.
 *
 * RAM only. Verifies every stock value before writing, reads back after.
 * Build: zig cc -target aarch64-linux-gnu ... -o libJit.so
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U
#define PA  0x20d9000UL          /* the page holding entry 6 (0x20d9be4..0x20d9d44) */
#define PB  0x20db000UL          /* not used: entry 25 source is read-only reference  */
#define ENTRY6 0x20d9be4UL

struct patch { u32 addr; u16 old_v; u16 new_v; };
static const struct patch P[] = {
  {0x20d9bf6, 267,  270 },   /* +0x12  mode selector */
  {0x20d9c34, 34,   12  },   /* +0x50 */
  {0x20d9c36, 12,   26  },   /* +0x52 */
  {0x20d9c38, 5952, 6192},   /* +0x54  WIDTH  */
  {0x20d9c3a, 12,   26  },   /* +0x56 */
  {0x20d9c3c, 38,   8   },   /* +0x58 */
  {0x20d9c46, 2,    8   },   /* +0x62 */
  {0x20d9c48, 4,    20  },   /* +0x64 */
  {0x20d9c4a, 3348, 4128},   /* +0x66  HEIGHT */
  {0x20d9c4c, 4,    20  },   /* +0x68 */
  {0x20d9c4e, 2,    8   },   /* +0x6a */
};
#define NPATCH (sizeof(P)/sizeof(P[0]))

static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++;
  long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){ sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }
static u16 ld16(const u8*b,u32 off){ return (u16)(b[off] | (b[off+1]<<8)); }
static void st16(u8*b,u32 off,u16 v){ b[off]=(u8)v; b[off+1]=(u8)(v>>8); }

static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/sg_ok","/tmp_wbshd/sg_bad","/tmp_wbshd/sg_mismatch","/tmp_wbshd/sg_modeid",
                    "/tmp_wbshd/sg_outsz_ok","/tmp_wbshd/sg_outsz_bad","/tmp_wbshd/sg_outsz_mismatch",
                    "/tmp_wbshd/sg_outsz_rd","/tmp_wbshd/sg_outsz_wr","/tmp_wbshd/sg_outsz_rb","/tmp_wbshd/sg_start","/tmp_wbshd/sg_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0); /* unlinkat(AT_FDCWD,path,0) */
}
__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/sg_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/sg_alloc","A"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/sg_phys","P"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/sg_devmem","D"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/sg_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;
  if(sen_dma_transfer(0,PA,phys,CP_PAGE)){ wr("/tmp_wbshd/sg_read","R"); goto out; }
  /* verify every stock value */
  for(u32 i=0;i<NPATCH;i++){
    u32 off=(u32)(P[i].addr - PA);
    u16 cv=ld16(b,off);
    if(cv!=P[i].old_v && cv!=P[i].new_v){ wr("/tmp_wbshd/sg_mismatch","!"); goto out; }
  }
  /* keep mode id check (0x8c0 at +0x24) */
  if(ld16(b,(u32)(0x20d9c08UL-PA))!=0x08c0){ wr("/tmp_wbshd/sg_modeid","E"); goto out; }
  /* apply */
  for(u32 i=0;i<NPATCH;i++) st16(b,(u32)(P[i].addr-PA),P[i].new_v);
  if(sen_dma_transfer(1,phys,PA,CP_PAGE)){ wr("/tmp_wbshd/sg_write","W"); goto out; }
  /* read back */
  if(sen_dma_transfer(0,PA,phys,CP_PAGE)){ wr("/tmp_wbshd/sg_rb","B"); goto out; }
  { int ok=1;
    for(u32 i=0;i<NPATCH;i++) if(ld16(b,(u32)(P[i].addr-PA))!=P[i].new_v) ok=0;
    if(ld16(b,(u32)(0x20d9c08UL-PA))!=0x08c0) ok=0;
    wr(ok?"/tmp_wbshd/sg_ok":"/tmp_wbshd/sg_bad", ok?"OK":"BAD"); }
out:
  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); wr("/tmp_wbshd/sg_done","D");
}
