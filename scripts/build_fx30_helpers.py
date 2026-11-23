#!/usr/bin/env python3
"""Build helper shared libraries for Sony FX30 firmware 6.01:
1. libJit_level19_fx30menu_v1.so: Patches HAITA rules in appFw.so to unlock All-Intra 4K & HFR menu lockout on V60 cards.
2. libJit_level19_fx30rate_v1.so: Patches RateCheck::GetMediaTransferRateInh in libmpr.so to allow recording on V60 cards.
"""

import hashlib
import struct
from pathlib import Path

RESEARCH_DIR = Path("/Users/nauck/Documents/sony_research")
HELPER_DIR = RESEARCH_DIR / "a7iv-open-gate-test" / "windows" / "tools" / "camera_tests" / "CT6_memory_read_check" / "helper"

# Stock guards for FX30 6.01
FX30_AVCAM_GUARD_0 = bytes.fromhex("d8882701c0ffffffd8882701c0ffffff")  # at av-cam.bin 0x381b40
FX30_AVCAM_GUARD_1 = bytes.fromhex("c0fffffff89f2801c0ffffff20a02801")  # at av-cam.bin 0x3838bc
ELF_HEADER_GUARD = bytes.fromhex("7f454c46020101030000000000000000")

# Guard at appFw.so HAITA rule 0x0270 (immediately precedes rule 0x02a6)
HAITA_GUARD = (0x047c1ea0, bytes.fromhex("70020000ae00150005000000ad001500"))

# 10 HAITA Rules for FX30 6.01 appFw.so (0x047c1eb0 .. 0x047c1f40)
HAITA_RULES = [
    (0x047c1eb0, bytes.fromhex("a602000000001a000500000001001a00"), bytes.fromhex("a602000000001a000000000001001a00")),
    (0x047c1ec0, bytes.fromhex("a702000002001a000500000003001a00"), bytes.fromhex("a702000002001a000000000003001a00")),
    (0x047c1ed0, bytes.fromhex("a802000004001a000500000005001a00"), bytes.fromhex("a802000004001a000000000005001a00")),
    (0x047c1ee0, bytes.fromhex("a902000006001a000500000007001a00"), bytes.fromhex("a902000006001a000000000007001a00")),
    (0x047c1ef0, bytes.fromhex("aa02000008001a000500000009001a00"), bytes.fromhex("aa02000008001a000000000009001a00")),
    (0x047c1f00, bytes.fromhex("ab0200000a001a00050000000b001a00"), bytes.fromhex("ab0200000a001a00000000000b001a00")),
    (0x047c1f10, bytes.fromhex("ac0200000c001a00050000000d001a00"), bytes.fromhex("ac0200000c001a00000000000d001a00")),
    (0x047c1f20, bytes.fromhex("ad0200000e001a00050000000f001a00"), bytes.fromhex("ad0200000e001a00000000000f001a00")),
    (0x047c1f30, bytes.fromhex("ae02000010001a000500000011001a00"), bytes.fromhex("ae02000010001a000000000011001a00")),
    (0x047c1f40, bytes.fromhex("af02000012001a000500000013001a00"), bytes.fromhex("af02000012001a000000000013001a00")),
]

# RateCheck patch for FX30 6.01 libmpr.so
RATECHECK_OFFSET = 0x103c88
RATECHECK_OLD = bytes.fromhex("ff8309d1001c005304040151fd7bbca9")
RATECHECK_NEW = bytes.fromhex("5f0000397f0000b9e0031f2ac0035fd6")


def make_record(file_off: int, length: int, flags: int, old_bytes: bytes, new_bytes: bytes = b"") -> bytes:
    """Build a 40-byte patch/guard record."""
    old_padded = old_bytes.ljust(16, b"\x00")
    new_padded = new_bytes.ljust(16, b"\x00")
    return struct.pack("<IBB", file_off, length, flags) + old_padded + new_padded + b"\x00\x00"


