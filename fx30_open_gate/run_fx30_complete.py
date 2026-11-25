#!/usr/bin/env python3
"""Unified Sony FX30 (FW 6.01 / IMX684) Volatile RAM Runner:
  --step1           V60 Card Unlock (fx30menu_v1 + fx30rate_v1) + fx30_ogsq_v6.so
  --full            V60 Card Unlock + fx30og_v3 (101-site encoder) + fx30_ogsq_v6 + fx30_outsz_lcd_v1
  --with-dmmwb      Also inject fx30_dmmwb_safe_v2.so (contingency 2x28.5MB steered DMM)
  --probe           Run fx30_read_probe_v2.so (in Movie mode -> IMG INIT cure) & decode probe_dump.bin
  --card-only       Run V60 Card Unlock only (fx30menu_v1 + fx30rate_v1)
  --verify-offline  Verify SHA-256 & ELF integrity of all 7 binaries without camera connected
"""
import argparse
import hashlib
import os
import re
import struct
import subprocess
import sys
import time
from pathlib import Path

RESEARCH = Path("/Users/nauck/Documents/sony_research")
OG_DIR = RESEARCH / "fx30_open_gate"
A7IV = RESEARCH / "a7iv-open-gate-test"
CT1 = A7IV / "windows" / "tools" / "camera_tests" / "CT1_usb_first_contact"
CT6 = A7IV / "windows" / "tools" / "camera_tests" / "CT6_memory_read_check"
HELPER_DIR = CT6 / "helper"
LISTER = CT1 / "ct1_stage2_service.py"
RUNNER = CT6 / "ct6_level1_run.py"

sys.path.insert(0, str(CT1))
sys.path.insert(0, str(CT6))

AP_STEPS = {
    "fx30menu_v1": (19, HELPER_DIR / "libJit_level19_fx30menu_v1.so", "809ddcf293c1e42f94c6460ff32078891d246a43498550bc6ba7cbfb8f11848b"),
    "fx30rate_v1": (19, HELPER_DIR / "libJit_level19_fx30rate_v1.so", "b86b3a263dfdb8895f44129daf342cfc4f30a0f885f80b21d550cf7d02a052d0"),
    "fx30og_v3":   (19, HELPER_DIR / "libJit_level19_fx30og_v3.so",   "ab701d59ca211b2543ebfe48fcd045ed10c55906e1dca81d96f5cf28785c1269"),
}
CP_BINS = {
    "fx30_ogsq_v6":       OG_DIR / "fx30_ogsq_v6.so",
    "fx30_outsz_lcd_v1":  OG_DIR / "fx30_outsz_lcd_v1.so",
    "fx30_dmmwb_safe_v2": OG_DIR / "fx30_dmmwb_safe_v2.so",
    "fx30_read_probe_v2": OG_DIR / "fx30_read_probe_v2.so",
}

SET_FILE_PATH, GET_FILE2, SET_FILE2, FILE_DELETE2 = 0x400F, 0x4011, 0x4010, 0x4012
LIB_PATH = b"/tmp/jiritsu/libJit.so"


def verify_offline() -> int:
    print("=== OFFLINE INTEGRITY CHECK (7 BINARIES) ===")
    ok = True
    for name, (lvl, path, expected_sha) in AP_STEPS.items():
        if not path.is_file():
            print(f"  [MISSING] {name}: {path}")
            ok = False
            continue
        got = hashlib.sha256(path.read_bytes()).hexdigest()
        match = (got == expected_sha)
        print(f"  [{'OK' if match else 'BAD SHA'}] {name:18s} ({path.stat().st_size} B) sha256={got[:16]}...")
        if not match:
            ok = False
    for name, path in CP_BINS.items():
        if not path.is_file():
            print(f"  [MISSING] {name}: {path}")
            ok = False
            continue
        raw = path.read_bytes()
        is_elf = raw[:4] == b"\x7fELF" and raw[4] == 2 and struct.unpack_from("<H", raw, 18)[0] == 183
        has_sencore = b"libsencore.so" in raw
        has_init = b".init_array" in raw
        good = is_elf and has_sencore and has_init
        print(f"  [{'OK' if good else 'BAD ELF'}] {name:18s} ({len(raw)} B) aarch64={is_elf} libsencore={has_sencore} init_array={has_init}")
        if not good:
            ok = False
    return 0 if ok else 1


def usb_state() -> str:
    r = subprocess.run([sys.executable, "-I", "-B", str(LISTER), "--list"], capture_output=True, text=True, errors="replace", cwd=str(CT6.parent))
    m = re.search(r"libusb sees: (\d+) Sony mass-storage device\(s\).*?, (\d+) Sony service device\(s\)", r.stdout)
    if not m:
        return "other"
    return "drive" if int(m.group(1)) == 1 and int(m.group(2)) == 0 else "service" if int(m.group(2)) else "other"


