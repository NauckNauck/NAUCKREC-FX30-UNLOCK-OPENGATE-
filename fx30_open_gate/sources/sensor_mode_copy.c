/* FX30 Open Gate — strategy B: copy the native 3:2 mode entry (25) over the 4K mode entry (6),
 * then restore the 4K mode id 0x8c0. This brings BOTH the geometry and the timing of the
 * Sony-validated 3:2 mode, avoiding the 4K timing/3:2 geometry mismatch.
 *
 * source: entry 25  @ 0x20db604  (352 bytes)
 * dest  : entry 6   @ 0x20d9be4  (352 bytes)
 * mode id kept: 0x20d9c08 = 0x8c0
 *
 * RAM only, volatile. Build with zig cc (aarch64), link libsencore.so.
 */
typedef unsigned long u64; typedef long i64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");
extern u64 cam_dmm_free(u32 handle) __asm__("_Z8dmm_freej");
extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);

#define CP_PAGE 4096U
#define SRC 0x20db604UL     /* entry 25 (3:2) */
#define DST 0x20d9be4UL     /* entry 6 (4K) */
#define LEN 0x160U          /* 352 bytes */
#define MODEID_OFF 0x24U    /* within the entry */
#define MODEID_VAL 0x08c0U

static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory"); return x0; }
static void wr(const char*p,const char*m){ long n=0; while(m[n]) n++;
  long fd=sc6(56,-100,(long)p,0x241,0644,0,0); if(fd>=0){ sc6(64,fd,(long)m,n,0,0,0); sc6(57,fd,0,0,0,0,0);} }

/* read a 4K page from CP into the buffer, then optionally write it back */
static u8 *map_page(u64 phys, u64 *lead_out, u64 *maplen_out){
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0) return 0;
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ sc6(57,fd,0,0,0,0,0); return 0; }
  *lead_out=lead; *maplen_out=maplen;
  return (u8*)(m+lead);   /* NB: the caller must unmap through the saved maplen; see below */
}
/* We avoid the helper-above complexity and inline the two pages directly. */
__attribute__((constructor)) static void go(void){
  wr("/tmp_wbshd/sb_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/sb_alloc","A"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/sb_phys","P"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/sb_devmem","D"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/sb_mmap","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;
  u8 src[0x160];
  /* read source entry 25 */
  if(sen_dma_transfer(0, SRC & ~0xfffUL, phys, CP_PAGE)){ wr("/tmp_wbshd/sb_rd_src","1"); goto out; }
  { u32 off=(u32)(SRC & 0xfffUL); for(u32 i=0;i<LEN;i++) src[i]=b[off+i]; }
  /* write it over entry 6 */
  if(sen_dma_transfer(0, DST & ~0xfffUL, phys, CP_PAGE)){ wr("/tmp_wbshd/sb_rd_dst","2"); goto out; }
  { u32 off=(u32)(DST & 0xfffUL); for(u32 i=0;i<LEN;i++) b[off+i]=src[i]; }
  /* restore the 4K mode id */
  { u32 off=(u32)((DST & 0xfffUL)+MODEID_OFF); b[off]=(u8)(MODEID_VAL&0xff); b[off+1]=(u8)(MODEID_VAL>>8); }
  if(sen_dma_transfer(1, DST & ~0xfffUL, phys, CP_PAGE)){ wr("/tmp_wbshd/sb_write","W"); goto out; }
  /* read back */
  if(sen_dma_transfer(0, DST & ~0xfffUL, phys, CP_PAGE)){ wr("/tmp_wbshd/sb_rb","B"); goto out; }
  { u32 off=(u32)((DST & 0xfffUL)+MODEID_OFF);
    u16 id=(u16)(b[off] | (b[off+1]<<8));
    wr(id==MODEID_VAL?"/tmp_wbshd/sb_ok":"/tmp_wbshd/sb_bad", id==MODEID_VAL?"OK":"BAD"); }
  /* --- outsz: 0x155d446  0x0870 -> 0x0a00 (3840x2560) --- */
  { u32 h2=0; u64 ph2=cam_dmm_alloc(CP_PAGE,&h2);
    if(ph2){
      u64 lead2=ph2&0xfff, maplen2=(lead2+CP_PAGE+0xfffUL)&~0xfffUL;
      i64 m2=sc6(222,0,(long)maplen2,3,1,fd,(long)(ph2-lead2));
      if(m2>=0){
        u8 *b2=(u8*)m2+lead2;
        u32 off=(u32)(0x155d446UL-0x155d000UL);
        if(sen_dma_transfer(0,0x155d000UL,ph2,CP_PAGE)==0){
          u16 cur=(u16)(b2[off]|(b2[off+1]<<8));
          if(cur==0x0870 || cur==0x0a00){
            b2[off]=(u8)0x00; b2[off+1]=(u8)0x0a;
            if(sen_dma_transfer(1,ph2,0x155d000UL,CP_PAGE)==0){
              if(sen_dma_transfer(0,0x155d000UL,ph2,CP_PAGE)==0){
                u16 nv=(u16)(b2[off]|(b2[off+1]<<8));
                wr(nv==0x0a00?"/tmp_wbshd/sb_outsz_ok":"/tmp_wbshd/sb_outsz_bad","OUTSZ_OK");
              }
            }
          } else wr("/tmp_wbshd/sb_outsz_mismatch","!");
        }
        sc6(215,m2,(long)maplen2,0,0,0,0);
      }
      cam_dmm_free(h2);
    } }
out:
  sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); wr("/tmp_wbshd/sb_done","D");
}
