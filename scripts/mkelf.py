#!/usr/bin/env python3
"""Build a minimal aarch64-linux-gnu ELF shared object from a clang .o.

The Mac has no ELF linker (only ld64), so this applies the handful of
PC-relative relocations clang emits for a self-contained -nostdlib object and
writes a minimal ET_DYN with DT_INIT / .init_array.
"""
import struct, sys
from pathlib import Path

OBJ = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("dump.o")
OUT = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("dump.so")

o = OBJ.read_bytes()

def u16(b, off): return struct.unpack_from("<H", b, off)[0]
def u32(b, off): return struct.unpack_from("<I", b, off)[0]
def u64(b, off): return struct.unpack_from("<Q", b, off)[0]

# ---- parse the relocatable object -------------------------------------------
e_shoff = u64(o, 0x28); e_shentsize = u16(o, 0x3a); e_shnum = u16(o, 0x3c)
e_shstrndx = u16(o, 0x3e)

sections = []
for i in range(e_shnum):
    off = e_shoff + i * e_shentsize
    name = u32(o, off)
    typ = u32(o, off + 4)
    flags = u64(o, off + 8)
    addr = u64(o, off + 16)
    soff = u64(o, off + 24)
    size = u64(o, off + 32)
    link = u32(o, off + 40)
    info = u32(o, off + 44)
    addralign = u64(o, off + 48)
    entsize = u64(o, off + 56)
    sections.append(dict(name=name, type=typ, flags=flags, addr=addr, off=soff,
                         size=size, link=link, info=info, align=addralign, entsize=entsize))

shstr = o[sections[e_shstrndx]["off"]:sections[e_shstrndx]["off"] + sections[e_shstrndx]["size"]]
def shname(idx):
    n = sections[idx]["name"]
    end = shstr.find(b"\0", n)
    return shstr[n:end].decode()

by_name = {shname(i): i for i in range(e_shnum)}
print("sections:", {k: (hex(sections[v]["size"]), sections[v]["type"]) for k, v in by_name.items()})

text = bytearray(o[sections[by_name[".text"]]["off"]:sections[by_name[".text"]]["off"] + sections[by_name[".text"]]["size"]])
ro   = bytearray(o[sections[by_name[".rodata.str1.1"]]["off"]:sections[by_name[".rodata.str1.1"]]["off"] + sections[by_name[".rodata.str1.1"]]["size"]])

# ---- final layout ------------------------------------------------------------
TEXT_VA = 0x1000
RO_VA   = 0x2000
DYN_VA  = 0x3000
DYNSTR_VA = 0x3100
DYNSYM_VA = 0x3200
HASH_VA   = 0x3300
RELA_VA   = 0x3400

def patch_adrp(code, insn_off, target_va, insn_va):
    page_delta = ((target_va & ~0xfff) - (insn_va & ~0xfff)) >> 12
    immlo = page_delta & 3
    immhi = (page_delta >> 2) & 0x7ffff
    rd = u32(code, insn_off) & 0x1f
    insn = 0x90000000 | (immlo << 29) | (immhi << 5) | rd
    struct.pack_into("<I", code, insn_off, insn)

def patch_add_lo12(code, insn_off, target_va):
    imm = target_va & 0xfff
    old = u32(code, insn_off)
    rd = old & 0x1f
    rn = (old >> 5) & 0x1f
    insn = 0x91000000 | (imm << 10) | (rn << 5) | rd
    struct.pack_into("<I", code, insn_off, insn)

# relocations recorded by clang: .text+4 ADRP / .text+8 ADD -> rodata+0
#                              .text+0x80 ADRP / .text+0x84 ADD -> rodata+9
patch_adrp(text, 0x0c, RO_VA + 0x0, TEXT_VA + 0x0c); patch_add_lo12(text, 0x10, RO_VA + 0x0)
patch_adrp(text, 0x38, RO_VA + 0x9, TEXT_VA + 0x38); patch_add_lo12(text, 0x3c, RO_VA + 0x9)

