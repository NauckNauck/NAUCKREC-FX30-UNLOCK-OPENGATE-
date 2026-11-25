#!/usr/bin/env python3
"""Generate and compile fx30_ap_all_v4.so — Unified Zero-Dependency AArch64 AP Patcher
for Sony ILME-FX30 (FW 6.01).

Applies ALL AP-side patches in a SINGLE USB session (replacing 3 separate CT6 runs):
  1. appFw.so (rw-p): 1 guard + 10 HAITA rules (0x047c1eb0..0x047c1f40) -> V60 Card Unlock
  2. libmpr.so (r-xp via /proc/self/mem): 2 guards + 13 patches:
       - V60 RateCheck::GetMediaTransferRateInh (0x103c88)
       - XAVC S 4K AVC Long-GOP tables (0x2cb13e, 0x2cb16e, 0x2e0bb6, 0x2e0bee)
       - XAVC S-I 4K All-Intra Row 9 tables (0x2cb216, 0x2e0cb2)
       - XavcsMovieWriter / TransMPFResolution2PubDefinition / WriteMetaInfo (0x1164B0, 0x180098, 0x109768)
       - libXavcMp4 native 3840x2560 container boxes & avcC level_idc (0x21ff68, 0x21fb34, 0x2200a0, 0x2263bc)
  3. av-cam.bin (physical /dev/mem via /sys/module/liro/parameters/adr):
       - 2 guards + 110 verified AVC sites + 16 XAVC S-I All-Intra sites (105200/01/04/07) = 126 patches

Every single guard and old byte sequence is asserted byte-for-byte against the unpacked FX30 6.01 firmware!
"""
from __future__ import annotations

import hashlib
import json
import os
import struct
import subprocess
from pathlib import Path

RESEARCH = Path(os.environ.get("FX30_RESEARCH_DIR", "/Users/nauck/Documents/sony_research"))
REPO = Path(__file__).resolve().parents[1]
SRC_OUT = REPO / "fx30_open_gate" / "sources" / "fx30_ap_all_v4.c"
SO_OUT = RESEARCH / "fx30_open_gate" / "fx30_ap_all_v4.so"

A7IV_JSON = RESEARCH / "a7iv-open-gate-test/windows/tools/modding/designs/ogfile2560_hs_s_v1.json"
FX30_APPFW = RESEARCH / "fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha15_unpacked/lib/appFw.so"
FX30_AVCAM = RESEARCH / "fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha3_unpacked/av-cam.bin"
FX30_LIBMPR = RESEARCH / "fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha15_unpacked/lib/libmpr.so"

ELF_HEADER_GUARD = bytes.fromhex("7f454c46020101030000000000000000")
FX30_AVCAM_GUARD_0 = bytes.fromhex("d8882701c0ffffffd8882701c0ffffff")  # 0x381b40
FX30_AVCAM_GUARD_1 = bytes.fromhex("c0fffffff89f2801c0ffffff20a02801")  # 0x3838bc


def c_bytes(b: bytes) -> str:
    return "{" + ",".join(f"0x{x:02x}" for x in b.ljust(16, b"\x00")) + "}"


def fmt_table(name: str, rows: list[tuple[int, int, int, bytes, bytes, str]]) -> str:
    out = [f"static const struct patch_rec {name}[] = {{"]
    for off, ln, flg, oldb, newb, note in rows:
        out.append(f"  {{0x{off:08x}U, {ln:2d}, {flg}, {c_bytes(oldb)}, {c_bytes(newb)}}}, /* {note} */")
    out.append("};")
    out.append(f"#define N_{name} (sizeof({name})/sizeof({name}[0]))")
    return "\n".join(out)


