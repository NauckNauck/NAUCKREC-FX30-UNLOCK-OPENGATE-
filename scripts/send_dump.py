#!/usr/bin/env python3
"""Send the memory-dump helper to the Sony FX30 and pull back the CP dump.

Reuses the CT1/CT6 connection flow (MSC -> service mode -> Jiritsu). The helper
is written to /tmp/jiritsu/libJit.so (the only path the camera will load), it
dumps /dev/mem[0x01000000..0x02000000) to /tmp/wbshd/dump.bin, and this script
reads it back.

    python3 send_dump.py
"""
import sys, os, struct, time, argparse
from pathlib import Path

BASE = Path(__file__).resolve().parent
CT = BASE.parent / "a7iv-open-gate-test" / "windows" / "tools" / "camera_tests"
sys.path.insert(0, str(CT / "CT1_usb_first_contact"))
sys.path.insert(0, str(CT / "CT6_memory_read_check"))

import ct1_common as c
import ct1_stage2_service as service

SET_FILE_PATH, GET_FILE2, SET_FILE2, FILE_DELETE2 = 0x400F, 0x4011, 0x4010, 0x4012
ST_OK = 0
STATUS_TEXT = {0: "ok", 0x10000006: "no such file", 0x10000000: "written but load failed",
               0x10000011: "busy", 0x1000000B: "remove failed", 0x1000000F: "service partition full"}

LIB_PATH = b"/tmp/jiritsu/libJit.so"
DUMP_PATH = b"../../tmp_wbshd/dump.bin"
REL_MK = b"../../tmp_wbshd/mk_rel"
ABS_MK = b"/tmp/wbshd/mk_abs"
ROOT_MK = b"/tmp/mk_root"
EMS = [b"../../tmp_wbshd/dci3_start", b"../../tmp_wbshd/dci3_ok", b"../../tmp_wbshd/dci3_bad",
       b"../../tmp_wbshd/dci3_mis", b"../../tmp_wbshd/dci3_ipmis", b"../../tmp_wbshd/dci3_done"]
LIB = Path(os.environ.get("FX30_HELPER", str(BASE.parent / "fx30_open_gate" / "sources" / "dci_native.c")))
if LIB.suffix != ".so":
    LIB = LIB.with_suffix(".so")  # build the .c first with scripts/build_fx30_helpers.py
KEEP = "--keep" in sys.argv
OUT = BASE / "fx30_cp_dump.bin"

# allow-list: SET_FILE_PATH only to the two paths we use, SET_FILE2 any 0x00ff00 body
RULES = {
    (c.PFUNC_ADJUST, 0x1001, SET_FILE_PATH): ("SET_FILE_PATH", lambda p: p[:1] == b"\x00" and p[1:] in (LIB_PATH, DUMP_PATH, REL_MK, ABS_MK, ROOT_MK, *EMS)),
    (c.PFUNC_ADJUST, 0x1001, GET_FILE2): ("GET_FILE2", lambda p: p == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, SET_FILE2): ("SET_FILE2", lambda p: p[:3] == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, FILE_DELETE2): ("FILE_DELETE2", lambda p: p == b"\x00\xff"),
}


class DumpGuard(c.ServiceGuard):
    """CT1's guard without the 0x1000-byte cap: this run writes one 13.8 KB helper."""

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
    lib = LIB.read_bytes()
    print("helper: %d bytes" % len(lib))
    select(camera, LIB_PATH)
    command(camera, FILE_DELETE2, b"\x00\xff")
    select(camera, LIB_PATH)
    print("writing helper to %s" % LIB_PATH.decode())
    status, _ = command(camera, SET_FILE2, b"\x00\xff\x00" + lib)
    print("write/load status: %s" % STATUS_TEXT.get(status, hex(status) if status else status))
    time.sleep(4)
    for mk in EMS:
        t, md = read_file(camera, mk)
        print("marker %s: present=%s content=%r" % (mk.decode(), t, md[:8]))
    there, data = read_file(camera, DUMP_PATH)
    print("dump %s: present=%s bytes=%d" % (DUMP_PATH.decode(), there, len(data)))
    if there and data:
        OUT.write_bytes(data)
        print("saved %s" % OUT)
    if not KEEP:
        select(camera, LIB_PATH)
        st, _ = command(camera, FILE_DELETE2, b"\x00\xff")
        print("helper removed from service partition (status %s) -> encoder step can run" % st)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--no-stop", action="store_true", help="leave the camera in service mode")
    args = ap.parse_args()
    session = c.Session("fx30_dump", test="FX30DUMP")
    try:
        inner = service.connect(session)
        from pmca.usb.sony import SonySenserAuthDevice, SonySenserCamera, SonySenserDevice
        driver = c.GuardedDriver(inner, session, "senser")
        auth = SonySenserAuthDevice(driver)
        auth.start()
        try:
            auth.authenticate()
            device = SonySenserDevice(driver)
            DumpGuard(device, session, RULES)
            body(SonySenserCamera(device))
        finally:
            if args.no_stop:
                print("Stop request NOT sent (camera stays in service mode)")
            else:
                auth.stop()
                print("Stop request sent")
        session.finish("completed")
    except BaseException as exc:
        session.finish("failed", "%s: %s" % (type(exc).__name__, exc))
        raise


if __name__ == "__main__":
    main()