def build_menu_helper():
    src_path = HELPER_DIR / "libJit_level19_lcdcycle_v6.so"
    data = bytearray(src_path.read_bytes())

    # 1. Update design name at 0x8b5 ("lcdcycle_v6" -> "fx30menu_v1", both 11 chars)
    name_pos = 0x8b5
    assert data[name_pos : name_pos + 11] == b"lcdcycle_v6"
    data[name_pos : name_pos + 11] = b"fx30menu_v1"

    # 2. Update Table 1 guards for FX30 av-cam.bin (at 0x938, 2 entries)
    rec0 = make_record(0x381b40, 16, 0, FX30_AVCAM_GUARD_0)
    rec1 = make_record(0x3838bc, 16, 0, FX30_AVCAM_GUARD_1)
    data[0x938 : 0x938 + 40] = rec0
    data[0x960 : 0x960 + 40] = rec1

    # 3. Build Table 2 at 0x988 (11 entries total: 1 guard + 10 HAITA rules)
    t2_records = [make_record(HAITA_GUARD[0], 16, 0, HAITA_GUARD[1])]
    for off, old, new in HAITA_RULES:
        t2_records.append(make_record(off, 16, 1, old, new))

    assert len(t2_records) == 11
    t2_bytes = b"".join(t2_records)
    t2_start = 0x988
    data[t2_start : t2_start + len(t2_bytes)] = t2_bytes

    # Zero out remaining old entries (from index 11 to 94: (95 - 11) * 40 = 3360 bytes)
    zero_start = t2_start + len(t2_bytes)
    zero_len = (95 - 11) * 40
    data[zero_start : zero_start + zero_len] = b"\x00" * zero_len

    # 4. Patch maps scanner for appFw.so rw-p data segment:
    # 0x2234: cbz w2, #0x2274 -> mov x2, #0x47c0000 (keep x9 intact as maps fd!)
    assert data[0x2234 : 0x2238] == bytes.fromhex("02020034")
    data[0x2234 : 0x2238] = bytes.fromhex("828fa0d2")

    # 0x2238: cbnz x13, #0x2274 -> cmp x13, x2
    assert data[0x2238 : 0x223c] == bytes.fromhex("ed0100b5")
    data[0x2238 : 0x223c] = bytes.fromhex("bf0102eb")

    # 0x223c: cmp x18, x17 -> b.ne #0x2260 (branch to 0x2260 to reset line buffer index x15 on mismatch!)
    assert data[0x223c : 0x2240] == bytes.fromhex("5f0211eb")
    data[0x223c : 0x2240] = bytes.fromhex("21010054")

    # 0x2240: b.ls #0x2274 -> nop
    assert data[0x2240 : 0x2244] == bytes.fromhex("a9010054")
    data[0x2240 : 0x2244] = bytes.fromhex("1f2003d5")

    # 0x2254: mov x23, x17 -> sub x23, x17, x2 (base = VA - file_offset, where x2 is 0x047c0000)
    assert data[0x2254 : 0x2258] == bytes.fromhex("f70311aa")
    data[0x2254 : 0x2258] = bytes.fromhex("370202cb")

    # 0x225c: b #0x2010 -> b #0x2280 (break out of maps scan immediately once appFw.so rw-p is found!)
    assert data[0x225c : 0x2260] == bytes.fromhex("6dffff17")
    data[0x225c : 0x2260] = bytes.fromhex("09000014")

    # 5. Patch loop count instructions and log entry count for Table 2: 95 -> 11 (0x0b)
    # 0x2960: mov w9, #0x3539 (ASCII "95") -> mov w9, #0x3131 (ASCII "11")
    assert data[0x2960 : 0x2964] == bytes.fromhex("29a78652")
    data[0x2960 : 0x2964] = bytes.fromhex("29268652")

    # 0x2ab8: mov w2, #0x5f -> mov w2, #0x0b
    assert data[0x2ab8 : 0x2abc] == bytes.fromhex("e20b8052")
    data[0x2ab8 : 0x2abc] = bytes.fromhex("62018052")

    # 0x3064: cmp x22, #0x5f -> cmp x22, #0x0b
    assert data[0x3064 : 0x3068] == bytes.fromhex("df7e01f1")
    data[0x3064 : 0x3068] = bytes.fromhex("df2e00f1")

    # 0x33ec: mov w10, #0x5e -> mov w10, #0x0a
    assert data[0x33ec : 0x33f0] == bytes.fromhex("ca0b8052")
    data[0x33ec : 0x33f0] = bytes.fromhex("4a018052")

    # 0x3408: cmp x26, #0x5f -> cmp x26, #0x0b
    assert data[0x3408 : 0x340c] == bytes.fromhex("5f7f01f1")
    data[0x3408 : 0x340c] = bytes.fromhex("5f2f00f1")

    # 0x3598: mov w10, #0x5e -> mov w10, #0x0a
    assert data[0x3598 : 0x359c] == bytes.fromhex("ca0b8052")
    data[0x3598 : 0x359c] = bytes.fromhex("4a018052")

    # Write both name variants
    for name in ("libJit_level19_fx30menu_v1.so", "libJit_level19_fx30_menu_v1.so"):
        dst_path = HELPER_DIR / name
        dst_path.write_bytes(data)

    sha = hashlib.sha256(data).hexdigest()
    print(f"Created libJit_level19_fx30menu_v1.so: {len(data)} bytes, SHA-256: {sha}")
    return sha


