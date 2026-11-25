#!/usr/bin/env python3
"""Build libJit_level19_fx30og_v3.so — COMPLETE 110-site av-cam.bin + 7-site libmpr.so
Open Gate 3:2 (3840x2560) AP Encoder helper for Sony FX30 FW 6.01.
"""
import hashlib
import json
import os
import struct
from pathlib import Path

RESEARCH = Path(os.environ.get("FX30_RESEARCH_DIR", "/Users/nauck/Documents/sony_research"))
HELPER_DIR = RESEARCH / "a7iv-open-gate-test/windows/tools/camera_tests/CT6_memory_read_check/helper"
A7IV_JSON = RESEARCH / "a7iv-open-gate-test/windows/tools/modding/designs/ogfile2560_hs_s_v1.json"
FX30_AVCAM = RESEARCH / "fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha3_unpacked/av-cam.bin"
FX30_LIBMPR = RESEARCH / "fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha15_unpacked/lib/libmpr.so"
OUT_SO = RESEARCH / "fx30_open_gate/libJit_level19_fx30og_v3.so"

FX30_AVCAM_GUARD_0 = bytes.fromhex("d8882701c0ffffffd8882701c0ffffff")  # at 0x381b40
FX30_AVCAM_GUARD_1 = bytes.fromhex("c0fffffff89f2801c0ffffff20a02801")  # at 0x3838bc
ELF_HEADER_GUARD = bytes.fromhex("7f454c46020101030000000000000000")


def make_record(file_off: int, length: int, flags: int, old_bytes: bytes, new_bytes: bytes = b"") -> bytes:
    assert 1 <= length <= 16
    assert len(old_bytes) == length
    if flags == 1:
        assert len(new_bytes) == length
    return struct.pack("<IBB", file_off, length, flags) + old_bytes.ljust(16, b"\x00") + new_bytes.ljust(16, b"\x00") + b"\x00\x00"


