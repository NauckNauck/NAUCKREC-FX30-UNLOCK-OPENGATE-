/* SONY FX30 OPEN GATE 3:2 (3840x2560) PURE A7 IV APPROACH
 * NO TG REGISTERS, NO CLOCK REGISTERS.
 * Total: 376x u16 patches (360 Mode 3 fields + 16 scalars)
 *        9x u32 patches (2 DMM buffers + 7 LCD preview sites)
 * Volatile RAM only. Cold boot (battery pull) restores stock.
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

struct p16_entry { u32 addr; u16 oldv; u16 newv; };
static const struct p16_entry P16[] = {
  {0x00c7abb6UL,  3190,  4000}, /* 0x006 */
  {0x00c7b76eUL,  3190,  4000}, /* 0x006 */
  {0x00c7abbaUL,    26,    32}, /* 0x00a */
  {0x00c7b772UL,    26,    32}, /* 0x00a */
  {0x00c7abbeUL,  3164,  3968}, /* 0x00e */
  {0x00c7b776UL,  3164,  3968}, /* 0x00e */
  {0x00c7abc2UL,    30,    36}, /* 0x012 */
  {0x00c7b77aUL,    30,    36}, /* 0x012 */
  {0x00c7abc6UL,  3156,  3960}, /* 0x016 */
  {0x00c7b77eUL,  3156,  3960}, /* 0x016 */
  {0x00c7abdaUL,  3164,  3968}, /* 0x02a */
  {0x00c7b792UL,  3164,  3968}, /* 0x02a */
  {0x00c7abe2UL,  3164,  3968}, /* 0x032 */
  {0x00c7b79aUL,  3164,  3968}, /* 0x032 */
  {0x00c7abeaUL,  3156,  3960}, /* 0x03a */
  {0x00c7b7a2UL,  3156,  3960}, /* 0x03a */
  {0x00c7abfeUL,  3164,  3968}, /* 0x04e */
  {0x00c7b7b6UL,  3164,  3968}, /* 0x04e */
  {0x00c7ac06UL,  3164,  3968}, /* 0x056 */
  {0x00c7b7beUL,  3164,  3968}, /* 0x056 */
  {0x00c7ac0eUL,  3156,  3960}, /* 0x05e */
  {0x00c7b7c6UL,  3156,  3960}, /* 0x05e */
  {0x00c7ac16UL,  3164,  3968}, /* 0x066 */
  {0x00c7b7ceUL,  3164,  3968}, /* 0x066 */
  {0x00c7ac1eUL,  3164,  3968}, /* 0x06e */
  {0x00c7b7d6UL,  3164,  3968}, /* 0x06e */
  {0x00c7ac26UL,  3156,  3960}, /* 0x076 */
  {0x00c7b7deUL,  3156,  3960}, /* 0x076 */
  {0x00c7ac3aUL,  3190,  4000}, /* 0x08a */
  {0x00c7b7f2UL,  3190,  4000}, /* 0x08a */
  {0x00c7ac3eUL,    26,    32}, /* 0x08e */
  {0x00c7b7f6UL,    26,    32}, /* 0x08e */
  {0x00c7ac42UL,  3164,  3968}, /* 0x092 */
  {0x00c7b7faUL,  3164,  3968}, /* 0x092 */
  {0x00c7ac46UL,    30,    36}, /* 0x096 */
  {0x00c7b7feUL,    30,    36}, /* 0x096 */
  {0x00c7ac4aUL,  3156,  3960}, /* 0x09a */
  {0x00c7b802UL,  3156,  3960}, /* 0x09a */
  {0x00c7ac5eUL,  3164,  3968}, /* 0x0ae */
  {0x00c7b816UL,  3164,  3968}, /* 0x0ae */
  {0x00c7ac66UL,  3164,  3968}, /* 0x0b6 */
  {0x00c7b81eUL,  3164,  3968}, /* 0x0b6 */
  {0x00c7ac6eUL,  3156,  3960}, /* 0x0be */
  {0x00c7b826UL,  3156,  3960}, /* 0x0be */
  {0x00c7ac76UL,  3164,  3968}, /* 0x0c6 */
  {0x00c7b82eUL,  3164,  3968}, /* 0x0c6 */
  {0x00c7ac7eUL,  3164,  3968}, /* 0x0ce */
  {0x00c7b836UL,  3164,  3968}, /* 0x0ce */
  {0x00c7ac86UL,  3156,  3960}, /* 0x0d6 */
  {0x00c7b83eUL,  3156,  3960}, /* 0x0d6 */
  {0x00c7ac8eUL,  3164,  3968}, /* 0x0de */
  {0x00c7b846UL,  3164,  3968}, /* 0x0de */
  {0x00c7ac96UL,  3164,  3968}, /* 0x0e6 */
  {0x00c7b84eUL,  3164,  3968}, /* 0x0e6 */
  {0x00c7ac9eUL,  3156,  3960}, /* 0x0ee */
  {0x00c7b856UL,  3156,  3960}, /* 0x0ee */
  {0x00c7acaeUL,  3164,  3968}, /* 0x0fe */
  {0x00c7b866UL,  3164,  3968}, /* 0x0fe */
  {0x00c7acb6UL,  3164,  3968}, /* 0x106 */
  {0x00c7b86eUL,  3164,  3968}, /* 0x106 */
  {0x00c7acbeUL,  3156,  3960}, /* 0x10e */
  {0x00c7b876UL,  3156,  3960}, /* 0x10e */
  {0x00c7acc6UL,  3164,  3968}, /* 0x116 */
  {0x00c7b87eUL,  3164,  3968}, /* 0x116 */
  {0x00c7acceUL,  3164,  3968}, /* 0x11e */
  {0x00c7b886UL,  3164,  3968}, /* 0x11e */
  {0x00c7acd6UL,  3156,  3960}, /* 0x126 */
  {0x00c7b88eUL,  3156,  3960}, /* 0x126 */
  {0x00c7acdeUL,  3164,  3968}, /* 0x12e */
  {0x00c7b896UL,  3164,  3968}, /* 0x12e */
  {0x00c7ace6UL,  3164,  3968}, /* 0x136 */
  {0x00c7b89eUL,  3164,  3968}, /* 0x136 */
  {0x00c7aceeUL,  3156,  3960}, /* 0x13e */
  {0x00c7b8a6UL,  3156,  3960}, /* 0x13e */
  {0x00c7acfaUL,  3164,  3968}, /* 0x14a */
  {0x00c7b8b2UL,  3164,  3968}, /* 0x14a */
  {0x00c7ad02UL,  3164,  3968}, /* 0x152 */
  {0x00c7b8baUL,  3164,  3968}, /* 0x152 */
  {0x00c7ad0aUL,  3156,  3960}, /* 0x15a */
  {0x00c7b8c2UL,  3156,  3960}, /* 0x15a */
  {0x00c7ad12UL,  3164,  3968}, /* 0x162 */
  {0x00c7b8caUL,  3164,  3968}, /* 0x162 */
  {0x00c7ad1aUL,  3164,  3968}, /* 0x16a */
  {0x00c7b8d2UL,  3164,  3968}, /* 0x16a */
  {0x00c7ad22UL,  3156,  3960}, /* 0x172 */
  {0x00c7b8daUL,  3156,  3960}, /* 0x172 */
  {0x00c7ad8eUL,    68,    84}, /* 0x1de */
  {0x00c7b946UL,    68,    84}, /* 0x1de */
  {0x00c7ad96UL,    64,    80}, /* 0x1e6 */
  {0x00c7b94eUL,    64,    80}, /* 0x1e6 */
  {0x00c7ad9eUL,    64,    80}, /* 0x1ee */
  {0x00c7b956UL,    64,    80}, /* 0x1ee */
  {0x00c7adb8UL,    64,    80}, /* 0x208 */
  {0x00c7b970UL,    64,    80}, /* 0x208 */
  {0x00c7adc0UL,    64,    80}, /* 0x210 */
  {0x00c7b978UL,    64,    80}, /* 0x210 */
  {0x00c7adc8UL,    64,    80}, /* 0x218 */
  {0x00c7b980UL,    64,    80}, /* 0x218 */
  {0x00c7ade8UL,  3190,  4000}, /* 0x238 */
  {0x00c7b9a0UL,  3190,  4000}, /* 0x238 */
  {0x00c7ae40UL,    84,   100}, /* 0x290 */
  {0x00c7b9f8UL,    84,   100}, /* 0x290 */
  {0x00c7ae50UL,  3309,  3917}, /* 0x2a0 */
  {0x00c7ba08UL,  3309,  3917}, /* 0x2a0 */
  {0x00c7ae66UL,   144,   166}, /* 0x2b6 */
  {0x00c7ba1eUL,   144,   166}, /* 0x2b6 */
  {0x00c7aeb0UL,  3308,  4134}, /* 0x300 */
  {0x00c7ba68UL,  3308,  4134}, /* 0x300 */
  {0x00c7aebcUL,  3307,  4133}, /* 0x30c */
  {0x00c7ba74UL,  3307,  4133}, /* 0x30c */
  {0x00c7aec0UL,   118,   134}, /* 0x310 */
  {0x00c7ba78UL,   118,   134}, /* 0x310 */
  {0x00c7aec4UL,   143,   165}, /* 0x314 */
  {0x00c7ba7cUL,   143,   165}, /* 0x314 */
  {0x00c7aec8UL,   144,   166}, /* 0x318 */
  {0x00c7ba80UL,   144,   166}, /* 0x318 */
  {0x00c7aeccUL,  3307,  4133}, /* 0x31c */
  {0x00c7ba84UL,  3307,  4133}, /* 0x31c */
  {0x00c7aed0UL,   118,   134}, /* 0x320 */
  {0x00c7ba88UL,   118,   134}, /* 0x320 */
  {0x00c7aedeUL,  3307,  4133}, /* 0x32e */
  {0x00c7ba96UL,  3307,  4133}, /* 0x32e */
  {0x00c7af34UL,  3189,  3999}, /* 0x384 */
  {0x00c7baecUL,  3189,  3999}, /* 0x384 */
  {0x00c7af3cUL,    25,    31}, /* 0x38c */
  {0x00c7baf4UL,    25,    31}, /* 0x38c */
  {0x00c7af40UL,    26,    32}, /* 0x390 */
  {0x00c7baf8UL,    26,    32}, /* 0x390 */
  {0x00c7af44UL,  3189,  3999}, /* 0x394 */
  {0x00c7bafcUL,  3189,  3999}, /* 0x394 */
  {0x00c7af4cUL,  3189,  3999}, /* 0x39c */
  {0x00c7bb04UL,  3189,  3999}, /* 0x39c */
  {0x00c7af50UL,  3189,  3999}, /* 0x3a0 */
  {0x00c7bb08UL,  3189,  3999}, /* 0x3a0 */
  {0x00c7afacUL,  3196,  4006}, /* 0x3fc */
  {0x00c7bb64UL,  3196,  4006}, /* 0x3fc */
  {0x00c7afb4UL,  3189,  3999}, /* 0x404 */
  {0x00c7bb6cUL,  3189,  3999}, /* 0x404 */
  {0x00c7afb8UL,    26,    32}, /* 0x408 */
  {0x00c7bb70UL,    26,    32}, /* 0x408 */
  {0x00c7afbcUL,  3189,  3999}, /* 0x40c */
  {0x00c7bb74UL,  3189,  3999}, /* 0x40c */
  {0x00c7afc0UL,    26,    32}, /* 0x410 */
  {0x00c7bb78UL,    26,    32}, /* 0x410 */
  {0x00c7afc4UL,  3189,  3999}, /* 0x414 */
  {0x00c7bb7cUL,  3189,  3999}, /* 0x414 */
  {0x00c7afc8UL,    26,    32}, /* 0x418 */
  {0x00c7bb80UL,    26,    32}, /* 0x418 */
  {0x00c7afccUL,  3189,  3999}, /* 0x41c */
  {0x00c7bb84UL,  3189,  3999}, /* 0x41c */
  {0x00c7afd0UL,    26,    32}, /* 0x420 */
  {0x00c7bb88UL,    26,    32}, /* 0x420 */
  {0x00c7afd4UL,  3189,  3999}, /* 0x424 */
  {0x00c7bb8cUL,  3189,  3999}, /* 0x424 */
  {0x00c7afdcUL,  3196,  4006}, /* 0x42c */
  {0x00c7bb94UL,  3196,  4006}, /* 0x42c */
  {0x00c7afe0UL,  3196,  4006}, /* 0x430 */
  {0x00c7bb98UL,  3196,  4006}, /* 0x430 */
  {0x00c7afe4UL,  3191,  4001}, /* 0x434 */
  {0x00c7bb9cUL,  3191,  4001}, /* 0x434 */
  {0x00c7afe8UL,  3191,  4001}, /* 0x438 */
  {0x00c7bba0UL,  3191,  4001}, /* 0x438 */
  {0x00c7afecUL,  3191,  4001}, /* 0x43c */
  {0x00c7bba4UL,  3191,  4001}, /* 0x43c */
  {0x00c7aff0UL,  3191,  4001}, /* 0x440 */
  {0x00c7bba8UL,  3191,  4001}, /* 0x440 */
  {0x00c7aff4UL,    26,    32}, /* 0x444 */
  {0x00c7bbacUL,    26,    32}, /* 0x444 */
  {0x00c7aff8UL,  3189,  3999}, /* 0x448 */
  {0x00c7bbb0UL,  3189,  3999}, /* 0x448 */
  {0x00c7affcUL,  3196,  4006}, /* 0x44c */
  {0x00c7bbb4UL,  3196,  4006}, /* 0x44c */
  {0x00c7b000UL,  3196,  4006}, /* 0x450 */
  {0x00c7bbb8UL,  3196,  4006}, /* 0x450 */
  {0x00c7b03eUL,  3160,  3964}, /* 0x48e */
  {0x00c7bbf6UL,  3160,  3964}, /* 0x48e */
  {0x00c7b048UL,    13,    16}, /* 0x498 */
  {0x00c7bc00UL,    13,    16}, /* 0x498 */
  {0x00c7b04cUL,  1595,  2000}, /* 0x49c */
  {0x00c7bc04UL,  1595,  2000}, /* 0x49c */
  {0x00c7b058UL,    13,    16}, /* 0x4a8 */
  {0x00c7bc10UL,    13,    16}, /* 0x4a8 */
  {0x00c7b05cUL,  1595,  2000}, /* 0x4ac */
  {0x00c7bc14UL,  1595,  2000}, /* 0x4ac */
  {0x00c7b068UL,    13,    16}, /* 0x4b8 */
  {0x00c7bc20UL,    13,    16}, /* 0x4b8 */
  {0x00c7b06cUL,  1595,  2000}, /* 0x4bc */
  {0x00c7bc24UL,  1595,  2000}, /* 0x4bc */
  {0x00c7b070UL,  3164,  3968}, /* 0x4c0 */
  {0x00c7bc28UL,  3164,  3968}, /* 0x4c0 */
  {0x00c7b078UL,  3163,  3967}, /* 0x4c8 */
  {0x00c7bc30UL,  3163,  3967}, /* 0x4c8 */
  {0x00c7b080UL,  3163,  3967}, /* 0x4d0 */
  {0x00c7bc38UL,  3163,  3967}, /* 0x4d0 */
  {0x00c7b088UL,  3164,  3968}, /* 0x4d8 */
  {0x00c7bc40UL,  3164,  3968}, /* 0x4d8 */
  {0x00c7b090UL,  3163,  3967}, /* 0x4e0 */
  {0x00c7bc48UL,  3163,  3967}, /* 0x4e0 */
  {0x00c7b098UL,  3163,  3967}, /* 0x4e8 */
  {0x00c7bc50UL,  3163,  3967}, /* 0x4e8 */
  {0x00c7b0a8UL,  3164,  3968}, /* 0x4f8 */
  {0x00c7bc60UL,  3164,  3968}, /* 0x4f8 */
  {0x00c7b0acUL,  3164,  3968}, /* 0x4fc */
  {0x00c7bc64UL,  3164,  3968}, /* 0x4fc */
  {0x00c7b100UL,  3179,  3983}, /* 0x550 */
  {0x00c7bcb8UL,  3179,  3983}, /* 0x550 */
  {0x00c7b108UL,  3163,  3967}, /* 0x558 */
  {0x00c7bcc0UL,  3163,  3967}, /* 0x558 */
  {0x00c7b110UL,  3163,  3967}, /* 0x560 */
  {0x00c7bcc8UL,  3163,  3967}, /* 0x560 */
  {0x00c7b114UL,  3166,  3970}, /* 0x564 */
  {0x00c7bcccUL,  3166,  3970}, /* 0x564 */
  {0x00c7b118UL,  3165,  3969}, /* 0x568 */
  {0x00c7bcd0UL,  3165,  3969}, /* 0x568 */
  {0x00c7b11cUL,  3165,  3969}, /* 0x56c */
  {0x00c7bcd4UL,  3165,  3969}, /* 0x56c */
  {0x00c7b120UL,  3179,  3983}, /* 0x570 */
  {0x00c7bcd8UL,  3179,  3983}, /* 0x570 */
  {0x00c7b132UL,  3150,  3954}, /* 0x582 */
  {0x00c7bceaUL,  3150,  3954}, /* 0x582 */
  {0x00c7b140UL,   791,   992}, /* 0x590 */
  {0x00c7bcf8UL,   791,   992}, /* 0x590 */
  {0x00c7b144UL,   791,   992}, /* 0x594 */
  {0x00c7bcfcUL,   791,   992}, /* 0x594 */
  {0x00c7b1dcUL,  3164,  3968}, /* 0x62c */
  {0x00c7bd94UL,  3164,  3968}, /* 0x62c */
  {0x00c7b1e4UL,  3163,  3967}, /* 0x634 */
  {0x00c7bd9cUL,  3163,  3967}, /* 0x634 */
  {0x00c7b1ecUL,  3163,  3967}, /* 0x63c */
  {0x00c7bda4UL,  3163,  3967}, /* 0x63c */
  {0x00c7b1f4UL,  3163,  3967}, /* 0x644 */
  {0x00c7bdacUL,  3163,  3967}, /* 0x644 */
  {0x00c7b1fcUL,  3163,  3967}, /* 0x64c */
  {0x00c7bdb4UL,  3163,  3967}, /* 0x64c */
  {0x00c7b200UL,  3164,  3968}, /* 0x650 */
  {0x00c7bdb8UL,  3164,  3968}, /* 0x650 */
  {0x00c7b208UL,  3164,  3968}, /* 0x658 */
  {0x00c7bdc0UL,  3164,  3968}, /* 0x658 */
  {0x00c7b210UL,  3164,  3968}, /* 0x660 */
  {0x00c7bdc8UL,  3164,  3968}, /* 0x660 */
  {0x00c7b218UL,  3163,  3967}, /* 0x668 */
  {0x00c7bdd0UL,  3163,  3967}, /* 0x668 */
  {0x00c7b248UL,  1584,  1986}, /* 0x698 */
  {0x00c7be00UL,  1584,  1986}, /* 0x698 */
  {0x00c7b264UL,  1582,  1984}, /* 0x6b4 */
  {0x00c7be1cUL,  1582,  1984}, /* 0x6b4 */
  {0x00c7b26cUL,  1582,  1984}, /* 0x6bc */
  {0x00c7be24UL,  1582,  1984}, /* 0x6bc */
  {0x00c7b278UL,  3177,  3981}, /* 0x6c8 */
  {0x00c7be30UL,  3177,  3981}, /* 0x6c8 */
  {0x00c7b280UL,  3163,  3967}, /* 0x6d0 */
  {0x00c7be38UL,  3163,  3967}, /* 0x6d0 */
  {0x00c7b288UL,  3163,  3967}, /* 0x6d8 */
  {0x00c7be40UL,  3163,  3967}, /* 0x6d8 */
  {0x00c7b28cUL,  3168,  3972}, /* 0x6dc */
  {0x00c7be44UL,  3168,  3972}, /* 0x6dc */
  {0x00c7b290UL,  3165,  3969}, /* 0x6e0 */
  {0x00c7be48UL,  3165,  3969}, /* 0x6e0 */
  {0x00c7b294UL,  3164,  3968}, /* 0x6e4 */
  {0x00c7be4cUL,  3164,  3968}, /* 0x6e4 */
  {0x00c7b2baUL,  3164,  3968}, /* 0x70a */
  {0x00c7be72UL,  3164,  3968}, /* 0x70a */
  {0x00c7b2caUL,  3164,  3968}, /* 0x71a */
  {0x00c7be82UL,  3164,  3968}, /* 0x71a */
  {0x00c7b2d4UL,  3170,  3974}, /* 0x724 */
  {0x00c7be8cUL,  3170,  3974}, /* 0x724 */
  {0x00c7b2dcUL,  3163,  3967}, /* 0x72c */
  {0x00c7be94UL,  3163,  3967}, /* 0x72c */
  {0x00c7b2e4UL,  3163,  3967}, /* 0x734 */
  {0x00c7be9cUL,  3163,  3967}, /* 0x734 */
  {0x00c7b2e8UL,  3164,  3968}, /* 0x738 */
  {0x00c7bea0UL,  3164,  3968}, /* 0x738 */
  {0x00c7b2ecUL,  3166,  3970}, /* 0x73c */
  {0x00c7bea4UL,  3166,  3970}, /* 0x73c */
  {0x00c7b302UL,  3164,  3968}, /* 0x752 */
  {0x00c7bebaUL,  3164,  3968}, /* 0x752 */
  {0x00c7b30eUL,  3164,  3968}, /* 0x75e */
  {0x00c7bec6UL,  3164,  3968}, /* 0x75e */
  {0x00c7b32cUL,  3164,  3968}, /* 0x77c */
  {0x00c7bee4UL,  3164,  3968}, /* 0x77c */
  {0x00c7b334UL,  5302,  6649}, /* 0x784 */
  {0x00c7beecUL,  5302,  6649}, /* 0x784 */
  {0x00c7b33cUL,  6733,  3287}, /* 0x78c */
  {0x00c7bef4UL,  6733,  3287}, /* 0x78c */
  {0x00c7b50eUL,   502,     0}, /* 0x95e */
  {0x00c7c0c6UL,   502,     0}, /* 0x95e */
  {0x00c7b58cUL,  3174,  3978}, /* 0x9dc */
  {0x00c7c144UL,  3174,  3978}, /* 0x9dc */
  {0x00c7b594UL,  3163,  3967}, /* 0x9e4 */
  {0x00c7c14cUL,  3163,  3967}, /* 0x9e4 */
  {0x00c7b59cUL,  3163,  3967}, /* 0x9ec */
  {0x00c7c154UL,  3163,  3967}, /* 0x9ec */
  {0x00c7b5a4UL,  3163,  3967}, /* 0x9f4 */
  {0x00c7c15cUL,  3163,  3967}, /* 0x9f4 */
  {0x00c7b5acUL,  3163,  3967}, /* 0x9fc */
  {0x00c7c164UL,  3163,  3967}, /* 0x9fc */
  {0x00c7b5b0UL,  3174,  3978}, /* 0xa00 */
  {0x00c7c168UL,  3174,  3978}, /* 0xa00 */
  {0x00c7b5b8UL,  3174,  3978}, /* 0xa08 */
  {0x00c7c170UL,  3174,  3978}, /* 0xa08 */
  {0x00c7b5c0UL,  3164,  3968}, /* 0xa10 */
  {0x00c7c178UL,  3164,  3968}, /* 0xa10 */
  {0x00c7b5c8UL,  3173,  3977}, /* 0xa18 */
  {0x00c7c180UL,  3173,  3977}, /* 0xa18 */
  {0x00c7b5f8UL,  1584,  1986}, /* 0xa48 */
  {0x00c7c1b0UL,  1584,  1986}, /* 0xa48 */
  {0x00c7b614UL,  1582,  1984}, /* 0xa64 */
  {0x00c7c1ccUL,  1582,  1984}, /* 0xa64 */
  {0x00c7b61cUL,  1582,  1984}, /* 0xa6c */
  {0x00c7c1d4UL,  1582,  1984}, /* 0xa6c */
  {0x00c7b62cUL,  3174,  3978}, /* 0xa7c */
  {0x00c7c1e4UL,  3174,  3978}, /* 0xa7c */
  {0x00c7b634UL,  3163,  3967}, /* 0xa84 */
  {0x00c7c1ecUL,  3163,  3967}, /* 0xa84 */
  {0x00c7b63cUL,  3163,  3967}, /* 0xa8c */
  {0x00c7c1f4UL,  3163,  3967}, /* 0xa8c */
  {0x00c7b644UL,  3163,  3967}, /* 0xa94 */
  {0x00c7c1fcUL,  3163,  3967}, /* 0xa94 */
  {0x00c7b64cUL,  3163,  3967}, /* 0xa9c */
  {0x00c7c204UL,  3163,  3967}, /* 0xa9c */
  {0x00c7b650UL,  3174,  3978}, /* 0xaa0 */
  {0x00c7c208UL,  3174,  3978}, /* 0xaa0 */
  {0x00c7b658UL,  3174,  3978}, /* 0xaa8 */
  {0x00c7c210UL,  3174,  3978}, /* 0xaa8 */
  {0x00c7b660UL,  3164,  3968}, /* 0xab0 */
  {0x00c7c218UL,  3164,  3968}, /* 0xab0 */
  {0x00c7b668UL,  3173,  3977}, /* 0xab8 */
  {0x00c7c220UL,  3173,  3977}, /* 0xab8 */
  {0x00c7b698UL,  1584,  1986}, /* 0xae8 */
  {0x00c7c250UL,  1584,  1986}, /* 0xae8 */
  {0x00c7b6b4UL,  1582,  1984}, /* 0xb04 */
  {0x00c7c26cUL,  1582,  1984}, /* 0xb04 */
  {0x00c7b6bcUL,  1582,  1984}, /* 0xb0c */
  {0x00c7c274UL,  1582,  1984}, /* 0xb0c */
  {0x00c7b6ccUL,  3192,  3996}, /* 0xb1c */
  {0x00c7c284UL,  3192,  3996}, /* 0xb1c */
  {0x00c7b6d4UL,  3163,  3967}, /* 0xb24 */
  {0x00c7c28cUL,  3163,  3967}, /* 0xb24 */
  {0x00c7b6dcUL,  3163,  3967}, /* 0xb2c */
  {0x00c7c294UL,  3163,  3967}, /* 0xb2c */
  {0x00c7b6e4UL,  3163,  3967}, /* 0xb34 */
  {0x00c7c29cUL,  3163,  3967}, /* 0xb34 */
  {0x00c7b6ecUL,  3163,  3967}, /* 0xb3c */
  {0x00c7c2a4UL,  3163,  3967}, /* 0xb3c */
  {0x00c7b6f0UL,  3192,  3996}, /* 0xb40 */
  {0x00c7c2a8UL,  3192,  3996}, /* 0xb40 */
  {0x00c7b6f4UL,  3181,  3985}, /* 0xb44 */
  {0x00c7c2acUL,  3181,  3985}, /* 0xb44 */
  {0x00c7b6f8UL,  3192,  3996}, /* 0xb48 */
  {0x00c7c2b0UL,  3192,  3996}, /* 0xb48 */
  {0x00c7b700UL,  3164,  3968}, /* 0xb50 */
  {0x00c7c2b8UL,  3164,  3968}, /* 0xb50 */
  {0x00c7b708UL,  3191,  3995}, /* 0xb58 */
  {0x00c7c2c0UL,  3191,  3995}, /* 0xb58 */
  {0x00c7b738UL,  1584,  1986}, /* 0xb88 */
  {0x00c7c2f0UL,  1584,  1986}, /* 0xb88 */
  {0x00c7b754UL,  1582,  1984}, /* 0xba4 */
  {0x00c7c30cUL,  1582,  1984}, /* 0xba4 */
  {0x00c7b75cUL,  1582,  1984}, /* 0xbac */
  {0x00c7c314UL,  1582,  1984}, /* 0xbac */
  {0x00e027f0UL,  3348,  3968}, /* S1 */
  {0x00e03290UL,    41,    47}, /* S9_r0 */
  {0x00e03298UL,  1704,  2020}, /* S3_r0 */
  {0x00e032acUL,    41,    47}, /* S9_r2 */
  {0x00e032b4UL,  1704,  2020}, /* S3_r2 */
  {0x00e032baUL,    41,    47}, /* S9_r3 */
  {0x00e032c2UL,  1704,  2020}, /* S3_r3 */
  {0x00e032c8UL,    41,    47}, /* S9_r4 */
  {0x00e032d0UL,  1704,  2020}, /* S3_r4 */
  {0x00e032d6UL,    41,    47}, /* S9_r5 */
  {0x00e032deUL,  1704,  2020}, /* S3_r5 */
  {0x00e030f0UL,   502,     0}, /* S7 */
  {0x00c9b308UL,   502,     0}, /* S2 */
  {0x00c365d4UL,   502,     0}, /* S4 */
  {0x020d9c4aUL,  3348,  3968}, /* DrvParam6 */
  {0x0155d446UL,  2160,  2560}, /* outsz */
};
#define NP16 (sizeof(P16)/sizeof(P16[0]))

