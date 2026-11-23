#!/usr/bin/env python3
"""Diagnostic: send the ORIGINAL (unmodified) dmmwb helper and see whether its
constructor runs (its readcheck_result.txt appears). Then send our dumper and
read the dump. One auth for both."""
import sys, struct, time
from pathlib import Path

BASE = Path(__file__).resolve().parent
CT = BASE.parent / "a7iv-open-gate-test" / "windows" / "tools" / "camera_tests"
sys.path.insert(0, str(CT / "CT1_usb_first_contact"))
sys.path.insert(0, str(CT / "CT6_memory_read_check"))

import ct1_common as c
import ct1_stage2_service as service

SET_FILE_PATH, GET_FILE2, SET_FILE2, FILE_DELETE2 = 0x400F, 0x4011, 0x4010, 0x4012
ST_OK = 0
LIB_PATH = b"/tmp/jiritsu/libJit.so"
RESULT = b"../../tmp_wbshd/readcheck_result.txt"
DUMP = b"../../tmp_wbshd/dump.bin"
ROOT_MK = b"/tmp/mk_root"
ORIG = CT / "CT6_memory_read_check" / "helper" / "libJit_level13_dmmwb4688_v1.so"
MINE = BASE / "stub_lib.so"

RULES = {
    (c.PFUNC_ADJUST, 0x1001, SET_FILE_PATH): ("SET_FILE_PATH", lambda p: p[:1] == b"\x00" and p[1:] in (LIB_PATH, RESULT, DUMP, ROOT_MK)),
    (c.PFUNC_ADJUST, 0x1001, GET_FILE2): ("GET_FILE2", lambda p: p == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, SET_FILE2): ("SET_FILE2", lambda p: p[:3] == b"\x00\xff\x00"),
    (c.PFUNC_ADJUST, 0x1001, FILE_DELETE2): ("FILE_DELETE2", lambda p: p == b"\x00\xff"),
}


class G(c.ServiceGuard):
    def _send(self, pFunc, data, _oData):
        d = self._device
        h = struct.pack(c.SENSER_HEADER, len(data), pFunc, d._sequence, 0, 0, 0, 0)
        d.driver.write(h + bytes(data))
        r, o = c.read_service_answer(d.driver, d._sequence, self.session.log)
        d._sequence += 1
        return r, o


def command(cam, code, payload):
    a = cam._sendAdjustControlPacket(0x1001, code, payload)
    return (struct.unpack_from("<I", a)[0], a[4:]) if len(a) >= 4 else (None, b"")


def select(cam, path):
    return command(cam, SET_FILE_PATH, b"\x00" + path)


def read_file(cam, path):
    select(cam, path)
    st, data = command(cam, GET_FILE2, b"\x00\xff\x00")
    return (st == ST_OK), data


def send_lib(cam, lib):
    select(cam, LIB_PATH)
    command(cam, FILE_DELETE2, b"\x00\xff")
    select(cam, LIB_PATH)
    st, _ = command(cam, SET_FILE2, b"\x00\xff\x00" + lib)
    return st


def body(cam):
    lib = ORIG.read_bytes()
    print("== original helper, %d bytes ==" % len(lib))
    st = send_lib(cam, lib)
    print("write/load status:", hex(st) if st else st)
    time.sleep(3)
    there, data = read_file(cam, RESULT)
    print("original helper result file: present=%s bytes=%d" % (there, len(data)))
    if there and data:
        (BASE / "original_helper_result.txt").write_bytes(data)
        print("saved original_helper_result.txt")
    else:
        print("=> the original helper's constructor did NOT write its result on the FX30")

    # clear the stale result first
    select(cam, RESULT)
    command(cam, FILE_DELETE2, b"\x00\xff")
    t0, _ = read_file(cam, RESULT)
    print("after delete, result present:", t0)
    mine = MINE.read_bytes()
    print("== stub, %d bytes ==" % len(mine))
    st = send_lib(cam, mine)
    print("write/load status:", hex(st) if st else st)
    time.sleep(4)
    t, mk = read_file(cam, RESULT)
    print("stub result: present=%s content=%r" % (t, mk[:8]))
    t, d = read_file(cam, DUMP)
    print("dump: present=%s bytes=%d" % (t, len(d)))
    if t and d:
        (BASE / "fx30_cp_dump.bin").write_bytes(d)
        print("saved fx30_cp_dump.bin")


def main():
    session = c.Session("fx30_diag", test="FX30DIAG")
    try:
        inner = service.connect(session)
        from pmca.usb.sony import SonySenserAuthDevice, SonySenserCamera, SonySenserDevice
        driver = c.GuardedDriver(inner, session, "senser")
        auth = SonySenserAuthDevice(driver)
        auth.start()
        try:
            auth.authenticate()
            device = SonySenserDevice(driver)
            G(device, session, RULES)
            body(SonySenserCamera(device))
        finally:
            auth.stop()
            print("Stop request sent")
        session.finish("completed")
    except BaseException as exc:
        session.finish("failed", "%s: %s" % (type(exc).__name__, exc))
        raise


if __name__ == "__main__":
    main()
