/* SONY FX30 — STEP 1 v5: MODE 2 SQUEEZED OPEN GATE (PROBE-GUIDED FIX — NO DMMWB, SYNCED N7/N8/N10/S10/LC)
 * Changes vs v4 (derived directly from live Movie-mode probe_dump.bin):
 *   - REMOVED DMMWB 29.29 MB (stock 24.67 MB did not freeze in v1/v2; A7 IV ogsq_v3 runs without DMMWB).
 *   - Synced TG Node 10 (reg 0x19: 0x29 -> 0x2f) with S9 (41 -> 47) so sensor & ISP match.
 *   - Updated TG Node 7 (0x09 -> 0x02), Node 8 (0x22 -> 0x29, sum=43), and S10/DrvParam+0x8c (68 -> 82 = Node8*2).
 *   - Updated LC table S7 (406 -> 0), S8 (457 -> 51) and DrvParam+0x15a (457 -> 51) for Modes 2, 4, 5, 6.
 *   - Kept 27/32 ratio (+0x06/+0x07), cropV1/cropV2, height=3968, S3=2020, 180x2 Mode 2 fields, TG Nodes 3..6.
 * Volatile RAM only. Battery pull (10s) restores 100% stock.
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
  {0x00c79446UL,  3382,  4008}, /* Mode2_L 0x006 */
  {0x00c79ffeUL,  3382,  4008}, /* Mode2_R 0x006 */
  {0x00c7944aUL,    26,    32}, /* Mode2_L 0x00a */
  {0x00c7a002UL,    26,    32}, /* Mode2_R 0x00a */
  {0x00c7944eUL,  3356,  3976}, /* Mode2_L 0x00e */
  {0x00c7a006UL,  3356,  3976}, /* Mode2_R 0x00e */
  {0x00c79452UL,    30,    36}, /* Mode2_L 0x012 */
  {0x00c7a00aUL,    30,    36}, /* Mode2_R 0x012 */
  {0x00c79456UL,  3348,  3968}, /* Mode2_L 0x016 */
  {0x00c7a00eUL,  3348,  3968}, /* Mode2_R 0x016 */
  {0x00c7946aUL,  3356,  3976}, /* Mode2_L 0x02a */
  {0x00c7a022UL,  3356,  3976}, /* Mode2_R 0x02a */
  {0x00c79472UL,  3356,  3976}, /* Mode2_L 0x032 */
  {0x00c7a02aUL,  3356,  3976}, /* Mode2_R 0x032 */
  {0x00c7947aUL,  3348,  3968}, /* Mode2_L 0x03a */
  {0x00c7a032UL,  3348,  3968}, /* Mode2_R 0x03a */
  {0x00c7948eUL,  3356,  3976}, /* Mode2_L 0x04e */
  {0x00c7a046UL,  3356,  3976}, /* Mode2_R 0x04e */
  {0x00c79496UL,  3356,  3976}, /* Mode2_L 0x056 */
  {0x00c7a04eUL,  3356,  3976}, /* Mode2_R 0x056 */
  {0x00c7949eUL,  3348,  3968}, /* Mode2_L 0x05e */
  {0x00c7a056UL,  3348,  3968}, /* Mode2_R 0x05e */
  {0x00c794a6UL,  3356,  3976}, /* Mode2_L 0x066 */
  {0x00c7a05eUL,  3356,  3976}, /* Mode2_R 0x066 */
  {0x00c794aeUL,  3356,  3976}, /* Mode2_L 0x06e */
  {0x00c7a066UL,  3356,  3976}, /* Mode2_R 0x06e */
  {0x00c794b6UL,  3348,  3968}, /* Mode2_L 0x076 */
  {0x00c7a06eUL,  3348,  3968}, /* Mode2_R 0x076 */
  {0x00c794caUL,  3382,  4008}, /* Mode2_L 0x08a */
  {0x00c7a082UL,  3382,  4008}, /* Mode2_R 0x08a */
  {0x00c794ceUL,    26,    32}, /* Mode2_L 0x08e */
  {0x00c7a086UL,    26,    32}, /* Mode2_R 0x08e */
  {0x00c794d2UL,  3356,  3976}, /* Mode2_L 0x092 */
  {0x00c7a08aUL,  3356,  3976}, /* Mode2_R 0x092 */
  {0x00c794d6UL,    30,    36}, /* Mode2_L 0x096 */
  {0x00c7a08eUL,    30,    36}, /* Mode2_R 0x096 */
  {0x00c794daUL,  3348,  3968}, /* Mode2_L 0x09a */
  {0x00c7a092UL,  3348,  3968}, /* Mode2_R 0x09a */
  {0x00c794eeUL,  3356,  3976}, /* Mode2_L 0x0ae */
  {0x00c7a0a6UL,  3356,  3976}, /* Mode2_R 0x0ae */
  {0x00c794f6UL,  3356,  3976}, /* Mode2_L 0x0b6 */
  {0x00c7a0aeUL,  3356,  3976}, /* Mode2_R 0x0b6 */
  {0x00c794feUL,  3348,  3968}, /* Mode2_L 0x0be */
  {0x00c7a0b6UL,  3348,  3968}, /* Mode2_R 0x0be */
  {0x00c79506UL,  3356,  3976}, /* Mode2_L 0x0c6 */
  {0x00c7a0beUL,  3356,  3976}, /* Mode2_R 0x0c6 */
  {0x00c7950eUL,  3356,  3976}, /* Mode2_L 0x0ce */
  {0x00c7a0c6UL,  3356,  3976}, /* Mode2_R 0x0ce */
  {0x00c79516UL,  3348,  3968}, /* Mode2_L 0x0d6 */
  {0x00c7a0ceUL,  3348,  3968}, /* Mode2_R 0x0d6 */
  {0x00c7951eUL,  3356,  3976}, /* Mode2_L 0x0de */
  {0x00c7a0d6UL,  3356,  3976}, /* Mode2_R 0x0de */
  {0x00c79526UL,  3356,  3976}, /* Mode2_L 0x0e6 */
  {0x00c7a0deUL,  3356,  3976}, /* Mode2_R 0x0e6 */
  {0x00c7952eUL,  3348,  3968}, /* Mode2_L 0x0ee */
  {0x00c7a0e6UL,  3348,  3968}, /* Mode2_R 0x0ee */
  {0x00c7953eUL,  3356,  3976}, /* Mode2_L 0x0fe */
  {0x00c7a0f6UL,  3356,  3976}, /* Mode2_R 0x0fe */
  {0x00c79546UL,  3356,  3976}, /* Mode2_L 0x106 */
  {0x00c7a0feUL,  3356,  3976}, /* Mode2_R 0x106 */
  {0x00c7954eUL,  3348,  3968}, /* Mode2_L 0x10e */
  {0x00c7a106UL,  3348,  3968}, /* Mode2_R 0x10e */
  {0x00c79556UL,  3356,  3976}, /* Mode2_L 0x116 */
  {0x00c7a10eUL,  3356,  3976}, /* Mode2_R 0x116 */
  {0x00c7955eUL,  3356,  3976}, /* Mode2_L 0x11e */
  {0x00c7a116UL,  3356,  3976}, /* Mode2_R 0x11e */
  {0x00c79566UL,  3348,  3968}, /* Mode2_L 0x126 */
  {0x00c7a11eUL,  3348,  3968}, /* Mode2_R 0x126 */
  {0x00c7956eUL,  3356,  3976}, /* Mode2_L 0x12e */
  {0x00c7a126UL,  3356,  3976}, /* Mode2_R 0x12e */
  {0x00c79576UL,  3356,  3976}, /* Mode2_L 0x136 */
  {0x00c7a12eUL,  3356,  3976}, /* Mode2_R 0x136 */
  {0x00c7957eUL,  3348,  3968}, /* Mode2_L 0x13e */
  {0x00c7a136UL,  3348,  3968}, /* Mode2_R 0x13e */
  {0x00c7958aUL,  3356,  3976}, /* Mode2_L 0x14a */
  {0x00c7a142UL,  3356,  3976}, /* Mode2_R 0x14a */
  {0x00c79592UL,  3356,  3976}, /* Mode2_L 0x152 */
  {0x00c7a14aUL,  3356,  3976}, /* Mode2_R 0x152 */
  {0x00c7959aUL,  3348,  3968}, /* Mode2_L 0x15a */
  {0x00c7a152UL,  3348,  3968}, /* Mode2_R 0x15a */
  {0x00c795a2UL,  3356,  3976}, /* Mode2_L 0x162 */
  {0x00c7a15aUL,  3356,  3976}, /* Mode2_R 0x162 */
  {0x00c795aaUL,  3356,  3976}, /* Mode2_L 0x16a */
  {0x00c7a162UL,  3356,  3976}, /* Mode2_R 0x16a */
  {0x00c795b2UL,  3348,  3968}, /* Mode2_L 0x172 */
  {0x00c7a16aUL,  3348,  3968}, /* Mode2_R 0x172 */
  {0x00c7961eUL,    72,    88}, /* Mode2_L 0x1de */
  {0x00c7a1d6UL,    72,    88}, /* Mode2_R 0x1de */
  {0x00c79626UL,    68,    84}, /* Mode2_L 0x1e6 */
  {0x00c7a1deUL,    68,    84}, /* Mode2_R 0x1e6 */
  {0x00c7962eUL,    68,    84}, /* Mode2_L 0x1ee */
  {0x00c7a1e6UL,    68,    84}, /* Mode2_R 0x1ee */
  {0x00c79648UL,    68,    84}, /* Mode2_L 0x208 */
  {0x00c7a200UL,    68,    84}, /* Mode2_R 0x208 */
  {0x00c79650UL,    68,    84}, /* Mode2_L 0x210 */
  {0x00c7a208UL,    68,    84}, /* Mode2_R 0x210 */
  {0x00c79658UL,    68,    84}, /* Mode2_L 0x218 */
  {0x00c7a210UL,    68,    84}, /* Mode2_R 0x218 */
  {0x00c79678UL,  3382,  4008}, /* Mode2_L 0x238 */
  {0x00c7a230UL,  3382,  4008}, /* Mode2_R 0x238 */
  {0x00c796d0UL,    86,   102}, /* Mode2_L 0x290 */
  {0x00c7a288UL,    86,   102}, /* Mode2_R 0x290 */
  {0x00c796e0UL,  3503,  4151}, /* Mode2_L 0x2a0 */
  {0x00c7a298UL,  3503,  4151}, /* Mode2_R 0x2a0 */
  {0x00c796f6UL,   146,   168}, /* Mode2_L 0x2b6 */
  {0x00c7a2aeUL,   146,   168}, /* Mode2_R 0x2b6 */
  {0x00c79740UL,  3502,  4144}, /* Mode2_L 0x300 */
  {0x00c7a2f8UL,  3502,  4144}, /* Mode2_R 0x300 */
  {0x00c7974cUL,  3501,  4143}, /* Mode2_L 0x30c */
  {0x00c7a304UL,  3501,  4143}, /* Mode2_R 0x30c */
  {0x00c79750UL,   120,   136}, /* Mode2_L 0x310 */
  {0x00c7a308UL,   120,   136}, /* Mode2_R 0x310 */
  {0x00c79754UL,   145,   167}, /* Mode2_L 0x314 */
  {0x00c7a30cUL,   145,   167}, /* Mode2_R 0x314 */
  {0x00c79758UL,   146,   168}, /* Mode2_L 0x318 */
  {0x00c7a310UL,   146,   168}, /* Mode2_R 0x318 */
  {0x00c7975cUL,  3501,  4143}, /* Mode2_L 0x31c */
  {0x00c7a314UL,  3501,  4143}, /* Mode2_R 0x31c */
  {0x00c79760UL,   120,   136}, /* Mode2_L 0x320 */
  {0x00c7a318UL,   120,   136}, /* Mode2_R 0x320 */
  {0x00c7976eUL,  3501,  4143}, /* Mode2_L 0x32e */
  {0x00c7a326UL,  3501,  4143}, /* Mode2_R 0x32e */
  {0x00c797c4UL,  3381,  4007}, /* Mode2_L 0x384 */
  {0x00c7a37cUL,  3381,  4007}, /* Mode2_R 0x384 */
  {0x00c797ccUL,    25,    31}, /* Mode2_L 0x38c */
  {0x00c7a384UL,    25,    31}, /* Mode2_R 0x38c */
  {0x00c797d0UL,    26,    32}, /* Mode2_L 0x390 */
  {0x00c7a388UL,    26,    32}, /* Mode2_R 0x390 */
  {0x00c797d4UL,  3381,  4007}, /* Mode2_L 0x394 */
  {0x00c7a38cUL,  3381,  4007}, /* Mode2_R 0x394 */
  {0x00c797dcUL,  3381,  4007}, /* Mode2_L 0x39c */
  {0x00c7a394UL,  3381,  4007}, /* Mode2_R 0x39c */
  {0x00c797e0UL,  3381,  4007}, /* Mode2_L 0x3a0 */
  {0x00c7a398UL,  3381,  4007}, /* Mode2_R 0x3a0 */
  {0x00c7983cUL,  3388,  4014}, /* Mode2_L 0x3fc */
  {0x00c7a3f4UL,  3388,  4014}, /* Mode2_R 0x3fc */
  {0x00c79844UL,  3381,  4007}, /* Mode2_L 0x404 */
  {0x00c7a3fcUL,  3381,  4007}, /* Mode2_R 0x404 */
  {0x00c79848UL,    26,    32}, /* Mode2_L 0x408 */
  {0x00c7a400UL,    26,    32}, /* Mode2_R 0x408 */
  {0x00c7984cUL,  3381,  4007}, /* Mode2_L 0x40c */
  {0x00c7a404UL,  3381,  4007}, /* Mode2_R 0x40c */
  {0x00c79850UL,    26,    32}, /* Mode2_L 0x410 */
  {0x00c7a408UL,    26,    32}, /* Mode2_R 0x410 */
  {0x00c79854UL,  3381,  4007}, /* Mode2_L 0x414 */
  {0x00c7a40cUL,  3381,  4007}, /* Mode2_R 0x414 */
  {0x00c79858UL,    26,    32}, /* Mode2_L 0x418 */
  {0x00c7a410UL,    26,    32}, /* Mode2_R 0x418 */
  {0x00c7985cUL,  3381,  4007}, /* Mode2_L 0x41c */
  {0x00c7a414UL,  3381,  4007}, /* Mode2_R 0x41c */
  {0x00c79860UL,    26,    32}, /* Mode2_L 0x420 */
  {0x00c7a418UL,    26,    32}, /* Mode2_R 0x420 */
  {0x00c79864UL,  3381,  4007}, /* Mode2_L 0x424 */
  {0x00c7a41cUL,  3381,  4007}, /* Mode2_R 0x424 */
  {0x00c7986cUL,  3388,  4014}, /* Mode2_L 0x42c */
  {0x00c7a424UL,  3388,  4014}, /* Mode2_R 0x42c */
  {0x00c79870UL,  3388,  4014}, /* Mode2_L 0x430 */
  {0x00c7a428UL,  3388,  4014}, /* Mode2_R 0x430 */
  {0x00c79874UL,  3383,  4009}, /* Mode2_L 0x434 */
  {0x00c7a42cUL,  3383,  4009}, /* Mode2_R 0x434 */
  {0x00c79878UL,  3383,  4009}, /* Mode2_L 0x438 */
  {0x00c7a430UL,  3383,  4009}, /* Mode2_R 0x438 */
  {0x00c7987cUL,  3383,  4009}, /* Mode2_L 0x43c */
  {0x00c7a434UL,  3383,  4009}, /* Mode2_R 0x43c */
  {0x00c79880UL,  3383,  4009}, /* Mode2_L 0x440 */
  {0x00c7a438UL,  3383,  4009}, /* Mode2_R 0x440 */
  {0x00c79884UL,    26,    32}, /* Mode2_L 0x444 */
  {0x00c7a43cUL,    26,    32}, /* Mode2_R 0x444 */
  {0x00c79888UL,  3381,  4007}, /* Mode2_L 0x448 */
  {0x00c7a440UL,  3381,  4007}, /* Mode2_R 0x448 */
  {0x00c7988cUL,  3388,  4014}, /* Mode2_L 0x44c */
  {0x00c7a444UL,  3388,  4014}, /* Mode2_R 0x44c */
  {0x00c79890UL,  3388,  4014}, /* Mode2_L 0x450 */
  {0x00c7a448UL,  3388,  4014}, /* Mode2_R 0x450 */
  {0x00c798ceUL,  3352,  3972}, /* Mode2_L 0x48e */
  {0x00c7a486UL,  3352,  3972}, /* Mode2_R 0x48e */
  {0x00c798d8UL,    13,    16}, /* Mode2_L 0x498 */
  {0x00c7a490UL,    13,    16}, /* Mode2_R 0x498 */
  {0x00c798dcUL,  1691,  2004}, /* Mode2_L 0x49c */
  {0x00c7a494UL,  1691,  2004}, /* Mode2_R 0x49c */
  {0x00c798e8UL,    13,    16}, /* Mode2_L 0x4a8 */
  {0x00c7a4a0UL,    13,    16}, /* Mode2_R 0x4a8 */
  {0x00c798ecUL,  1691,  2004}, /* Mode2_L 0x4ac */
  {0x00c7a4a4UL,  1691,  2004}, /* Mode2_R 0x4ac */
  {0x00c798f8UL,    13,    16}, /* Mode2_L 0x4b8 */
  {0x00c7a4b0UL,    13,    16}, /* Mode2_R 0x4b8 */
  {0x00c798fcUL,  1691,  2004}, /* Mode2_L 0x4bc */
  {0x00c7a4b4UL,  1691,  2004}, /* Mode2_R 0x4bc */
  {0x00c79900UL,  3356,  3976}, /* Mode2_L 0x4c0 */
  {0x00c7a4b8UL,  3356,  3976}, /* Mode2_R 0x4c0 */
  {0x00c79908UL,  3355,  3975}, /* Mode2_L 0x4c8 */
  {0x00c7a4c0UL,  3355,  3975}, /* Mode2_R 0x4c8 */
  {0x00c79910UL,  3355,  3975}, /* Mode2_L 0x4d0 */
  {0x00c7a4c8UL,  3355,  3975}, /* Mode2_R 0x4d0 */
  {0x00c79918UL,  3356,  3976}, /* Mode2_L 0x4d8 */
  {0x00c7a4d0UL,  3356,  3976}, /* Mode2_R 0x4d8 */
  {0x00c79920UL,  3355,  3975}, /* Mode2_L 0x4e0 */
  {0x00c7a4d8UL,  3355,  3975}, /* Mode2_R 0x4e0 */
  {0x00c79928UL,  3355,  3975}, /* Mode2_L 0x4e8 */
  {0x00c7a4e0UL,  3355,  3975}, /* Mode2_R 0x4e8 */
  {0x00c79938UL,  3356,  3976}, /* Mode2_L 0x4f8 */
  {0x00c7a4f0UL,  3356,  3976}, /* Mode2_R 0x4f8 */
  {0x00c7993cUL,  3356,  3976}, /* Mode2_L 0x4fc */
  {0x00c7a4f4UL,  3356,  3976}, /* Mode2_R 0x4fc */
  {0x00c79990UL,  3371,  3991}, /* Mode2_L 0x550 */
  {0x00c7a548UL,  3371,  3991}, /* Mode2_R 0x550 */
  {0x00c79998UL,  3355,  3975}, /* Mode2_L 0x558 */
  {0x00c7a550UL,  3355,  3975}, /* Mode2_R 0x558 */
  {0x00c799a0UL,  3355,  3975}, /* Mode2_L 0x560 */
  {0x00c7a558UL,  3355,  3975}, /* Mode2_R 0x560 */
  {0x00c799a4UL,  3358,  3978}, /* Mode2_L 0x564 */
  {0x00c7a55cUL,  3358,  3978}, /* Mode2_R 0x564 */
  {0x00c799a8UL,  3357,  3977}, /* Mode2_L 0x568 */
  {0x00c7a560UL,  3357,  3977}, /* Mode2_R 0x568 */
  {0x00c799acUL,  3357,  3977}, /* Mode2_L 0x56c */
  {0x00c7a564UL,  3357,  3977}, /* Mode2_R 0x56c */
  {0x00c799b0UL,  3371,  3991}, /* Mode2_L 0x570 */
  {0x00c7a568UL,  3371,  3991}, /* Mode2_R 0x570 */
  {0x00c799c2UL,  3342,  3962}, /* Mode2_L 0x582 */
  {0x00c7a57aUL,  3342,  3962}, /* Mode2_R 0x582 */
  {0x00c799d0UL,   839,   994}, /* Mode2_L 0x590 */
  {0x00c7a588UL,   839,   994}, /* Mode2_R 0x590 */
  {0x00c799d4UL,   839,   994}, /* Mode2_L 0x594 */
  {0x00c7a58cUL,   839,   994}, /* Mode2_R 0x594 */
  {0x00c79a6cUL,  3356,  3976}, /* Mode2_L 0x62c */
  {0x00c7a624UL,  3356,  3976}, /* Mode2_R 0x62c */
  {0x00c79a74UL,  3355,  3975}, /* Mode2_L 0x634 */
  {0x00c7a62cUL,  3355,  3975}, /* Mode2_R 0x634 */
  {0x00c79a7cUL,  3355,  3975}, /* Mode2_L 0x63c */
  {0x00c7a634UL,  3355,  3975}, /* Mode2_R 0x63c */
  {0x00c79a84UL,  3355,  3975}, /* Mode2_L 0x644 */
  {0x00c7a63cUL,  3355,  3975}, /* Mode2_R 0x644 */
  {0x00c79a8cUL,  3355,  3975}, /* Mode2_L 0x64c */
  {0x00c7a644UL,  3355,  3975}, /* Mode2_R 0x64c */
  {0x00c79a90UL,  3356,  3976}, /* Mode2_L 0x650 */
  {0x00c7a648UL,  3356,  3976}, /* Mode2_R 0x650 */
  {0x00c79a98UL,  3356,  3976}, /* Mode2_L 0x658 */
  {0x00c7a650UL,  3356,  3976}, /* Mode2_R 0x658 */
  {0x00c79aa0UL,  3356,  3976}, /* Mode2_L 0x660 */
  {0x00c7a658UL,  3356,  3976}, /* Mode2_R 0x660 */
  {0x00c79aa8UL,  3355,  3975}, /* Mode2_L 0x668 */
  {0x00c7a660UL,  3355,  3975}, /* Mode2_R 0x668 */
  {0x00c79ad8UL,  1680,  1990}, /* Mode2_L 0x698 */
  {0x00c7a690UL,  1680,  1990}, /* Mode2_R 0x698 */
  {0x00c79af4UL,  1678,  1988}, /* Mode2_L 0x6b4 */
  {0x00c7a6acUL,  1678,  1988}, /* Mode2_R 0x6b4 */
  {0x00c79afcUL,  1678,  1988}, /* Mode2_L 0x6bc */
  {0x00c7a6b4UL,  1678,  1988}, /* Mode2_R 0x6bc */
  {0x00c79b08UL,  3369,  3989}, /* Mode2_L 0x6c8 */
  {0x00c7a6c0UL,  3369,  3989}, /* Mode2_R 0x6c8 */
  {0x00c79b10UL,  3355,  3975}, /* Mode2_L 0x6d0 */
  {0x00c7a6c8UL,  3355,  3975}, /* Mode2_R 0x6d0 */
  {0x00c79b18UL,  3355,  3975}, /* Mode2_L 0x6d8 */
  {0x00c7a6d0UL,  3355,  3975}, /* Mode2_R 0x6d8 */
  {0x00c79b1cUL,  3360,  3980}, /* Mode2_L 0x6dc */
  {0x00c7a6d4UL,  3360,  3980}, /* Mode2_R 0x6dc */
  {0x00c79b20UL,  3357,  3977}, /* Mode2_L 0x6e0 */
  {0x00c7a6d8UL,  3357,  3977}, /* Mode2_R 0x6e0 */
  {0x00c79b24UL,  3356,  3976}, /* Mode2_L 0x6e4 */
  {0x00c7a6dcUL,  3356,  3976}, /* Mode2_R 0x6e4 */
  {0x00c79b4aUL,  3356,  3976}, /* Mode2_L 0x70a */
  {0x00c7a702UL,  3356,  3976}, /* Mode2_R 0x70a */
  {0x00c79b5aUL,  3356,  3976}, /* Mode2_L 0x71a */
  {0x00c7a712UL,  3356,  3976}, /* Mode2_R 0x71a */
  {0x00c79b64UL,  3362,  3982}, /* Mode2_L 0x724 */
  {0x00c7a71cUL,  3362,  3982}, /* Mode2_R 0x724 */
  {0x00c79b6cUL,  3355,  3975}, /* Mode2_L 0x72c */
  {0x00c7a724UL,  3355,  3975}, /* Mode2_R 0x72c */
  {0x00c79b74UL,  3355,  3975}, /* Mode2_L 0x734 */
  {0x00c7a72cUL,  3355,  3975}, /* Mode2_R 0x734 */
  {0x00c79b78UL,  3356,  3976}, /* Mode2_L 0x738 */
  {0x00c7a730UL,  3356,  3976}, /* Mode2_R 0x738 */
  {0x00c79b7cUL,  3358,  3978}, /* Mode2_L 0x73c */
  {0x00c7a734UL,  3358,  3978}, /* Mode2_R 0x73c */
  {0x00c79b92UL,  3356,  3976}, /* Mode2_L 0x752 */
  {0x00c7a74aUL,  3356,  3976}, /* Mode2_R 0x752 */
  {0x00c79b9eUL,  3356,  3976}, /* Mode2_L 0x75e */
  {0x00c7a756UL,  3356,  3976}, /* Mode2_R 0x75e */
  {0x00c79bbcUL,  3356,  3976}, /* Mode2_L 0x77c */
  {0x00c7a774UL,  3356,  3976}, /* Mode2_R 0x77c */
  {0x00c79bc4UL,  5625,  6667}, /* Mode2_L 0x784 */
  {0x00c7a77cUL,  5625,  6667}, /* Mode2_R 0x784 */
  {0x00c79d9eUL,   406,     0}, /* Mode2_L 0x95e */
  {0x00c7a956UL,   406,     0}, /* Mode2_R 0x95e */
  {0x00c79e1cUL,  3366,  3986}, /* Mode2_L 0x9dc */
  {0x00c7a9d4UL,  3366,  3986}, /* Mode2_R 0x9dc */
  {0x00c79e24UL,  3355,  3975}, /* Mode2_L 0x9e4 */
  {0x00c7a9dcUL,  3355,  3975}, /* Mode2_R 0x9e4 */
  {0x00c79e2cUL,  3355,  3975}, /* Mode2_L 0x9ec */
  {0x00c7a9e4UL,  3355,  3975}, /* Mode2_R 0x9ec */
  {0x00c79e34UL,  3355,  3975}, /* Mode2_L 0x9f4 */
  {0x00c7a9ecUL,  3355,  3975}, /* Mode2_R 0x9f4 */
  {0x00c79e3cUL,  3355,  3975}, /* Mode2_L 0x9fc */
  {0x00c7a9f4UL,  3355,  3975}, /* Mode2_R 0x9fc */
  {0x00c79e40UL,  3366,  3986}, /* Mode2_L 0xa00 */
  {0x00c7a9f8UL,  3366,  3986}, /* Mode2_R 0xa00 */
  {0x00c79e48UL,  3366,  3986}, /* Mode2_L 0xa08 */
  {0x00c7aa00UL,  3366,  3986}, /* Mode2_R 0xa08 */
  {0x00c79e50UL,  3356,  3976}, /* Mode2_L 0xa10 */
  {0x00c7aa08UL,  3356,  3976}, /* Mode2_R 0xa10 */
  {0x00c79e58UL,  3365,  3985}, /* Mode2_L 0xa18 */
  {0x00c7aa10UL,  3365,  3985}, /* Mode2_R 0xa18 */
  {0x00c79e88UL,  1680,  1990}, /* Mode2_L 0xa48 */
  {0x00c7aa40UL,  1680,  1990}, /* Mode2_R 0xa48 */
  {0x00c79ea4UL,  1678,  1988}, /* Mode2_L 0xa64 */
  {0x00c7aa5cUL,  1678,  1988}, /* Mode2_R 0xa64 */
  {0x00c79eacUL,  1678,  1988}, /* Mode2_L 0xa6c */
  {0x00c7aa64UL,  1678,  1988}, /* Mode2_R 0xa6c */
  {0x00c79ebcUL,  3366,  3986}, /* Mode2_L 0xa7c */
  {0x00c7aa74UL,  3366,  3986}, /* Mode2_R 0xa7c */
  {0x00c79ec4UL,  3355,  3975}, /* Mode2_L 0xa84 */
  {0x00c7aa7cUL,  3355,  3975}, /* Mode2_R 0xa84 */
  {0x00c79eccUL,  3355,  3975}, /* Mode2_L 0xa8c */
  {0x00c7aa84UL,  3355,  3975}, /* Mode2_R 0xa8c */
  {0x00c79ed4UL,  3355,  3975}, /* Mode2_L 0xa94 */
  {0x00c7aa8cUL,  3355,  3975}, /* Mode2_R 0xa94 */
  {0x00c79edcUL,  3355,  3975}, /* Mode2_L 0xa9c */
  {0x00c7aa94UL,  3355,  3975}, /* Mode2_R 0xa9c */
  {0x00c79ee0UL,  3366,  3986}, /* Mode2_L 0xaa0 */
  {0x00c7aa98UL,  3366,  3986}, /* Mode2_R 0xaa0 */
  {0x00c79ee8UL,  3366,  3986}, /* Mode2_L 0xaa8 */
  {0x00c7aaa0UL,  3366,  3986}, /* Mode2_R 0xaa8 */
  {0x00c79ef0UL,  3356,  3976}, /* Mode2_L 0xab0 */
  {0x00c7aaa8UL,  3356,  3976}, /* Mode2_R 0xab0 */
  {0x00c79ef8UL,  3365,  3985}, /* Mode2_L 0xab8 */
  {0x00c7aab0UL,  3365,  3985}, /* Mode2_R 0xab8 */
  {0x00c79f28UL,  1680,  1990}, /* Mode2_L 0xae8 */
  {0x00c7aae0UL,  1680,  1990}, /* Mode2_R 0xae8 */
  {0x00c79f44UL,  1678,  1988}, /* Mode2_L 0xb04 */
  {0x00c7aafcUL,  1678,  1988}, /* Mode2_R 0xb04 */
  {0x00c79f4cUL,  1678,  1988}, /* Mode2_L 0xb0c */
  {0x00c7ab04UL,  1678,  1988}, /* Mode2_R 0xb0c */
  {0x00c79f5cUL,  3384,  4004}, /* Mode2_L 0xb1c */
  {0x00c7ab14UL,  3384,  4004}, /* Mode2_R 0xb1c */
  {0x00c79f64UL,  3355,  3975}, /* Mode2_L 0xb24 */
  {0x00c7ab1cUL,  3355,  3975}, /* Mode2_R 0xb24 */
  {0x00c79f6cUL,  3355,  3975}, /* Mode2_L 0xb2c */
  {0x00c7ab24UL,  3355,  3975}, /* Mode2_R 0xb2c */
  {0x00c79f74UL,  3355,  3975}, /* Mode2_L 0xb34 */
  {0x00c7ab2cUL,  3355,  3975}, /* Mode2_R 0xb34 */
  {0x00c79f7cUL,  3355,  3975}, /* Mode2_L 0xb3c */
  {0x00c7ab34UL,  3355,  3975}, /* Mode2_R 0xb3c */
  {0x00c79f80UL,  3384,  4004}, /* Mode2_L 0xb40 */
  {0x00c7ab38UL,  3384,  4004}, /* Mode2_R 0xb40 */
  {0x00c79f84UL,  3373,  3993}, /* Mode2_L 0xb44 */
  {0x00c7ab3cUL,  3373,  3993}, /* Mode2_R 0xb44 */
  {0x00c79f88UL,  3384,  4004}, /* Mode2_L 0xb48 */
  {0x00c7ab40UL,  3384,  4004}, /* Mode2_R 0xb48 */
  {0x00c79f90UL,  3356,  3976}, /* Mode2_L 0xb50 */
  {0x00c7ab48UL,  3356,  3976}, /* Mode2_R 0xb50 */
  {0x00c79f98UL,  3383,  4003}, /* Mode2_L 0xb58 */
  {0x00c7ab50UL,  3383,  4003}, /* Mode2_R 0xb58 */
  {0x00c79fc8UL,  1680,  1990}, /* Mode2_L 0xb88 */
  {0x00c7ab80UL,  1680,  1990}, /* Mode2_R 0xb88 */
  {0x00c79fe4UL,  1678,  1988}, /* Mode2_L 0xba4 */
  {0x00c7ab9cUL,  1678,  1988}, /* Mode2_R 0xba4 */
  {0x00c79fecUL,  1678,  1988}, /* Mode2_L 0xbac */
  {0x00c7aba4UL,  1678,  1988}, /* Mode2_R 0xbac */
  {0x00e027f0UL,  3348,  3968}, /* S1_mode2_preview */
  {0x00e0283cUL,  3348,  3968}, /* S1_mode4_4k */
  {0x00e02862UL,  3348,  3968}, /* S1_mode5_4k */
  {0x00e02888UL,  3348,  3968}, /* S1_mode6_4k24p */
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
  {0x00e02c8aUL,    68,    82}, /* S10_mode2_pdaf (Node8*2) */
  {0x00e02cd6UL,    68,    82}, /* S10_mode4_pdaf (Node8*2) */
  {0x00e02cfcUL,    68,    82}, /* S10_mode5_pdaf (Node8*2) */
  {0x00e02d22UL,    68,    82}, /* S10_mode6_pdaf (Node8*2) */
  {0x00e030e2UL,   406,     0}, /* LC_S7_mode2_voff */
  {0x00e030e6UL,   457,    51}, /* LC_S8_mode2_f6 */
  {0x00e030feUL,   406,     0}, /* LC_S7_mode4_voff */
  {0x00e03102UL,   457,    51}, /* LC_S8_mode4_f6 */
  {0x00e0310cUL,   406,     0}, /* LC_S7_mode5_voff */
  {0x00e03110UL,   457,    51}, /* LC_S8_mode5_f6 */
  {0x00e0311aUL,   406,     0}, /* LC_S7_mode6_voff */
  {0x00e0311eUL,   457,    51}, /* LC_S8_mode6_f6 */
  {0x020d96caUL,  3348,  3968}, /* DrvParam2_height (+0x66) */
  {0x020d96f0UL,    68,    82}, /* DrvParam2_S10 (+0x8c) */
  {0x020d975aUL,   406,     0}, /* DrvParam2_cropV1 (+0xf6) */
  {0x020d9766UL,   406,     0}, /* DrvParam2_cropV2 (+0x102) */
  {0x020d97a6UL,  1704,  2020}, /* DrvParam2_S3 (+0x142) */
  {0x020d97beUL,   457,    51}, /* DrvParam2_S8 (+0x15a) */
  {0x020d998aUL,  3348,  3968}, /* DrvParam4_height (+0x66) */
  {0x020d99b0UL,    68,    82}, /* DrvParam4_S10 (+0x8c) */
  {0x020d9a1aUL,   406,     0}, /* DrvParam4_cropV1 (+0xf6) */
  {0x020d9a26UL,   406,     0}, /* DrvParam4_cropV2 (+0x102) */
  {0x020d9a66UL,  1704,  2020}, /* DrvParam4_S3 (+0x142) */
  {0x020d9a7eUL,   457,    51}, /* DrvParam4_S8 (+0x15a) */
  {0x020d9aeaUL,  3348,  3968}, /* DrvParam5_height (+0x66) */
  {0x020d9b10UL,    68,    82}, /* DrvParam5_S10 (+0x8c) */
  {0x020d9b7aUL,   406,     0}, /* DrvParam5_cropV1 (+0xf6) */
  {0x020d9b86UL,   406,     0}, /* DrvParam5_cropV2 (+0x102) */
  {0x020d9bc6UL,  1704,  2020}, /* DrvParam5_S3 (+0x142) */
  {0x020d9bdeUL,   457,    51}, /* DrvParam5_S8 (+0x15a) */
  {0x020d9c4aUL,  3348,  3968}, /* DrvParam6_height (+0x66) */
  {0x020d9c70UL,    68,    82}, /* DrvParam6_S10 (+0x8c) */
  {0x020d9cdaUL,   406,     0}, /* DrvParam6_cropV1 (+0xf6) */
  {0x020d9ce6UL,   406,     0}, /* DrvParam6_cropV2 (+0x102) */
  {0x020d9d26UL,  1704,  2020}, /* DrvParam6_S3 (+0x142) */
  {0x020d9d3eUL,   457,    51}, /* DrvParam6_S8 (+0x15a) */
};
#define NP16 (sizeof(P16)/sizeof(P16[0]))