def wait_for_drive(first: bool, mode_hint: str = "STILLS (Photo)"):
    if not first:
        print("\n>>> Unplug USB-C from the camera and plug it back in (MSC).")
        print(">>> If 'Image Transfer (MSC)' appears, press the center circle button.")
        end_u = time.time() + 30
        while time.time() < end_u and usb_state() == "drive":
            time.sleep(0.5)
    asked = False
    while True:
        if usb_state() == "drive":
            time.sleep(2)
            return
        if not asked:
            print(f"\n>>> Waiting for FX30 in USB Mass Storage ({mode_hint})...")
            asked = True
        time.sleep(1)


def run_ct6_step(design: str):
    lvl, _, _ = AP_STEPS[design]
    print(f"\n=== Running AP Step: {design} (level {lvl}) ===")
    r = subprocess.run([sys.executable, "-I", "-B", str(RUNNER), "--level", str(lvl), "--design", design], cwd=str(CT6))
    if r.returncode != 0:
        raise SystemExit(f"AP step {design} failed with exit code {r.returncode}")


def inject_cp_helpers(helpers: list, dump_out: Path = None):
    import ct1_common as c
    import ct1_stage2_service as service
    from pmca.usb.sony import SonySenserAuthDevice, SonySenserCamera, SonySenserDevice

    all_markers = []
    for _, _, mks in helpers:
        all_markers.extend(mks)
    allowed_paths = {LIB_PATH, b"../../tmp_wbshd/probe_dump.bin", *all_markers}
    rules = {
        (c.PFUNC_ADJUST, 0x1001, SET_FILE_PATH): ("SET_FILE_PATH", lambda p: p[:1] == b"\x00" and p[1:] in allowed_paths),
        (c.PFUNC_ADJUST, 0x1001, GET_FILE2): ("GET_FILE2", lambda p: p == b"\x00\xff\x00"),
        (c.PFUNC_ADJUST, 0x1001, SET_FILE2): ("SET_FILE2", lambda p: p[:3] == b"\x00\xff\x00"),
        (c.PFUNC_ADJUST, 0x1001, FILE_DELETE2): ("FILE_DELETE2", lambda p: p == b"\x00\xff"),
    }

    class FlexGuard(c.ServiceGuard):
        def _send(self, pFunc, data, _oData):
            dev = self._device
            hdr = struct.pack(c.SENSER_HEADER, len(data), pFunc, dev._sequence, 0, 0, 0, 0)
            dev.driver.write(hdr + bytes(data))
            resp, out = c.read_service_answer(dev.driver, dev._sequence, self.session.log)
            dev._sequence += 1
            return resp, out

    def cmd(cam, code, payload):
        ans = cam._sendAdjustControlPacket(0x1001, code, payload)
        if len(ans) < 4:
            return None, b""
        return struct.unpack_from("<I", ans)[0], ans[4:]

    session = c.Session("fx30_complete", test="FX30COMPLETE")
    try:
        inner = service.connect(session)
        drv = c.GuardedDriver(inner, session, "senser")
        auth = SonySenserAuthDevice(drv)
        auth.start()
        try:
            auth.authenticate()
            dev = SonySenserDevice(drv)
            FlexGuard(dev, session, rules)
            cam = SonySenserCamera(dev)
            for name, so_path, markers in helpers:
                print(f"\n=== Injecting CP Helper: {name} ({so_path.stat().st_size} B) ===")
                cmd(cam, SET_FILE_PATH, b"\x00" + LIB_PATH)
                cmd(cam, FILE_DELETE2, b"\x00\xff")
                cmd(cam, SET_FILE_PATH, b"\x00" + LIB_PATH)
                st, _ = cmd(cam, SET_FILE2, b"\x00\xff\x00" + so_path.read_bytes())
                print(f"  write/load status: {st}")
                time.sleep(3)
                for mk in markers:
                    cmd(cam, SET_FILE_PATH, b"\x00" + mk)
                    mst, md = cmd(cam, GET_FILE2, b"\x00\xff\x00")
                    if mst == 0 or md:
                        print(f"  marker {mk.decode()}: present={mst == 0} content={md!r}")
                cmd(cam, SET_FILE_PATH, b"\x00" + LIB_PATH)
                cmd(cam, FILE_DELETE2, b"\x00\xff")
            if dump_out is not None:
                cmd(cam, SET_FILE_PATH, b"\x00../../tmp_wbshd/probe_dump.bin")
                dst, dbytes = cmd(cam, GET_FILE2, b"\x00\xff\x00")
                if dst == 0 and dbytes:
                    dump_out.write_bytes(dbytes)
                    print(f"\nSaved live probe dump ({len(dbytes)} B) -> {dump_out}")
        finally:
            auth.stop()
            print("Stop request sent (IMG INIT triggered).")
        session.finish("completed")
    except BaseException as exc:
        session.finish("failed", f"{type(exc).__name__}: {exc}")
        raise


