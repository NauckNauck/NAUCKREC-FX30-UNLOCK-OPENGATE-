import sys

ROOT = "/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research"
ROM = ROOT + "/fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha8_unpacked/cpapp-b.bin"
DMP = ROOT + "/fx30_dump_helper/fx30_cp_dump.bin"
IMG = ROOT + "/fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha8_unpacked/cpImg.dat"

BASE = 0x108000
D0 = 0xC0000

rom = open(ROM, "rb").read()
dmp = open(DMP, "rb").read()
img = open(IMG, "rb").read()

print("rom %#x  dump %#x (CP %#x..%#x)  cpImg %#x" % (len(rom), len(dmp), D0, D0 + len(dmp), len(img)))

CH = 0x10000
rows = []
for off in range(0, len(dmp), CH):
    cp = D0 + off
    chunk = dmp[off:off + CH]
    f = cp - BASE
    if 0 <= f and f + len(chunk) <= len(rom):
        r = rom[f:f + len(chunk)]
        same = chunk == r
        rz = r.count(0) == len(r)
        dz = chunk.count(0) == len(chunk)
        if same:
            kind = "same" if not rz else "both-zero"
        elif rz and not dz:
            kind = "RAM-only(rom zero)"
        else:
            kind = "DIFF"
    else:
        kind = "no-rom"
    rows.append((cp, kind, chunk.count(0) / len(chunk)))

# summarize contiguous runs
cur = None
start = None
for cp, kind, z in rows:
    if kind != cur:
        if cur is not None:
            print("%08x..%08x  %s" % (start, prev, cur))
        cur = kind
        start = cp
    prev = cp
print("%08x..%08x  %s" % (start, prev, cur))