struct p8_entry { u32 addr; u8 oldv; u8 newv; };
static const struct p8_entry P8[] = {
  {0x01038d5dUL, 0xe7, 0xb1}, /* TG_Node3_Col1_VBLANK_lo */
  {0x01038d71UL, 0x01, 0x00}, /* TG_Node4_Col1_VBLANK_hi */
  {0x01038d85UL, 0x90, 0xc6}, /* TG_Node5_Col1_Active_lo */
  {0x01038d99UL, 0x06, 0x07}, /* TG_Node6_Col1_Active_hi */
  {0x01038dadUL, 0x09, 0x02}, /* TG_Node7_Col1_PDAF_start (43 - 41) */
  {0x01038dc1UL, 0x22, 0x29}, /* TG_Node8_Col1_PDAF_len (41 -> S10=82) */
  {0x01038de9UL, 0x29, 0x2f}, /* TG_Node10_Col1_reg19 (47 == S9) */
  {0x020d966aUL, 0x01, 0x1b}, /* DrvParam2_vratio_num (+0x06: 27) */
  {0x020d966bUL, 0x01, 0x20}, /* DrvParam2_vratio_den (+0x07: 32) */
  {0x020d992aUL, 0x01, 0x1b}, /* DrvParam4_vratio_num (+0x06: 27) */
  {0x020d992bUL, 0x01, 0x20}, /* DrvParam4_vratio_den (+0x07: 32) */
  {0x020d9a8aUL, 0x01, 0x1b}, /* DrvParam5_vratio_num (+0x06: 27) */
  {0x020d9a8bUL, 0x01, 0x20}, /* DrvParam5_vratio_den (+0x07: 32) */
  {0x020d9beaUL, 0x01, 0x1b}, /* DrvParam6_vratio_num (+0x06: 27) */
  {0x020d9bebUL, 0x01, 0x20}, /* DrvParam6_vratio_den (+0x07: 32) */
};
#define NP8 (sizeof(P8)/sizeof(P8[0]))

