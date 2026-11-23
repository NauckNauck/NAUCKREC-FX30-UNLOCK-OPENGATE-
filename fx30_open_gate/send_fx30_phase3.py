#!/usr/bin/env python3
"""Send the Unified FX30 Open Gate Phase 3 helper over USB Service Mode.
RAM-only, volatile injection. Battery pull completely restores camera to stock.
"""
import sys, struct, time, argparse
from pathlib import Path

BASE = Path("/Users/nauck/Documents/sony_research/fx30_open_gate")
CT = Path("/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/tools/camera_tests")
sys.path.insert(0, str(CT / "CT1_usb_first_contact"))
sys.path.insert(0, str(CT / "CT6_memory_read_check"))

import ct1_common as c
import ct1_stage2_service as service

SET_FILE_PATH, GET_FILE2, SET_FILE2, FILE_DELETE2 = 0x400F, 0x4011, 0x4010, 0x4012
ST_OK = 0
STATUS_TEXT = {0: "ok", 0x10000006: "no such file", 0x10000000: "written but load failed",
               0x10000011: "busy", 0x1000000B: "remove failed", 0x1000000F: "service partition full"}

LIB_PATH = b"/tmp/jiritsu/libJit.so"
MARKERS = [
    b"../../tmp_wbshd/og3_start",
    b"../../tmp_wbshd/og3_s0_ok",
    b"../../tmp_wbshd/og3_s3_ok",
    b"../../tmp_wbshd/og3_s1_ok",
    b"../../tmp_wbshd/og3_s2s4_ok",
    b"../../tmp_wbshd/og3_records_ok",
    b"../../tmp_wbshd/og3_records_fail",
    b"../../tmp_wbshd/og3_outsz_ok",
    b"../../tmp_wbshd/og3_dmm_ok",
    b"../../tmp_wbshd/og3_lcd_ok",
    b"../../tmp_wbshd/og3_done",
    b"../../tmp_wbshd/og3_err_alloc",
    b"../../tmp_wbshd/og3_err_phys",
    b"../../tmp_wbshd/og3_err_mem",
    b"../../tmp_wbshd/og3_err_mmap",
]
LIB = BASE / "fx30_og_phase3.so"

RULES = {
    (c.PFUNC_ADJUST, 0x1001, SET_FILE_PATH): ("SET_FILE_PATH", lambda p: p[:1] == b"\x00" and p[1:] in (LIB_PATH, *MARKERS)),
    (c.PFUNC_ADJUST, 0x1001, GET_FILE2): ("GET_FILE2", lambda p: p == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, SET_FILE2): ("SET_FILE2", lambda p: p[:3] == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, FILE_DELETE2): ("FILE_DELETE2", lambda p: p == b"\x00\xff"),
}


class OGGuard(c.ServiceGuard):
    def _send(self, pFunc, data, _oData):
        device = self._device
        header = struct.pack(c.SENSER_HEADER, len(data), pFunc, device._sequence, 0, 0, 0, 0)
        device.driver.write(header + bytes(data))
        response, out = c.read_service_answer(device.driver, device._sequence, self.session.log)
        device._sequence += 1
        return response, out


def command(camera, code, payload):
    answer = camera._sendAdjustControlPacket(0x1001, code, payload)
    if len(answer) < 4:
        return None, b""
    return struct.unpack_from("<I", answer)[0], answer[4:]


def select(camera, path):
    status, _ = command(camera, SET_FILE_PATH, b"\x00" + path)
    return status


def read_file(camera, path):
    select(camera, path)
    status, data = command(camera, GET_FILE2, b"\x00\xff\x00")
    return (status == ST_OK), data


def body(camera):
    # 1. Clear old markers
    for mk in MARKERS:
        select(camera, mk)
        command(camera, FILE_DELETE2, b"\x00\xff")

    # 2. Upload and execute helper
    lib = LIB.read_bytes()
    print("FX30 Open Gate Phase 3 Helper: %d bytes" % len(lib))
    select(camera, LIB_PATH)
    command(camera, FILE_DELETE2, b"\x00\xff")
    select(camera, LIB_PATH)
    print("Writing helper to %s..." % LIB_PATH.decode())
    status, _ = command(camera, SET_FILE2, b"\x00\xff\x00" + lib)
    print("Execution status: %s" % STATUS_TEXT.get(status, hex(status) if status else status))
    time.sleep(3)

    # 3. Read execution markers
    print("\n--- Marker Verification ---")
    results = {}
    for mk in MARKERS:
        t, md = read_file(camera, mk)
        name = mk.split(b"/")[-1].decode()
        if t:
            results[name] = md.decode("latin1", errors="replace").strip()
            print("  [+] %s: %s" % (name, results[name]))

    expected = ["og3_start", "og3_s0_ok", "og3_s3_ok", "og3_s1_ok", "og3_s2s4_ok",
                "og3_records_ok", "og3_outsz_ok", "og3_dmm_ok", "og3_lcd_ok", "og3_done"]
    all_ok = all(k in results for k in expected)

    if all_ok:
        print("\n🎉 SUCCESS: All Phase 3 Open Gate modifications applied and verified in RAM!")
        print("  - Mode 6 on Column 1 + Continuous streaming (0x0F)")
        print("  - S3 vertical readout timing = 2020 (line periods unlocked)")
        print("  - S1 stream-0 raster active height = 3968")
        print("  - S7, S2, S4 crop & geometry offsets = 0")
        print("  - Mode 3 Left & Right 3000-byte records (180 fields ported)")
        print("  - ISP raster outsz = 3840 x 2560")
        print("  - DMM buffer pool = 29.29 MB")
        print("  - Display driver = 3:2 preview")
    else:
        print("\n⚠️ WARNING: Some markers were not found. Review output above.")


def main():
    session = c.Session("fx30_open_gate_phase3", test="FX30OG3")
    try:
        inner = service.connect(session)
        from pmca.usb.sony import SonySenserAuthDevice, SonySenserCamera, SonySenserDevice
        driver = c.GuardedDriver(inner, session, "senser")
        auth = SonySenserAuthDevice(driver)
        auth.start()
        try:
            auth.authenticate()
            device = SonySenserDevice(driver)
            OGGuard(device, session, RULES)
            body(SonySenserCamera(device))
        finally:
            auth.stop()
            print("Service session ended safely.")
        session.finish("completed")
    except BaseException as exc:
        session.finish("failed", "%s: %s" % (type(exc).__name__, exc))
        raise


if __name__ == "__main__":
    main()