def build_tables():
    appfw = FX30_APPFW.read_bytes()
    av = FX30_AVCAM.read_bytes()
    mpr = FX30_LIBMPR.read_bytes()
    a7og = json.loads(A7IV_JSON.read_text())

    # 1. appFw.so: 1 guard + 10 HAITA V60 rules
    appfw_spec = [
        (0x047C1EA0, 16, 0, bytes.fromhex("70020000ae00150005000000ad001500"), b"", "HAITA guard 0x0270"),
        (0x047C1EB0, 16, 1, bytes.fromhex("a602000000001a000500000001001a00"), bytes.fromhex("a602000000001a000000000001001a00"), "HAITA 0x02a6"),
        (0x047C1EC0, 16, 1, bytes.fromhex("a702000002001a000500000003001a00"), bytes.fromhex("a702000002001a000000000003001a00"), "HAITA 0x02a7"),
        (0x047C1ED0, 16, 1, bytes.fromhex("a802000004001a000500000005001a00"), bytes.fromhex("a802000004001a000000000005001a00"), "HAITA 0x02a8"),
        (0x047C1EE0, 16, 1, bytes.fromhex("a902000006001a000500000007001a00"), bytes.fromhex("a902000006001a000000000007001a00"), "HAITA 0x02a9"),
        (0x047C1EF0, 16, 1, bytes.fromhex("aa02000008001a000500000009001a00"), bytes.fromhex("aa02000008001a000000000009001a00"), "HAITA 0x02aa"),
        (0x047C1F00, 16, 1, bytes.fromhex("ab0200000a001a00050000000b001a00"), bytes.fromhex("ab0200000a001a00000000000b001a00"), "HAITA 0x02ab"),
        (0x047C1F10, 16, 1, bytes.fromhex("ac0200000c001a00050000000d001a00"), bytes.fromhex("ac0200000c001a00000000000d001a00"), "HAITA 0x02ac"),
        (0x047C1F20, 16, 1, bytes.fromhex("ad0200000e001a00050000000f001a00"), bytes.fromhex("ad0200000e001a00000000000f001a00"), "HAITA 0x02ad"),
        (0x047C1F30, 16, 1, bytes.fromhex("ae02000010001a000500000011001a00"), bytes.fromhex("ae02000010001a000000000011001a00"), "HAITA 0x02ae"),
        (0x047C1F40, 16, 1, bytes.fromhex("af02000012001a000500000013001a00"), bytes.fromhex("af02000012001a000000000013001a00"), "HAITA 0x02af"),
    ]
    for off, ln, flg, oldb, newb, note in appfw_spec:
        assert appfw[off : off + ln] == oldb, f"appFw mismatch at {hex(off)} ({note})"

    # 2. libmpr.so: 2 guards + 13 patches (V60 RateCheck + AVC + All-I + native MP4 avc1/tkhd/avcC)
    libmpr_spec = [
        (0x00000000, 16, 0, ELF_HEADER_GUARD, b"", "ELF guard"),
        (0x002CB138, 6, 0, bytes.fromhex("00000000000f"), b"", "g_videoParamComb Row0 guard"),
        (0x00103C88, 16, 1, bytes.fromhex("ff8309d1001c005304040151fd7bbca9"), bytes.fromhex("5f0000397f0000b9e0031f2ac0035fd6"), "RateCheck V60 unlock"),
        (0x002CB13E, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "g_videoParamComb Row0 AVC 2160->2560"),
        (0x002CB16E, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "g_videoParamComb Row2 AVC 2160->2560"),
        (0x002CB216, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "g_videoParamComb Row9 All-I 2160->2560"),
        (0x002E0BB6, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "m_vpcSt Row0 AVC 2160->2560"),
        (0x002E0BEE, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "m_vpcSt Row2 AVC 2160->2560"),
        (0x002E0CB2, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), "m_vpcSt Row9 All-I 2160->2560"),
        (0x001164B0, 4, 1, bytes.fromhex("000e8152"), bytes.fromhex("00408152"), "XavcsMovieWriter NRT height 2160->2560"),
        (0x00180098, 4, 1, bytes.fromhex("000e8152"), bytes.fromhex("00408152"), "TransMPFResolution2PubDefinition 2160->2560"),
        (0x00109768, 4, 1, bytes.fromhex("0b0e8152"), bytes.fromhex("0b408152"), "xavcsMovieMeta::WriteMetaInfo 2160->2560"),
        (0x0021FF68, 4, 1, bytes.fromhex("060e8152"), bytes.fromhex("06408152"), "libXavcMp4 avc1 track height mov w6,#2560"),
        (0x0021FB34, 4, 1, bytes.fromhex("1fc02171"), bytes.fromhex("1f002871"), "libXavcMp4 avcC level_idc cmp w0,#2560"),
        (0x002200A0, 4, 1, bytes.fromhex("060e8152"), bytes.fromhex("06408152"), "libXavcMp4 hvc1 track height mov w6,#2560"),
        (0x002263BC, 4, 1, bytes.fromhex("010e8152"), bytes.fromhex("01408152"), "libXavcMp4::Rotate avc1 height mov w1,#2560"),
    ]
    for off, ln, flg, oldb, newb, note in libmpr_spec:
        assert mpr[off : off + ln] == oldb, f"libmpr mismatch at {hex(off)} ({note})"

    # 3. av-cam.bin: 2 guards + 110 verified AVC sites + 16 XAVC S-I All-Intra sites
    avcam_spec = [
        (0x00381B40, 16, 0, FX30_AVCAM_GUARD_0, b"", "av-cam guard 0"),
        (0x003838BC, 16, 0, FX30_AVCAM_GUARD_1, b"", "av-cam guard 1"),
        (0x00390630, 12, 1, struct.pack("<3I", 8192, 4320, 3), struct.pack("<3I", 3840, 2560, 23), "A3-clip 3840x2560 fmt 23"),
    ]
    delta = 0xEE1C
    seen_av = {0x381B40, 0x3838BC, 0x390630}
    av_p = [p for p in a7og["patches"] if p["binary"] == "av-cam.bin" and p["kind"] != "A3-clip"]
    assert len(av_p) == 100
    for p in av_p:
        fx_off = int(p["file_off"], 16) + delta
        oldb = bytes.fromhex(p["old"])
        newb = bytes.fromhex(p["new"])
        assert fx_off not in seen_av, f"Duplicate av-cam offset {hex(fx_off)}"
        seen_av.add(fx_off)
        avcam_spec.append((fx_off, len(oldb), 1, oldb, newb, p.get("note", "a7iv-port")))

    extra_avc_offs = [
        (0x3AD378, "AVC 102600 +0x90"),
        (0x3AD678, "AVC 102601 +0x90"),
        (0x3AF878, "AVC 102603 +0x90"),
        (0x3AF978, "AVC 102604 +0x90"),
        (0x3B3778, "AVC 102607 +0x90"),
        (0x395328, "AVC 102602 desc #77"),
        (0x3AF6EE, "AVC 102602 prof +0x06"),
        (0x3AF720, "AVC 102602 prof +0x38"),
        (0x3AF778, "AVC 102602 prof +0x90"),
        # XAVC S-I 4K All-Intra (105200, 105201, 105204, 105207)
        (0x396498, "All-I 105200 desc"),
        (0x3B15EE, "All-I 105200 prof -0x1a"),
        (0x3B1620, "All-I 105200 prof +0x18"),
        (0x3B1678, "All-I 105200 prof +0x70"),
        (0x396528, "All-I 105201 desc"),
        (0x3B16EE, "All-I 105201 prof -0x1a"),
        (0x3B1720, "All-I 105201 prof +0x18"),
        (0x3B1778, "All-I 105201 prof +0x70"),
        (0x3966D8, "All-I 105204 desc"),
        (0x3B19EE, "All-I 105204 prof -0x1a"),
        (0x3B1A20, "All-I 105204 prof +0x18"),
        (0x3B1A78, "All-I 105204 prof +0x70"),
        (0x396888, "All-I 105207 desc"),
        (0x3B1CEE, "All-I 105207 prof -0x1a"),
        (0x3B1D20, "All-I 105207 prof +0x18"),
        (0x3B1D78, "All-I 105207 prof +0x70"),
    ]
    for fx_off, note in extra_avc_offs:
        assert fx_off not in seen_av, f"Duplicate av-cam offset {hex(fx_off)}"
        seen_av.add(fx_off)
        avcam_spec.append((fx_off, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), note))

    # Complete coverage of ALL 86 XAVC S 4K AVC (fmt=4) & XAVC S-I 4K All-Intra (fmt=24)
    # 3840x2160 rows across 23.98p, 25p, 29.97p, 50p, 59.94p, and S&Q (inp=120),
    # matched deterministically against the 0x100-byte secondary config table (0x3AAAE8..0x3C1FE8).
    cfg_map: dict[tuple[int, int], int] = {}
    for cfg_base in range(0x3AAAE8, 0x3C1FE8, 0x100):
        c_fmt = struct.unpack_from("<I", av, cfg_base)[0]
        c_id = struct.unpack_from("<I", av, cfg_base + 0x20)[0]
        cfg_map[(c_fmt, c_id)] = cfg_base

    for row_idx in range(689):
        r = 0x392758 + row_idx * 0x90
        fmt = struct.unpack_from("<I", av, r)[0]
        w, h = struct.unpack_from("<HH", av, r + 0x7E)
        if fmt not in (4, 24) or w != 3840 or h != 2160:
            continue
        cfg_id = struct.unpack_from("<I", av, r + 0x28)[0]
        assert (fmt, cfg_id) in cfg_map, f"Missing secondary config for fmt={fmt} id={cfg_id}"
        cfg_base = cfg_map[(fmt, cfg_id)]
        tag = "All-I" if fmt == 24 else "AVC"
        for off, label in (
            (r + 0x80, f"{tag} {cfg_id} desc"),
            (cfg_base + 0x06, f"{tag} {cfg_id} prof +0x06"),
            (cfg_base + 0x38, f"{tag} {cfg_id} prof +0x38"),
            (cfg_base + 0x90, f"{tag} {cfg_id} prof +0x90"),
        ):
            if off not in seen_av:
                seen_av.add(off)
                avcam_spec.append((off, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a"), label))

    for off, ln, flg, oldb, newb, note in avcam_spec:
        assert 0x380000 <= off and off + ln <= 0x3D0000, f"av-cam out of window {hex(off)}"
        assert av[off : off + ln] == oldb, f"av-cam mismatch at {hex(off)} ({note})"

    print(
        f"[fx30_ap_all_v4] VERIFIED 100%: "
        f"appFw={len(appfw_spec)-1}p+1g | libmpr={len(libmpr_spec)-2}p+2g | av-cam={len(avcam_spec)-2}p+2g (0 mismatches!)"
    )
    return appfw_spec, libmpr_spec, avcam_spec


def generate_c(appfw_spec, libmpr_spec, avcam_spec) -> str:
    return f"""/* SONY FX30 (FW 6.01) — UNIFIED AP HELPER v4 (V60 Unlock + Open Gate 3840x2560 AVC/All-I + Native MP4 Headers)
 * Zero-dependency AArch64 shared library (syscalls only). Volatile RAM only.
 *   1. appFw.so (rw-p): 1 guard + 10 HAITA rules (V60 menu unlock)
 *   2. libmpr.so (r-xp via /proc/self/mem): 2 guards + 14 patches (RateCheck + AVC/All-I + libXavcMp4 2560)
 *   3. av-cam.bin (/dev/mem via /sys/module/liro/parameters/adr): 2 guards + 126 patches (AVC + All-I)
 * Markers: /tmp_wbshd/ap_{{start,ok,mismatch,bad,done}} (falls back to /tmp/ap_* in QEMU).
 */
typedef unsigned long u64;
typedef long i64;
typedef unsigned int u32;
typedef unsigned char u8;

struct patch_rec {{
  u32 off;
  u8 len;
  u8 is_patch; /* 0 = guard only, 1 = patch */
  u8 oldb[16];
  u8 newb[16];
}};

{fmt_table("APPFW_REC", appfw_spec)}

{fmt_table("LIBMPR_REC", libmpr_spec)}

{fmt_table("AVCAM_REC", avcam_spec)}

static long sc6(long n, long a, long b, long c, long d, long e, long f) {{
  register long x8 asm("x8") = n;
  register long x0 asm("x0") = a;
  register long x1 asm("x1") = b;
  register long x2 asm("x2") = c;
  register long x3 asm("x3") = d;
  register long x4 asm("x4") = e;
  register long x5 asm("x5") = f;
  asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
  return x0;
}}

static int use_tmp_fallback;

static void make_mk_path(char *dst, const char *suffix) {{
  const char *base = use_tmp_fallback ? "/tmp/ap_" : "/tmp_wbshd/ap_";
  int i = 0;
  while (base[i]) {{ dst[i] = base[i]; i++; }}
  int j = 0;
  while (suffix[j]) {{ dst[i++] = suffix[j++]; }}
  dst[i] = 0;
}}

static void wr_mk(const char *suffix, const char *msg) {{
  char p[64];
  make_mk_path(p, suffix);
  long n = 0;
  while (msg[n]) n++;
  long fd = sc6(56, -100, (long)p, 0x241, 0644, 0, 0);
  if (fd >= 0) {{
    sc6(64, fd, (long)msg, n, 0, 0, 0);
    sc6(57, fd, 0, 0, 0, 0, 0);
  }}
}}

static void unlink_all_mk(void) {{
  const char *s[] = {{"start", "ok", "mismatch", "bad", "done", "no_liro"}};
  char p[64];
  for (u32 i = 0; i < 6; i++) {{
    make_mk_path(p, s[i]);
    sc6(35, -100, (long)p, 0, 0, 0, 0);
  }}
}}

static int mem_eq(const u8 *a, const u8 *b, u32 n) {{
  for (u32 i = 0; i < n; i++) if (a[i] != b[i]) return 0;
  return 1;
}}

static u64 parse_hex(const char *s, const char **endp) {{
  u64 v = 0;
  while (1) {{
    char c = *s;
    if (c >= '0' && c <= '9') v = (v << 4) | (u64)(c - '0');
    else if (c >= 'a' && c <= 'f') v = (v << 4) | (u64)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') v = (v << 4) | (u64)(c - 'A' + 10);
    else break;
    s++;
  }}
  if (endp) *endp = s;
  return v;
}}

static u64 parse_dec(const char *s) {{
  u64 v = 0;
  while (*s >= '0' && *s <= '9') {{
    v = v * 10 + (u64)(*s - '0');
    s++;
  }}
  return v;
}}

static int line_ends_with(const char *line, const char *end, const char *needle) {{
  long nlen = 0;
  while (needle[nlen]) nlen++;
  if (end - line < nlen) return 0;
  const char *p = end - nlen;
  for (long i = 0; i < nlen; i++) if (p[i] != needle[i]) return 0;
  return 1;
}}

static char MAPS_BUF[262144];

struct map_hit {{
  u64 start;
  u64 end;
  u64 foff;
  int found;
}};

static void find_maps(struct map_hit *appfw_rw, struct map_hit *libmpr_rx) {{
  appfw_rw->found = 0;
  libmpr_rx->found = 0;
  long fd = sc6(56, -100, (long)"/proc/self/maps", 0, 0, 0, 0);
  if (fd < 0) return;
  long total = 0;
  while (total < (long)sizeof(MAPS_BUF) - 1) {{
    long r = sc6(63, fd, (long)(MAPS_BUF + total), (long)sizeof(MAPS_BUF) - 1 - total, 0, 0, 0);
    if (r <= 0) break;
    total += r;
  }}
  sc6(57, fd, 0, 0, 0, 0, 0);
  MAPS_BUF[total] = 0;

  const char *p = MAPS_BUF;
  const char *lim = MAPS_BUF + total;
  while (p < lim) {{
    const char *nl = p;
    while (nl < lim && *nl != '\\n') nl++;
    const char *q = p;
    u64 st = parse_hex(q, &q);
    if (q < nl && *q == '-') {{
      q++;
      u64 en = parse_hex(q, &q);
      if (q + 6 < nl && *q == ' ') {{
        q++;
        const char *perms = q;
        q += 4;
        if (*q == ' ') {{
          q++;
          u64 foff = parse_hex(q, &q);
          if (!appfw_rw->found && perms[0] == 'r' && perms[1] == 'w' &&
              line_ends_with(p, nl, "/appFw.so") &&
              0x047C1EA0UL >= foff && 0x047C1F50UL <= foff + (en - st)) {{
            appfw_rw->start = st;
            appfw_rw->end = en;
            appfw_rw->foff = foff;
            appfw_rw->found = 1;
          }}
          if (!libmpr_rx->found && perms[0] == 'r' && perms[2] == 'x' &&
              foff == 0 && line_ends_with(p, nl, "/libmpr.so")) {{
            libmpr_rx->start = st;
            libmpr_rx->end = en;
            libmpr_rx->foff = 0;
            libmpr_rx->found = 1;
          }}
        }}
      }}
    }}
    p = (nl < lim) ? nl + 1 : lim;
  }}
}}

static int check_mem_table(long memfd, u64 base, u64 foff, const struct patch_rec *t, u32 n) {{
  u8 buf[16];
  for (u32 i = 0; i < n; i++) {{
    u64 addr = base + (u64)t[i].off - foff;
    long r = sc6(67, memfd, (long)buf, (long)t[i].len, (long)addr, 0, 0);
    if (r != (long)t[i].len) return 0;
    if (t[i].is_patch) {{
      if (!mem_eq(buf, t[i].oldb, t[i].len) && !mem_eq(buf, t[i].newb, t[i].len)) return 0;
    }} else {{
      if (!mem_eq(buf, t[i].oldb, t[i].len)) return 0;
    }}
  }}
  return 1;
}}

static int apply_mem_table(long memfd, u64 base, u64 foff, const struct patch_rec *t, u32 n) {{
  u8 buf[16];
  for (u32 i = 0; i < n; i++) {{
    if (!t[i].is_patch) continue;
    u64 addr = base + (u64)t[i].off - foff;
    long w = sc6(68, memfd, (long)t[i].newb, (long)t[i].len, (long)addr, 0, 0);
    if (w != (long)t[i].len) return 0;
    long r = sc6(67, memfd, (long)buf, (long)t[i].len, (long)addr, 0, 0);
    if (r != (long)t[i].len || !mem_eq(buf, t[i].newb, t[i].len)) return 0;
  }}
  return 1;
}}

static u64 read_liro_adr(void) {{
  long fd = sc6(56, -100, (long)"/sys/module/liro/parameters/adr", 0, 0, 0, 0);
  if (fd < 0) return 0;
  char buf[64];
  long r = sc6(63, fd, (long)buf, 63, 0, 0, 0);
  sc6(57, fd, 0, 0, 0, 0, 0);
  if (r <= 0) return 0;
  buf[r] = 0;
  return parse_dec(buf);
}}

__attribute__((constructor)) static void go(void) {{
  /* Detect /tmp_wbshd availability (real camera vs QEMU); on AArch64 O_DIRECTORY is 0x4000 */
  long dirfd = sc6(56, -100, (long)"/tmp_wbshd", 0x4000, 0, 0, 0); /* O_RDONLY | O_DIRECTORY */
  if (dirfd >= 0) {{
    sc6(57, dirfd, 0, 0, 0, 0, 0);
    use_tmp_fallback = 0;
  }} else {{
    use_tmp_fallback = 1;
  }}

  unlink_all_mk();
  wr_mk("start", "S");

  struct map_hit appfw_rw, libmpr_rx;
  find_maps(&appfw_rw, &libmpr_rx);
  if (!appfw_rw.found || !libmpr_rx.found) {{
    wr_mk("bad", !appfw_rw.found ? "NO_APPFW" : "NO_LIBMPR");
    wr_mk("done", "D");
    return;
  }}

  long memfd = sc6(56, -100, (long)"/proc/self/mem", 2, 0, 0, 0); /* O_RDWR */
  if (memfd < 0) {{
    wr_mk("bad", "SELF_MEM");
    wr_mk("done", "D");
    return;
  }}

  /* Phase 1: pre-check appFw.so + libmpr.so */
  if (!check_mem_table(memfd, appfw_rw.start, appfw_rw.foff, APPFW_REC, N_APPFW_REC)) {{
    wr_mk("mismatch", "APPFW");
    sc6(57, memfd, 0, 0, 0, 0, 0);
    wr_mk("done", "D");
    return;
  }}
  if (!check_mem_table(memfd, libmpr_rx.start, libmpr_rx.foff, LIBMPR_REC, N_LIBMPR_REC)) {{
    wr_mk("mismatch", "LIBMPR");
    sc6(57, memfd, 0, 0, 0, 0, 0);
    wr_mk("done", "D");
    return;
  }}

  /* Optional av-cam.bin physical memory check (present on real FX30, absent in QEMU) */
  u64 liro_phys = read_liro_adr();
  long devmem = -1;
  u8 *av_map = (u8 *)0;
  const u64 av_win_off = 0x380000UL;
  const u64 av_win_len = 0x050000UL;

  if (liro_phys) {{
    if (liro_phys & 0xFFFUL) {{
      wr_mk("bad", "LIRO_ALIGN");
      sc6(57, memfd, 0, 0, 0, 0, 0);
      wr_mk("done", "D");
      return;
    }}
    devmem = sc6(56, -100, (long)"/dev/mem", 0x101002, 0, 0, 0);
    if (devmem < 0) {{
      wr_mk("bad", "DEVMEM");
      sc6(57, memfd, 0, 0, 0, 0, 0);
      wr_mk("done", "D");
      return;
    }}
    i64 m = sc6(222, 0, (long)av_win_len, 3, 1, devmem, (long)(liro_phys + av_win_off));
    if (m < 0) {{
      wr_mk("bad", "AV_MMAP");
      sc6(57, devmem, 0, 0, 0, 0, 0);
      sc6(57, memfd, 0, 0, 0, 0, 0);
      wr_mk("done", "D");
      return;
    }}
    av_map = (u8 *)m;
    for (u32 i = 0; i < N_AVCAM_REC; i++) {{
      u8 *ptr = av_map + (AVCAM_REC[i].off - av_win_off);
      if (AVCAM_REC[i].is_patch) {{
        if (!mem_eq(ptr, AVCAM_REC[i].oldb, AVCAM_REC[i].len) &&
            !mem_eq(ptr, AVCAM_REC[i].newb, AVCAM_REC[i].len)) {{
          wr_mk("mismatch", "AVCAM");
          sc6(215, (long)av_map, (long)av_win_len, 0, 0, 0, 0);
          sc6(57, devmem, 0, 0, 0, 0, 0);
          sc6(57, memfd, 0, 0, 0, 0, 0);
          wr_mk("done", "D");
          return;
        }}
      }} else {{
        if (!mem_eq(ptr, AVCAM_REC[i].oldb, AVCAM_REC[i].len)) {{
          wr_mk("mismatch", "AVCAM_GUARD");
          sc6(215, (long)av_map, (long)av_win_len, 0, 0, 0, 0);
          sc6(57, devmem, 0, 0, 0, 0, 0);
          sc6(57, memfd, 0, 0, 0, 0, 0);
          wr_mk("done", "D");
          return;
        }}
      }}
    }}
  }} else {{
    wr_mk("no_liro", "QEMU_OR_NO_LIRO");
  }}

  /* Phase 2: apply + readback verify */
  int ok = 1;
  if (!apply_mem_table(memfd, appfw_rw.start, appfw_rw.foff, APPFW_REC, N_APPFW_REC)) ok = 0;
  if (ok && !apply_mem_table(memfd, libmpr_rx.start, libmpr_rx.foff, LIBMPR_REC, N_LIBMPR_REC)) ok = 0;
  if (ok && av_map) {{
    for (u32 i = 0; i < N_AVCAM_REC; i++) {{
      if (!AVCAM_REC[i].is_patch) continue;
      u8 *ptr = av_map + (AVCAM_REC[i].off - av_win_off);
      for (u32 k = 0; k < AVCAM_REC[i].len; k++) ptr[k] = AVCAM_REC[i].newb[k];
    }}
    asm volatile("dsb sy" ::: "memory");
    for (u32 i = 0; i < N_AVCAM_REC; i++) {{
      if (!AVCAM_REC[i].is_patch) continue;
      u8 *ptr = av_map + (AVCAM_REC[i].off - av_win_off);
      if (!mem_eq(ptr, AVCAM_REC[i].newb, AVCAM_REC[i].len)) {{ ok = 0; break; }}
    }}
  }}

  if (av_map) sc6(215, (long)av_map, (long)av_win_len, 0, 0, 0, 0);
  if (devmem >= 0) sc6(57, devmem, 0, 0, 0, 0, 0);
  sc6(57, memfd, 0, 0, 0, 0, 0);

  wr_mk(ok ? "ok" : "bad", ok ? "OK" : "WRITE_FAIL");
  wr_mk("done", "D");
}}
"""


def main() -> int:
    appfw_spec, libmpr_spec, avcam_spec = build_tables()
    c_src = generate_c(appfw_spec, libmpr_spec, avcam_spec)
    SRC_OUT.write_text(c_src, encoding="utf-8")
    print(f"Wrote {SRC_OUT} ({len(c_src)} bytes)")

    # Compile with native aarch64 gcc inside Lima VM 'fx30'
    cmd = [
        "limactl", "shell", "fx30",
        "aarch64-linux-gnu-gcc",
        "-shared", "-fPIC", "-nostdlib", "-ffreestanding", "-fno-builtin", "-O2", "-s",
        "-Wl,-z,max-page-size=4096,-soname,libJit.so",
        "-o", str(SO_OUT),
        str(SRC_OUT),
    ]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout, r.stderr)
        raise SystemExit(f"Compile failed rc={r.returncode}")

    raw = SO_OUT.read_bytes()
    sha = hashlib.sha256(raw).hexdigest()
    print(f"Compiled {SO_OUT} ({len(raw)} bytes, SHA-256: {sha})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
