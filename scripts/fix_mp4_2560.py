#!/usr/bin/env python3
"""Lossless MP4 container header fixer (avc1 + tkhd: 3840x2160 -> 3840x2560)
for Sony ILME-FX30 native 3840x2560 Open Gate recordings.

Usage:
  python3 fix_mp4_2560.py /path/to/clip.MP4 [more files or folders...]
"""
import struct
import sys
from pathlib import Path


def patch_mp4(path: Path) -> bool:
    data = bytearray(path.read_bytes())
    moov = data.rfind(b"moov")
    if moov == -1:
        print(f"[SKIP] {path.name}: no moov box found")
        return False

    c_avc1 = c_tkhd = 0
    i = data.find(b"avc1", moov)
    if i != -1:
        w, h = struct.unpack_from(">HH", data, i + 28)
        if (w, h) == (3840, 2160):
            struct.pack_into(">HH", data, i + 28, 3840, 2560)
            c_avc1 += 1

    pos = moov
    while True:
        i = data.find(b"tkhd", pos)
        if i == -1:
            break
        ver = data[i + 4]
        off = i + (80 if ver == 0 else 92)
        w, h = struct.unpack_from(">II", data, off)
        if (w >> 16, h >> 16) == (3840, 2160):
            struct.pack_into(">II", data, off, 3840 << 16, 2560 << 16)
            c_tkhd += 1
        pos = i + 4

    if c_avc1 or c_tkhd:
        path.write_bytes(data)
        print(f"[OK] {path.name}: patched avc1={c_avc1}, tkhd={c_tkhd} -> 3840x2560")
        return True
    print(f"[SKIP] {path.name}: already 3840x2560 or not 3840x2160")
    return False


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_mp4_2560.py <file_or_dir> [...]")
        sys.exit(1)
    files = []
    for arg in sys.argv[1:]:
        p = Path(arg)
        if p.is_dir():
            files.extend(sorted(p.glob("*.MP4")) + sorted(p.glob("*.mp4")))
        elif p.is_file():
            files.append(p)
    for f in files:
        patch_mp4(f)


if __name__ == "__main__":
    main()