def build_rate_helper():
    src_path = HELPER_DIR / "libJit_level19_ogfile2560_hs_s_v1.so"
    data = bytearray(src_path.read_bytes())

    # 1. Update design name at 0x8b8 ("ogfile2560_hs_s_v1" (18 bytes) -> "fx30rate_v1\x00..." (18 bytes))
    name_pos = 0x8b8
    assert data[name_pos : name_pos + 18] == b"ogfile2560_hs_s_v1"
    data[name_pos : name_pos + 18] = b"fx30rate_v1\x00\x00\x00\x00\x00\x00\x00"

    # 2. Table 1 (av-cam.bin) starts at 0x940: 2 entries for FX30 av-cam.bin guards
    t1_start = 0x940
    rec0 = make_record(0x381b40, 16, 0, FX30_AVCAM_GUARD_0)
    rec1 = make_record(0x3838bc, 16, 0, FX30_AVCAM_GUARD_1)
    data[t1_start : t1_start + 40] = rec0
    data[t1_start + 40 : t1_start + 80] = rec1
    # Zero out remaining 130 entries of Table 1: 130 * 40 = 5200 bytes
    t1_zero_start = t1_start + 80
    t1_zero_len = 130 * 40
    data[t1_zero_start : t1_zero_start + t1_zero_len] = b"\x00" * t1_zero_len

    # Table 1 print count string at 0x2a0: "132\x00" -> "2\x00\x00\x00"
    assert data[0x2a0 : 0x2a4] == b"132\x00"
    data[0x2a0 : 0x2a4] = b"2\x00\x00\x00"

    # Table 1 loop counts: 132 (0x84) -> 2
    # 0x315c: mov w2, #0x84 (82 10 80 52) -> mov w2, #2 (42 00 80 52)
    assert data[0x315c : 0x3160] == bytes.fromhex("82108052")
    data[0x315c : 0x3160] = bytes.fromhex("42008052")

    # 0x3444: cmp x22, #0x84 (df 12 02 f1) -> cmp x22, #2 (df 0a 00 f1)
    assert data[0x3444 : 0x3448] == bytes.fromhex("df1202f1")
    data[0x3444 : 0x3448] = bytes.fromhex("df0a00f1")

    # 0x3d60: cmp x28, #0x84 (9f 13 02 f1) -> cmp x28, #2 (9f 0b 00 f1)
    assert data[0x3d60 : 0x3d64] == bytes.fromhex("9f1302f1")
    data[0x3d60 : 0x3d64] = bytes.fromhex("9f0b00f1")

    # 3. Table 2 (libmpr.so) at 0x1de0: 2 entries (1 guard + 1 patch)
    t2_start = 0x1de0
    t2_rec0 = make_record(0, 16, 0, ELF_HEADER_GUARD)
    t2_rec1 = make_record(RATECHECK_OFFSET, 16, 1, RATECHECK_OLD, RATECHECK_NEW)
    data[t2_start : t2_start + 40] = t2_rec0
    data[t2_start + 40 : t2_start + 80] = t2_rec1
    # Zero out remaining 7 entries of Table 2: 7 * 40 = 280 bytes
    t2_zero_start = t2_start + 80
    t2_zero_len = 7 * 40
    data[t2_zero_start : t2_zero_start + t2_zero_len] = b"\x00" * t2_zero_len

    # Table 2 print count at 0x304c: mov w9, #0x39 (ASCII '9') -> mov w9, #0x32 (ASCII '2')
    assert data[0x304c : 0x3050] == bytes.fromhex("29078052")
    data[0x304c : 0x3050] = bytes.fromhex("49068052")

    # Table 2 loop counts: 9 -> 2
    # 0x319c: mov w2, #9 (22 01 80 52) -> mov w2, #2 (42 00 80 52)
    assert data[0x319c : 0x31a0] == bytes.fromhex("22018052")
    data[0x319c : 0x31a0] = bytes.fromhex("42008052")

    # 0x3718: cmp x22, #9 (df 26 00 f1) -> cmp x22, #2 (df 0a 00 f1)
    assert data[0x3718 : 0x371c] == bytes.fromhex("df2600f1")
    data[0x3718 : 0x371c] = bytes.fromhex("df0a00f1")

    # 0x3ab0: cmp x26, #9 (5f 27 00 f1) -> cmp x26, #2 (5f 0b 00 f1)
    assert data[0x3ab0 : 0x3ab4] == bytes.fromhex("5f2700f1")
    data[0x3ab0 : 0x3ab4] = bytes.fromhex("5f0b00f1")

    # 4. Patch Table 2 apply verification loop to compare full new_bytes without assuming byte 0 is 0:
    # 0x36f4 (VA 0x46f4): add x26, x26, #0xdf7 -> add x26, x26, #0xdf6 (point x26 to new_bytes[0])
    assert data[0x36f4 : 0x36f8] == bytes.fromhex("5adf3791")
    data[0x36f4 : 0x36f8] = bytes.fromhex("5adb3791")

    # 0x3774 (VA 0x4774): ldrb w9, [x23, x10] -> nop
    assert data[0x3774 : 0x3778] == bytes.fromhex("e96a6a38")
    data[0x3774 : 0x3778] = bytes.fromhex("1f2003d5")

    # 0x3778 (VA 0x4778): cbnz w9, #0x4904 -> nop
    assert data[0x3778 : 0x377c] == bytes.fromhex("690c0035")
    data[0x3778 : 0x377c] = bytes.fromhex("1f2003d5")

    # 0x3780 (VA 0x4780): add w10, w10, #1 -> nop
    assert data[0x3780 : 0x3784] == bytes.fromhex("4a050011")
    data[0x3780 : 0x3784] = bytes.fromhex("1f2003d5")

    # 0x3784 (VA 0x4784): sub x11, x8, #1 -> mov x11, x8
    assert data[0x3784 : 0x3788] == bytes.fromhex("0b0500d1")
    data[0x3784 : 0x3788] = bytes.fromhex("eb0308aa")

    # 0x37a4 (VA 0x47a4): b.eq #0x4788 -> b.ne #0x4904
    assert data[0x37a4 : 0x37a8] == bytes.fromhex("20ffff54")
    data[0x37a4 : 0x37a8] = bytes.fromhex("010b0054")

    # 0x37a8 (VA 0x47a8): cmp x9, x8 -> b #0x4788
    assert data[0x37a8 : 0x37ac] == bytes.fromhex("3f0108eb")
    data[0x37a8 : 0x37ac] = bytes.fromhex("f8ffff17")

    # 0x37ac (VA 0x47ac): b.lo #0x4904 -> nop
    assert data[0x37ac : 0x37b0] == bytes.fromhex("c30a0054")
    data[0x37ac : 0x37b0] = bytes.fromhex("1f2003d5")

    # Write both name variants
    for name in ("libJit_level19_fx30rate_v1.so", "libJit_level19_fx30_rate_v1.so"):
        dst_path = HELPER_DIR / name
        dst_path.write_bytes(data)

    sha = hashlib.sha256(data).hexdigest()
    print(f"Created libJit_level19_fx30rate_v1.so: {len(data)} bytes, SHA-256: {sha}")
    return sha