# ---- dynamic section ---------------------------------------------------------
# .init_array = [ init_fn ] with a RELATIVE reloc; plus DT_INIT for good measure
INIT_FN = TEXT_VA
RELA = struct.pack("<QQq", 0, 0x8, 0)  # r_offset=0 (filled below), r_info=R_RELATIVE|sym=0, addend=INIT_FN
# r_info for R_AARCH64_RELATIVE (1027 << 32)
rela_r_info = (1027 << 32)
rela = struct.pack("<QQq", DYN_VA, rela_r_info, INIT_FN)   # patch .init_array[0] at DYN_VA

DYNSTR = b"\0libsencore.so\0libJit.so\0"
dyn = [
    (1,  1),                # DT_NEEDED   "libsencore.so"
    (14, 15),               # DT_SONAME   "libJit.so"
    (12, INIT_FN),          # DT_INIT
    (25, DYN_VA),           # DT_INIT_ARRAY
    (27, 8),                # DT_INIT_ARRAYSZ
    (5,  DYNSTR_VA),        # DT_STRTAB
    (6,  DYNSYM_VA),        # DT_SYMTAB
    (10, len(DYNSTR)),      # DT_STRSZ
    (11, 24),               # DT_SYMENT
    (4,  HASH_VA),          # DT_HASH
    (17, RELA_VA),          # DT_RELA
    (18, 24),               # DT_RELASZ
    (19, 24),               # DT_RELAENT
    (0, 0),                 # DT_NULL
]
dyn_bytes = b"".join(struct.pack("<qQ", t, v) for t, v in dyn)
INIT_ARRAY = struct.pack("<Q", INIT_FN)
dynstr = DYNSTR
dynsym = b"\0" * 24
hash_tab = struct.pack("<IIII", 1, 1, 0, 0)  # nbucket, nchain, bucket[0]=0, chain[0]=0

# ---- assemble the image ------------------------------------------------------
img = bytearray(0x3600)
def put(va, data):
    img[va:va + len(data)] = data
put(TEXT_VA, text)
put(RO_VA, ro)
put(DYN_VA, INIT_ARRAY + dyn_bytes)   # init_array first 8 bytes, then dynamic
put(DYNSTR_VA, dynstr)
put(DYNSYM_VA, dynsym)
put(HASH_VA, hash_tab)
put(RELA_VA, rela)

# ---- ELF header --------------------------------------------------------------
e_ident = b"\x7fELF" + bytes([2, 1, 1, 0]) + b"\0" * 8
ehdr = e_ident + struct.pack("<HHIQQQIHHHHHH",
    3,        # ET_DYN
    183,      # EM_AARCH64
    1,        # EV_CURRENT
    0,        # e_entry
    0x40,     # e_phoff
    0,        # e_shoff
    0,        # e_flags
    64,       # e_ehsize
    56,       # e_phentsize
    4,        # e_phnum
    64,       # e_shentsize
    0,        # e_shnum
    0)        # e_shstrndx

phdrs = []
def ph(p_type, p_flags, p_offset, p_vaddr, p_filesz, p_memsz, p_align=0x1000):
    phdrs.append(struct.pack("<IIQQQQQQ", p_type, p_flags, p_offset, p_vaddr, p_vaddr, p_filesz, p_memsz, p_align))

# PT_LOAD RX: header..rodata
ph(1, 5, 0, 0, DYN_VA, DYN_VA)
# PT_LOAD RW: dynamic tables
ph(1, 6, DYN_VA, DYN_VA, RELA_VA + 24 - DYN_VA, RELA_VA + 24 - DYN_VA)
# PT_DYNAMIC
ph(2, 6, DYN_VA + 8, DYN_VA + 8, len(dyn_bytes), len(dyn_bytes))
# PT_GNU_STACK (RW, no exec)
ph(0x6474e551, 6, 0, 0, 0, 0, 0x10)

blob = bytearray(img)
blob[0:len(ehdr)] = ehdr
ph_off = 0x40
for p in phdrs:
    blob[ph_off:ph_off + len(p)] = p
    ph_off += len(p)

OUT.write_bytes(bytes(blob))
print(f"wrote {OUT} ({len(blob)} bytes)")