def build():
    av = FX30_AVCAM.read_bytes()
    mpr = FX30_LIBMPR.read_bytes()
    a7og = json.loads(A7IV_JSON.read_text())

    t1_records = [
        make_record(0x381B40, 16, 0, FX30_AVCAM_GUARD_0),
        make_record(0x3838BC, 16, 0, FX30_AVCAM_GUARD_1),
    ]
    assert av[0x381B40 : 0x381B50] == FX30_AVCAM_GUARD_0
    assert av[0x3838BC : 0x3838CC] == FX30_AVCAM_GUARD_1

    clip_old = struct.pack("<3I", 8192, 4320, 3)
    clip_new = struct.pack("<3I", 3840, 2560, 23)
    assert av[0x390630 : 0x39063C] == clip_old, f"Clip format mismatch: {av[0x390630:0x39063c].hex()}"
    t1_records.append(make_record(0x390630, 12, 1, clip_old, clip_new))

    DELTA = 0xEE1C
    av_p = [p for p in a7og["patches"] if p["binary"] == "av-cam.bin" and p["kind"] != "A3-clip"]
    assert len(av_p) == 100
    for p in av_p:
        fx_off = int(p["file_off"], 16) + DELTA
        oldb = bytes.fromhex(p["old"])
        newb = bytes.fromhex(p["new"])
        assert av[fx_off : fx_off + len(oldb)] == oldb, f"FX30 av-cam.bin mismatch at {hex(fx_off)} ({p['note']})"
        t1_records.append(make_record(fx_off, len(oldb), 1, oldb, newb))

    assert len(t1_records) == 103
    # Extra FX30 AVC 23.98p 4:2:2 10-bit sites: +0x90 on 102600/01/03/04/07 + full key 102602 (desc #77 + prof)
    extra_avc_offs = [
        0x3AD378, 0x3AD678, 0x3AF878, 0x3AF978, 0x3B3778,  # +0x90 on 102600, 102601, 102603, 102604, 102607
        0x395328, 0x3AF6EE, 0x3AF720, 0x3AF778,            # desc #77 (102602) + prof +0x06/+0x38/+0x90
    ]
    for fx_off in extra_avc_offs:
        assert av[fx_off : fx_off + 2] == bytes.fromhex("7008"), f"Extra AVC mismatch at {hex(fx_off)}"
        t1_records.append(make_record(fx_off, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")))

    N_T1 = len(t1_records)
    assert N_T1 == 112
    print(f"[fx30og_v3] VERIFIED 100%: 2 guards + {N_T1 - 2} av-cam.bin patches = {N_T1} Table-1 records!")

    t2_spec = [
        (0, 16, 0, ELF_HEADER_GUARD, b""),
        (0x2CB138, 6, 0, bytes.fromhex("00000000000f"), b""),
        (0x2CB13E, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        (0x2CB16E, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        (0x2E0BB6, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        (0x2E0BEE, 2, 1, bytes.fromhex("7008"), bytes.fromhex("000a")),
        (0x1164B0, 4, 1, bytes.fromhex("000e8152"), bytes.fromhex("00408152")),  # XavcsMovieWriter level height 2160 -> 2560
        (0x180098, 4, 1, bytes.fromhex("000e8152"), bytes.fromhex("00408152")),  # TransMPFResolution2PubDefinition 2160 -> 2560
        (0x109768, 4, 1, bytes.fromhex("0b0e8152"), bytes.fromhex("0b408152")),  # xavcsMovieMeta::WriteMetaInfo 2160 -> 2560
    ]
    t2_records = []
    for off, ln, flg, oldb, newb in t2_spec:
        assert mpr[off : off + ln] == oldb, f"FX30 libmpr.so mismatch at {hex(off)}"
        t2_records.append(make_record(off, ln, flg, oldb, newb))
    print("[fx30og_v3] VERIFIED 100%: 2 guards + 7 libmpr.so patches = 9 Table-2 records!")

    src_path = HELPER_DIR / "libJit_level19_ogfile2560_hs_s_v1.so"
    data = bytearray(src_path.read_bytes())

    assert data[0x8B8 : 0x8B8 + 18] == b"ogfile2560_hs_s_v1"
    data[0x8B8 : 0x8B8 + 18] = b"fx30og_v3\x00\x00\x00\x00\x00\x00\x00\x00\x00"

    t1_bytes = b"".join(t1_records)
    data[0x940 : 0x940 + len(t1_bytes)] = t1_bytes
    data[0x940 + len(t1_bytes) : 0x940 + 132 * 40] = b"\x00" * ((132 - N_T1) * 40)

    assert data[0x2A0 : 0x2A4] == b"132\x00"
    data[0x2A0 : 0x2A4] = f"{N_T1}\x00".encode()

    assert data[0x315C : 0x3160] == bytes.fromhex("82108052")
    data[0x315C : 0x3160] = struct.pack("<I", 0x52800002 | (N_T1 << 5))
    assert data[0x3444 : 0x3448] == bytes.fromhex("df1202f1")
    data[0x3444 : 0x3448] = struct.pack("<I", 0xF10002DF | (N_T1 << 10))
    assert data[0x3D60 : 0x3D64] == bytes.fromhex("9f1302f1")
    data[0x3D60 : 0x3D64] = struct.pack("<I", 0xF100039F | (N_T1 << 10))

    assert data[0x2530 : 0x2534] == bytes.fromhex("e1171432")
    data[0x2530 : 0x2534] = bytes.fromhex("c100a052")
    assert data[0x3168 : 0x316C] == bytes.fromhex("e6171432")
    data[0x3168 : 0x316C] = bytes.fromhex("c600a052")
    assert data[0x4120 : 0x4124] == bytes.fromhex("e1171432")
    data[0x4120 : 0x4124] = bytes.fromhex("c100a052")

    t2_bytes = b"".join(t2_records)
    data[0x1DE0 : 0x1DE0 + len(t2_bytes)] = t2_bytes

    data[0x36F4 : 0x36F8] = bytes.fromhex("5adb3791")
    data[0x3774 : 0x3778] = bytes.fromhex("1f2003d5")
    data[0x3778 : 0x377C] = bytes.fromhex("1f2003d5")
    data[0x3780 : 0x3784] = bytes.fromhex("1f2003d5")
    data[0x3784 : 0x3788] = bytes.fromhex("eb0308aa")
    data[0x37A4 : 0x37A8] = bytes.fromhex("010b0054")
    data[0x37A8 : 0x37AC] = bytes.fromhex("f8ffff17")
    data[0x37AC : 0x37B0] = bytes.fromhex("1f2003d5")

    OUT_SO.write_bytes(data)
    (HELPER_DIR / "libJit_level19_fx30og_v3.so").write_bytes(data)
    sha = hashlib.sha256(data).hexdigest()
    print(f"Created {OUT_SO} ({len(data)} bytes, SHA-256: {sha})")


if __name__ == "__main__":
    build()
