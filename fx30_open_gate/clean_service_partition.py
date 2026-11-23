#!/usr/bin/env python3
"""Clean leftover libJit.so or result files from service partition."""
import sys, struct
from pathlib import Path

CT = Path("/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/tools/camera_tests")
sys.path.insert(0, str(CT / "CT1_usb_first_contact"))
sys.path.insert(0, str(CT / "CT6_memory_read_check"))

import ct1_common as c
import ct1_stage2_service as service

SET_FILE_PATH, FILE_DELETE2 = 0x400F, 0x4012
LIB_PATH = b"/tmp/jiritsu/libJit.so"

RULES = {
    (c.PFUNC_ADJUST, 0x1001, SET_FILE_PATH): ("SET_FILE_PATH", lambda p: True),
    (c.PFUNC_ADJUST, 0x1001, FILE_DELETE2): ("FILE_DELETE2", lambda p: True),
}

class CleanGuard(c.ServiceGuard):
    def _send(self, pFunc, data, _oData):
        device = self._device
        header = struct.pack(c.SENSER_HEADER, len(data), pFunc, device._sequence, 0, 0, 0, 0)
        device.driver.write(header + bytes(data))
        response, out = c.read_service_answer(device.driver, device._sequence, self.session.log)
        device._sequence += 1
        return response, out

def command(camera, code, payload):
    answer = camera._sendAdjustControlPacket(0x1001, code, payload)
    if len(answer) < 4: return None, b""
    return struct.unpack_from("<I", answer)[0], answer[4:]

def main():
    session = c.Session("clean_partition", test="CLEAN")
    try:
        inner = service.connect(session)
        from pmca.usb.sony import SonySenserAuthDevice, SonySenserCamera, SonySenserDevice
        driver = c.GuardedDriver(inner, session, "senser")
        auth = SonySenserAuthDevice(driver)
        auth.start()
        try:
            auth.authenticate()
            device = SonySenserDevice(driver)
            CleanGuard(device, session, RULES)
            camera = SonySenserCamera(device)
            # Delete libJit.so
            camera._sendAdjustControlPacket(0x1001, SET_FILE_PATH, b"\x00" + LIB_PATH)
            st, _ = command(camera, FILE_DELETE2, b"\x00\xff")
            print("Deleted libJit.so from service partition: status %s" % hex(st) if st else "OK")
        finally:
            auth.stop()
        session.finish("completed")
    except BaseException as exc:
        session.finish("failed", "%s: %s" % (type(exc).__name__, exc))
        raise

if __name__ == "__main__":
    main()