struct p32_entry { u32 addr; u32 oldv; u32 newv; };
static const struct p32_entry P32[] = {
  {0x02080860UL, 0x01788700UL, 0x01bf0000UL}, /* DMM0 */
  {0x02080874UL, 0x01788700UL, 0x01bf0000UL}, /* DMM1 */
  {0x001e2d3cUL, 0x00000003UL, 0x00000000UL}, /* LCD0 */
  {0x00de791cUL, 0x00260000UL, 0x00000000UL}, /* LCD1 */
  {0x00de7920UL, 0x01940280UL, 0x01e00280UL}, /* LCD2 */
  {0x00de7994UL, 0x00260000UL, 0x00000000UL}, /* LCD3 */
  {0x00de7998UL, 0x01940280UL, 0x01e00280UL}, /* LCD4 */
  {0x00de7a0cUL, 0x00260000UL, 0x00000000UL}, /* LCD5 */
  {0x00de7a10UL, 0x01940280UL, 0x01e00280UL}, /* LCD6 */
};
#define NP32 (sizeof(P32)/sizeof(P32[0]))

static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/og_start","/tmp_wbshd/og_ok","/tmp_wbshd/og_bad",
                   "/tmp_wbshd/og_done","/tmp_wbshd/hg_start","/tmp_wbshd/hg_ok","/tmp_wbshd/hg_bad","/tmp_wbshd/hg_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/og_start","S");
  wr("/tmp_wbshd/hg_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/og_alloc_fail","A"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/og_phys_bad","P"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/og_devmem_fail","D"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/og_mmap_fail","M"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  /* Collect distinct pages */
  u64 pages[32]; u32 np=0;
  for(u32 i=0;i<NP32;i++){ u64 pg=P32[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<32) pages[np++]=pg; }
  for(u32 i=0;i<NP16;i++){ u64 pg=P16[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<32) pages[np++]=pg; }

  int all_ok = 1;
  for(u32 p=0; p<np; p++){
    u64 cur_pg = pages[p];
    /* 1. Read page from CP */
    if(sen_dma_transfer(0, cur_pg, phys, CP_PAGE) != 0){ all_ok = 0; break; }

    /* 2. Apply u16 patches for this page */
    for(u32 i=0; i<NP16; i++){
      if((P16[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P16[i].addr & 0xfffUL;
        u16 nv = P16[i].newv;
        b[off] = (u8)nv;
        b[off+1] = (u8)(nv >> 8);
      }
    }
    /* 3. Apply u32 patches for this page */
    for(u32 i=0; i<NP32; i++){
      if((P32[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P32[i].addr & 0xfffUL;
        u32 nv = P32[i].newv;
        b[off]   = (u8)nv;
        b[off+1] = (u8)(nv >> 8);
        b[off+2] = (u8)(nv >> 16);
        b[off+3] = (u8)(nv >> 24);
      }
    }

    /* 4. Write page back to CP */
    if(sen_dma_transfer(1, phys, cur_pg, CP_PAGE) != 0){ all_ok = 0; break; }

    /* 5. Verify by reading back */
    if(sen_dma_transfer(0, cur_pg, phys, CP_PAGE) != 0){ all_ok = 0; break; }
    for(u32 i=0; i<NP16; i++){
      if((P16[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P16[i].addr & 0xfffUL;
        u16 cv = (u16)(b[off] | (b[off+1] << 8));
        if(cv != P16[i].newv) { all_ok = 0; break; }
      }
    }
    for(u32 i=0; i<NP32; i++){
      if((P32[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P32[i].addr & 0xfffUL;
        u32 cv = (u32)(b[off] | (b[off+1] << 8) | (b[off+2] << 16) | (b[off+3] << 24));
        if(cv != P32[i].newv) { all_ok = 0; break; }
      }
    }
    if(!all_ok) break;
  }

  wr(all_ok ? "/tmp_wbshd/og_ok" : "/tmp_wbshd/og_bad", all_ok ? "OK" : "BAD");
  wr(all_ok ? "/tmp_wbshd/hg_ok" : "/tmp_wbshd/hg_bad", all_ok ? "OK" : "BAD");
  sc6(215,m,(long)maplen,0,0,0,0);
  sc6(57,fd,0,0,0,0,0);
  cam_dmm_free(handle);
  wr("/tmp_wbshd/og_done","D");
  wr("/tmp_wbshd/hg_done","D");
}
