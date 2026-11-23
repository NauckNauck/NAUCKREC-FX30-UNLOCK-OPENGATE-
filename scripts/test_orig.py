import sys, struct, time
from pathlib import Path
BASE = Path(__file__).resolve().parent
CT = BASE.parent / "a7iv-open-gate-test" / "windows" / "tools" / "camera_tests"
sys.path.insert(0, str(CT / "CT1_usb_first_contact"))
sys.path.insert(0, str(CT / "CT6_memory_read_check"))
import ct1_common as c
import ct1_stage2_service as service
SET_FILE_PATH, GET_FILE2, SET_FILE2, FILE_DELETE2 = 0x400F, 0x4011, 0x4010, 0x4012
LIB_PATH = b"/tmp/jiritsu/libJit.so"
RESULT = b"../../tmp_wbshd/readcheck_result.txt"
ORIG = CT / "CT6_memory_read_check" / "helper" / "libJit_level13_dmmwb4688_v1.so"
RULES = {
 (c.PFUNC_ADJUST,0x1001,SET_FILE_PATH):("SET_FILE_PATH",lambda p:p[:1]==b"\x00" and p[1:] in (LIB_PATH,RESULT)),
 (c.PFUNC_ADJUST,0x1001,GET_FILE2):("GET_FILE2",lambda p:p==b"\x00\xff\x00"),
 (c.PFUNC_ADJUST,0x1001,SET_FILE2):("SET_FILE2",lambda p:p[:3]==b"\x00\xff\x00"),
 (c.PFUNC_ADJUST,0x1001,FILE_DELETE2):("FILE_DELETE2",lambda p:p==b"\x00\xff"),
}
class G(c.ServiceGuard):
 def _send(self,pf,data,_o):
  d=self._device; h=struct.pack(c.SENSER_HEADER,len(data),pf,d._sequence,0,0,0,0)
  d.driver.write(h+bytes(data)); r,o=c.read_service_answer(d.driver,d._sequence,self.session.log); d._sequence+=1; return r,o
def command(cam,code,payload):
 a=cam._sendAdjustControlPacket(0x1001,code,payload)
 return (struct.unpack_from("<I",a)[0],a[4:]) if len(a)>=4 else (None,b"")
def select(cam,p): return command(cam,SET_FILE_PATH,b"\x00"+p)
def read_file(cam,p):
 select(cam,p); st,d=command(cam,GET_FILE2,b"\x00\xff\x00"); return (st==0),d
def body(cam):
 # clear
 select(cam,RESULT); command(cam,FILE_DELETE2,b"\x00\xff")
 t,_=read_file(cam,RESULT); print("cleared, present:",t)
 # send original
 lib=ORIG.read_bytes()
 select(cam,LIB_PATH); command(cam,FILE_DELETE2,b"\x00\xff"); select(cam,LIB_PATH)
 st,_=command(cam,SET_FILE2,b"\x00\xff\x00"+lib); print("orig write/load:",hex(st) if st else st)
 time.sleep(3)
 t,d=read_file(cam,RESULT); print("orig result after clear: present=%s bytes=%d"%(t,len(d)))
 if t and d: print("first bytes:",d[:40])
def main():
 session=c.Session("fx30_test",test="FX30TEST")
 try:
  inner=service.connect(session)
  from pmca.usb.sony import SonySenserAuthDevice,SonySenserCamera,SonySenserDevice
  driver=c.GuardedDriver(inner,session,"senser"); auth=SonySenserAuthDevice(driver); auth.start()
  try:
   auth.authenticate(); dev=SonySenserDevice(driver); G(dev,session,RULES); body(SonySenserCamera(dev))
  finally:
   auth.stop(); print("Stop sent")
  session.finish("completed")
 except BaseException as e:
  session.finish("failed","%s: %s"%(type(e).__name__,e)); raise
if __name__=="__main__": main()