def decode_probe(dump_path: Path):
    d = dump_path.read_bytes()
    rec = d[0:352]
    p2s = d[512:640]
    outsz = d[640:672]
    s2 = d[672:692]
    s4 = d[692:704]
    s1w = d[704:720]
    m2sc = d[720:736]
    lcd = d[736:744]

    print("\n=== LIVE PROBE DECODE ===")
    print(f"  ImagerDrvParam M6: vratio={rec[6]}/{rec[7]} (exp 27/32) S5={struct.unpack_from('<H',rec,0x62)[0]} H={struct.unpack_from('<H',rec,0x66)[0]} S6={struct.unpack_from('<H',rec,0x6a)[0]} S10={struct.unpack_from('<H',rec,0x8c)[0]} cropV=({struct.unpack_from('<H',rec,0xf6)[0]},{struct.unpack_from('<H',rec,0x102)[0]}) n7={struct.unpack_from('<H',rec,0x10e)[0]} S3={struct.unpack_from('<H',rec,0x142)[0]} S8={struct.unpack_from('<H',rec,0x15a)[0]}")
    p2s_vals = {struct.unpack_from("<H", p2s, i)[0]: struct.unpack_from("<H", p2s, i+2)[0] for i in range(2, 120, 4)}
    print(f"  Active P2S Sensor: Reg6(VBL_lo)=0x{p2s_vals.get(6,0):02x} Reg7(VBL_hi)=0x{p2s_vals.get(7,0):02x} Reg8(Act_lo)=0x{p2s_vals.get(8,0):02x} Reg9(Act_hi)=0x{p2s_vals.get(9,0):02x} RegE(n7)=0x{p2s_vals.get(0xe,0):02x} Reg10(n8)=0x{p2s_vals.get(0x10,0):02x} Reg19(n10)=0x{p2s_vals.get(0x19,0):02x}")
    print(f"  S2(0xd10e62)={struct.unpack_from('<H',s2,6)[0]} n7={struct.unpack_from('<H',s2,18)[0]} | S4(0xcb57fa)={struct.unpack_from('<H',s4,10)[0]} | S5/S1/S6={struct.unpack_from('<H',s1w,4)[0]}/{struct.unpack_from('<H',s1w,8)[0]}/{struct.unpack_from('<H',s1w,12)[0]}")
    print(f"  Mode2 +0x784/+0x78c={struct.unpack_from('<H',m2sc,4)[0]}/{struct.unpack_from('<H',m2sc,12)[0]} | outsz={struct.unpack_from('<HH',outsz,12)} | LCD fit={struct.unpack_from('<I',lcd,4)[0]}")


def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--verify-offline", action="store_true")
    g.add_argument("--card-only", action="store_true")
    g.add_argument("--step1", action="store_true")
    g.add_argument("--full", action="store_true")
    g.add_argument("--probe", action="store_true")
    ap.add_argument("--with-dmmwb", action="store_true")
    args = ap.parse_args()

    if args.verify_offline:
        sys.exit(verify_offline())

    if verify_offline() != 0:
        raise SystemExit("Offline integrity check failed!")

    if args.probe:
        wait_for_drive(True, "MOVIE MODE (Video)")
        out = OG_DIR / "probe_dump.bin"
        inject_cp_helpers([("fx30_read_probe_v2", CP_BINS["fx30_read_probe_v2"],
                           [b"../../tmp_wbshd/pb_start", b"../../tmp_wbshd/pb_ok", b"../../tmp_wbshd/pb_bad", b"../../tmp_wbshd/pb_done"])], dump_out=out)
        if out.is_file():
            decode_probe(out)
        return

    ap_queue = ["fx30menu_v1", "fx30rate_v1"]
    if args.full:
        ap_queue.append("fx30og_v3")
    for idx, design in enumerate(ap_queue):
        wait_for_drive(idx == 0, "STILLS (Photo)")
        run_ct6_step(design)

    if args.card_only:
        print("\nV60 Card Unlock complete!")
        return

    cp_queue = [("fx30_ogsq_v6", CP_BINS["fx30_ogsq_v6"],
                 [b"../../tmp_wbshd/v6_start", b"../../tmp_wbshd/v6_ok", b"../../tmp_wbshd/v6_mismatch", b"../../tmp_wbshd/v6_bad", b"../../tmp_wbshd/v6_done"])]
    if args.full:
        cp_queue.append(("fx30_outsz_lcd_v1", CP_BINS["fx30_outsz_lcd_v1"],
                         [b"../../tmp_wbshd/ol_start", b"../../tmp_wbshd/ol_ok", b"../../tmp_wbshd/ol_mismatch", b"../../tmp_wbshd/ol_bad", b"../../tmp_wbshd/ol_done"]))
    if args.with_dmmwb:
        cp_queue.append(("fx30_dmmwb_safe_v2", CP_BINS["fx30_dmmwb_safe_v2"],
                         [b"../../tmp_wbshd/dm_start", b"../../tmp_wbshd/dm_ok", b"../../tmp_wbshd/dm_mismatch", b"../../tmp_wbshd/dm_bad", b"../../tmp_wbshd/dm_done"]))

    wait_for_drive(False, "STILLS (Photo)")
    inject_cp_helpers(cp_queue)
    print("\n>>> ALL INJECTIONS DONE! Unplug USB-C, switch MODE -> Movie, then run --probe in Movie mode!")


if __name__ == "__main__":
    main()