static void unlink_markers(void){
  const char *m[]={"/tmp_wbshd/m2_start","/tmp_wbshd/m2_ok","/tmp_wbshd/m2_mismatch","/tmp_wbshd/m2_bad","/tmp_wbshd/m2_done"};
  for(unsigned i=0;i<sizeof(m)/sizeof(m[0]);i++) sc6(35,-100,(long)m[i],0,0,0,0);
}

__attribute__((constructor)) static void go(void){
  unlink_markers();
  wr("/tmp_wbshd/m2_start","S");
  u32 handle=0; u64 phys=cam_dmm_alloc(CP_PAGE,&handle);
  if(!phys){ wr("/tmp_wbshd/m2_bad","ALLOC"); return; }
  if((phys&0x3f)||phys<0x400000000UL||phys+CP_PAGE>0x500000000UL){ wr("/tmp_wbshd/m2_bad","PHYS"); cam_dmm_free(handle); return; }
  i64 fd=sc6(56,-100,(long)"/dev/mem",0x101002,0,0,0);
  if(fd<0){ wr("/tmp_wbshd/m2_bad","DEVMEM"); cam_dmm_free(handle); return; }
  u64 lead=phys&0xfff, maplen=(lead+CP_PAGE+0xfffUL)&~0xfffUL;
  i64 m=sc6(222,0,(long)maplen,3,1,fd,(long)(phys-lead));
  if(m<0){ wr("/tmp_wbshd/m2_bad","MMAP"); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle); return; }
  u8 *b=(u8*)m+lead;

  u64 pages[32]; u32 np=0;
  for(u32 i=0;i<NP16;i++){ u64 pg=P16[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<32) pages[np++]=pg; }
  for(u32 i=0;i<NP8;i++){ u64 pg=P8[i].addr & ~0xfffUL; int f=0;
    for(u32 k=0;k<np;k++) if(pages[k]==pg){f=1;break;}
    if(!f && np<32) pages[np++]=pg; }

  int stock_ok = 1;
  for(u32 p=0; p<np; p++){
    u64 cur_pg = pages[p];
    if(sen_dma_transfer(0, cur_pg, phys, CP_PAGE) != 0){ stock_ok = 0; break; }
    for(u32 i=0; i<NP16; i++){
      if((P16[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P16[i].addr & 0xfffUL;
        u16 cv = (u16)(b[off] | (b[off+1] << 8));
        if(cv != P16[i].oldv && cv != P16[i].newv){ stock_ok = 0; break; }
      }
    }
    for(u32 i=0; i<NP8; i++){
      if((P8[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P8[i].addr & 0xfffUL;
        if(b[off] != P8[i].oldv && b[off] != P8[i].newv){ stock_ok = 0; break; }
      }
    }
    if(!stock_ok) break;
  }
  if(!stock_ok){
    wr("/tmp_wbshd/m2_mismatch","MISMATCH");
    sc6(215,m,(long)maplen,0,0,0,0); sc6(57,fd,0,0,0,0,0); cam_dmm_free(handle);
    wr("/tmp_wbshd/m2_done","D"); return;
  }

  int all_ok = 1;
  for(u32 p=0; p<np; p++){
    u64 cur_pg = pages[p];
    if(sen_dma_transfer(0, cur_pg, phys, CP_PAGE) != 0){ all_ok = 0; break; }
    for(u32 i=0; i<NP16; i++){
      if((P16[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P16[i].addr & 0xfffUL;
        u16 nv = P16[i].newv;
        b[off] = (u8)nv; b[off+1] = (u8)(nv >> 8);
      }
    }
    for(u32 i=0; i<NP8; i++){
      if((P8[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P8[i].addr & 0xfffUL;
        b[off] = P8[i].newv;
      }
    }
    if(sen_dma_transfer(1, phys, cur_pg, CP_PAGE) != 0){ all_ok = 0; break; }
    if(sen_dma_transfer(0, cur_pg, phys, CP_PAGE) != 0){ all_ok = 0; break; }
    for(u32 i=0; i<NP16; i++){
      if((P16[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P16[i].addr & 0xfffUL;
        u16 cv = (u16)(b[off] | (b[off+1] << 8));
        if(cv != P16[i].newv){ all_ok = 0; break; }
      }
    }
    for(u32 i=0; i<NP8; i++){
      if((P8[i].addr & ~0xfffUL) == cur_pg){
        u32 off = P8[i].addr & 0xfffUL;
        if(b[off] != P8[i].newv){ all_ok = 0; break; }
      }
    }
    if(!all_ok) break;
  }

  wr(all_ok ? "/tmp_wbshd/m2_ok" : "/tmp_wbshd/m2_bad", all_ok ? "OK" : "BAD");
  sc6(215,m,(long)maplen,0,0,0,0);
  sc6(57,fd,0,0,0,0,0);
  cam_dmm_free(handle);
  wr("/tmp_wbshd/m2_done","D");
}