def build_lcd_helper():
    src_path = HELPER_DIR / "libJit_level16_lcdfill_v2.so"
    data = bytearray(src_path.read_bytes())

    # 1. Update design name at 0x979 ("lcdfill_v2" -> "fx30lcd_v2", both 10 chars)
    name_pos = 0x979
    assert data[name_pos : name_pos + 10] == b"lcdfill_v2"
    data[name_pos : name_pos + 10] = b"fx30lcd_v2"

    # 2. Update sites at 0xa34
    site_replacements = [
        (0x1e4bcc, 0x1e2d3c),
        (0xd7b5a0, 0xde791c),
        (0xd7b5a4, 0xde7920),
        (0xd7b618, 0xde7994),
        (0xd7b61c, 0xde7998),
        (0xd7b690, 0xde7a0c),
        (0xd7b694, 0xde7a10),
    ]

    for i, (old_a, new_a) in enumerate(site_replacements):
        off = 0xa34 + i * 12
        cur_a = struct.unpack_from("<I", data, off)[0]
        assert cur_a == old_a, f"Site {i}: expected {hex(old_a)}, got {hex(cur_a)}"
        struct.pack_into("<I", data, off, new_a)

    # 3. Update guards at 0xa88
    guard_replacements = [
        (0x1e4b78, 0x1e2ce8),
        (0x1e4bb8, 0x1e2d28),
        (0x1e4bc8, 0x1e2d38),
        (0x1e4bd0, 0x1e2d40),
        (0x1e4be0, 0x1e2d50),
        (0xd7b598, 0xde7914),
        (0xd7b5a8, 0xde7924),
        (0xd7b610, 0xde798c),
        (0xd7b620, 0xde799c),
        (0xd7b688, 0xde7a04),
        (0xd7b698, 0xde7a14),
    ]

    for i, (old_a, new_a) in enumerate(guard_replacements):
        off = 0xa88 + i * 24
        cur_a = struct.unpack_from("<I", data, off)[0]
        assert cur_a == old_a, f"Guard {i}: expected {hex(old_a)}, got {hex(cur_a)}"
        struct.pack_into("<I", data, off, new_a)

    dst_path = HELPER_DIR / "libJit_level16_fx30lcd_v2.so"
    dst_path.write_bytes(data)

    sha = hashlib.sha256(data).hexdigest()
    print(f"Created libJit_level16_fx30lcd_v2.so: {len(data)} bytes, SHA-256: {sha}")
    return sha


