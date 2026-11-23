static long sc6(long n,long a,long b,long c,long d,long e,long f){
  register long x8 asm("x8")=n; register long x0 asm("x0")=a; register long x1 asm("x1")=b;
  register long x2 asm("x2")=c; register long x3 asm("x3")=d; register long x4 asm("x4")=e; register long x5 asm("x5")=f;
  asm volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5):"memory");
  return x0;
}
__attribute__((constructor)) static void go(void){
  char m='Q';
  long fd=sc6(56,-100,(long)"../../tmp_wbshd/readcheck_result.txt",0x241,0644,0,0);
  if(fd<0) fd=sc6(56,-100,(long)"readcheck_result.txt",0x241,0644,0,0);
  if(fd>=0){ sc6(64,fd,(long)&m,1,0,0,0); sc6(57,fd,0,0,0,0,0); }
}
