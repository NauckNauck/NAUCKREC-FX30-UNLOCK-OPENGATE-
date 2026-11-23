import json
import sys

A4 = "/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv_unpacked/firmware.tar_unpacked/0700_part_image/dev/nflasha8_unpacked/cpapp-b.bin"
J = "/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/modding/designs/ogsq_v3.json"

BASE = 0x108000
a4 = open(A4, "rb").read()
j = json.load(open(J))

for p in j["patches"]:
    if p["kind"] != "S":
        continue
    cp = int(p["addr"], 16)
    f = cp - BASE
    old = bytes.fromhex(p["old"])
    got = a4[f:f + len(old)]
    status = "OK" if got == old else "MISMATCH"
    print("%12s -> file %#09x expect %s got %s %s  (%s)" % (p["addr"], f, old.hex(), got.hex(), status, p["note"]))