def build_og_helper():
    src_path = HELPER_DIR / "libJit_level19_ogfile2560_hs_s_v1.so"
    data = bytearray(src_path.read_bytes())

    # 1. Update design name at 0x8b8
    name_pos = 0x8b8
    assert data[name_pos : name_pos + 18] == b"ogfile2560_hs_s_v1"
    data[name_pos : name_pos + 18] = b"fx30og_v2\x00\x00\x00\x00\x00\x00\x00\x00\x00"

    # 2. Table 1: 21 records
    t1_records = [
        make_record(0x381b40, 16, 0, FX30_AVCAM_GUARD_0),
        make_record(0x3838bc, 16, 0, FX30_AVCAM_GUARD_1),
        make_record(0x390640, 12, 1, bytes.fromhex("001e0000e010000004000000"), bytes.fromhex("000f0000000a000010000000")),
        make_record(0x390674, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3926e8, 16, 1, bytes.fromhex("00000000730400007604000079040000"), bytes.fromhex("8604000074040000770400007a040000")),
        # Key 101702
        make_record(0x3a0c38, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c4220, 4, 1, bytes.fromhex("70080000"), bytes.fromhex("000a0000")),
        make_record(0x3c4274, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c42ee, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # Key 101901
        make_record(0x3a0cc8, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c4320, 4, 1, bytes.fromhex("70080000"), bytes.fromhex("000a0000")),
        make_record(0x3c4374, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c43ee, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # Key 102000
        make_record(0x3a0d58, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c4420, 4, 1, bytes.fromhex("70080000"), bytes.fromhex("000a0000")),
        make_record(0x3c4474, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c44ee, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # Key 101502
        make_record(0x3a0b18, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c4020, 4, 1, bytes.fromhex("70080000"), bytes.fromhex("000a0000")),
        make_record(0x3c4074, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        make_record(0x3c40ee, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
    ]
    assert len(t1_records) == 21
    t1_bytes = b"".join(t1_records)
    t1_start = 0x940
    data[t1_start : t1_start + len(t1_bytes)] = t1_bytes
    zero_start = t1_start + len(t1_bytes)
    zero_len = (132 - 21) * 40
    data[zero_start : zero_start + zero_len] = b"\x00" * zero_len

    # Table 1 print count string at 0x2a0
    assert data[0x2a0 : 0x2a4] == b"132\x00"
    data[0x2a0 : 0x2a4] = b"21\x00\x00"

    # Table 1 loop counts: 132 -> 21 (0x15)
    assert data[0x315c : 0x3160] == bytes.fromhex("82108052")
    data[0x315c : 0x3160] = bytes.fromhex("a2028052")

    assert data[0x3444 : 0x3448] == bytes.fromhex("df1202f1")
    data[0x3444 : 0x3448] = bytes.fromhex("df5600f1")

    assert data[0x3d60 : 0x3d64] == bytes.fromhex("9f1302f1")
    data[0x3d60 : 0x3d64] = bytes.fromhex("9f5700f1")

    # Expand av-cam.bin mmap and check range from 0x3f000 to 0x60000 (covers up to 0x3e0000):
    # 0x2530: mov w1, #0x3f000 (mmap length) -> mov w1, #0x60000
    assert data[0x2530 : 0x2534] == bytes.fromhex("e1171432")
    data[0x2530 : 0x2534] = bytes.fromhex("c100a052")

    # 0x3168: mov w6, #0x3f000 (range check limit) -> mov w6, #0x60000
    assert data[0x3168 : 0x316c] == bytes.fromhex("e6171432")
    data[0x3168 : 0x316c] = bytes.fromhex("c600a052")

    # 0x4120: mov w1, #0x3f000 (munmap length) -> mov w1, #0x60000
    assert data[0x4120 : 0x4124] == bytes.fromhex("e1171432")
    data[0x4120 : 0x4124] = bytes.fromhex("c100a052")

    # 3. Table 2: EXACTLY 9 records (both AVC and HEVC!)
    t2_records = [
        make_record(0, 16, 0, ELF_HEADER_GUARD),
        # AVC 4:2:0 8-bit (XAVC S 4K)
        make_record(0x2cb138, 6, 0, bytes.fromhex("00000000000f")),
        make_record(0x2cb13e, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # AVC 4:2:2 10-bit (XAVC S-I 4K)
        make_record(0x2cb168, 6, 0, bytes.fromhex("00000000000f")),
        make_record(0x2cb16e, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # HEVC 4:2:0 10-bit (XAVC HS 4K)
        make_record(0x2e0bb0, 6, 0, bytes.fromhex("00000000000f")),
        make_record(0x2e0bb6, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        # HEVC 4:2:2 10-bit (XAVC HS 4K 4:2:2)
        make_record(0x2e0be8, 6, 0, bytes.fromhex("00000000000f")),
        make_record(0x2e0bee, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
    ]
    assert len(t2_records) == 9
    t2_bytes = b"".join(t2_records)
    t2_start = 0x1de0
    data[t2_start : t2_start + len(t2_bytes)] = t2_bytes

    # Table 2 verification fix
    data[0x36f4 : 0x36f8] = bytes.fromhex("5adb3791")
    data[0x3774 : 0x3778] = bytes.fromhex("1f2003d5")
    data[0x3778 : 0x377c] = bytes.fromhex("1f2003d5")
    data[0x3780 : 0x3784] = bytes.fromhex("1f2003d5")
    data[0x3784 : 0x3788] = bytes.fromhex("eb0308aa")
    data[0x37a4 : 0x37a8] = bytes.fromhex("010b0054")
    data[0x37a8 : 0x37ac] = bytes.fromhex("f8ffff17")
    data[0x37ac : 0x37b0] = bytes.fromhex("1f2003d5")

    dst = HELPER_DIR / "libJit_level19_fx30og_v2.so"
    dst.write_bytes(data)
    sha = hashlib.sha256(data).hexdigest()
    print(f"Created libJit_level19_fx30og_v2.so: {len(data)} bytes, SHA-256: {sha}")
    return sha


if __name__ == "__main__":
    menu_sha = build_menu_helper()
    rate_sha = build_rate_helper()
    lcd_sha = build_lcd_helper()
    og_sha = build_og_helper()
    print(f"\nCompleted build:")
    print(f"  FX30 Menu Helper SHA-256: {menu_sha}")
    print(f"  FX30 Rate Helper SHA-256: {rate_sha}")
    print(f"  FX30 LCD Preview Helper SHA-256: {lcd_sha}")
    print(f"  FX30 Open Gate v2 Helper SHA-256: {og_sha}")
