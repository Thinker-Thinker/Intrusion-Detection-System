"""Run the STM32 build of the face pipeline (build/emu.elf) in the Unicorn ARM emulator (Cortex-M7)
on real frames and compare with the PC simulation (models/pcsim pipe). This checks the code the
board will actually execute: arm-none-eabi-gcc output, real DSP/SIMD instructions, memory layout.

usage: python run_emu.py <emu.elf> <pipe_pc exe> image1.png [image2.png ...]
"""
import sys, struct, subprocess, numpy as np, cv2
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_HOOK_INTR, UcError
from unicorn.arm_const import *

REGIONS = [(0x08000000, 0x200000), (0x20000000, 0x20000), (0x24000000, 0x50000), (0x30000000, 0x8000),
           (0x38000000, 0x4000), (0x90000000, 0x800000), (0xE0000000, 0x100000)]


def to565(bgr):
    r, g, b = [np.round(bgr[..., i].astype(np.float32) * k / 255).astype(np.uint16) for i, k in ((2, 31), (1, 63), (0, 31))]
    return ((r << 11) | (g << 5) | b).astype(">u2").tobytes()


def load(elf_path):
    elf = ELFFile(open(elf_path, "rb"))
    uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
    try:
        uc.ctl_set_cpu_model(UC_CPU_ARM_CORTEX_M7)
    except Exception:
        pass
    for a, n in REGIONS:
        uc.mem_map(a, n)
    for s in elf.iter_sections():
        if s["sh_flags"] & 2 and s["sh_type"] == "SHT_PROGBITS" and s["sh_size"]:
            uc.mem_write(s["sh_addr"], s.data())                       # .data goes straight to its RAM address
    sym = {x.name: x["st_value"] for x in elf.get_section_by_name(".symtab").iter_symbols()}
    return uc, sym


def run(uc, sym, frame_bytes):
    uc.mem_write(sym["axi_pool"], frame_bytes)
    uc.reg_write(UC_ARM_REG_SP, 0x20020000)
    try:  # enable the FPU (CPACR full access to CP10/CP11)
        uc.reg_write(UC_ARM_REG_CPACR, 0xF << 20)
    except Exception:
        pass
    try:
        uc.reg_write(UC_ARM_REG_FPEXC, 0x40000000)
    except Exception:
        pass
    stop = {}
    def intr(u, no, _):
        stop["pc"] = u.reg_read(UC_ARM_REG_PC); u.emu_stop()
    h = uc.hook_add(UC_HOOK_INTR, intr)
    try:
        uc.emu_start(sym["emu_run"] | 1, 0, count=3_000_000_000)
    except UcError as e:
        raise SystemExit(f"emulator error {e} at pc=0x{uc.reg_read(UC_ARM_REG_PC):08x}")
    uc.hook_del(h)
    n = 1 + 1 + 4 + 10 + 1 + 512
    return np.frombuffer(bytes(uc.mem_read(sym["emu_out"], 4 * n)), np.float32).copy()


def main():
    elf, pipe, imgs = sys.argv[1], sys.argv[2], sys.argv[3:]
    uc, sym = load(elf)
    frames = [to565(cv2.imread(f)) for f in imgs]
    out = subprocess.run([pipe], input=b"".join(frames), capture_output=True).stdout
    REC = 1 + 4 + 10 + 1 + 512; o = 0
    for f, fb in zip(imgs, frames):
        n_pc = struct.unpack("<i", out[o:o + 4])[0]; pc = np.frombuffer(out[o + 4:o + 4 + 4 * REC], np.float32); o += 4 + 4 * REC
        em = run(uc, sym, fb)
        n_em = int(em[0])
        msg = f"{f.split('/')[-1]}: faces pc={n_pc} arm={n_em}"
        if n_pc and n_em:
            box_d = np.abs(pc[1:5] - em[2:6]).max()
            cos = float(pc[16:] @ em[17:] / (np.linalg.norm(pc[16:]) * np.linalg.norm(em[17:]) + 1e-9))
            msg += f" | box diff {box_d:.3f}px | spoof pc={pc[15]:.3f} arm={em[16]:.3f} | embedding cos {cos:.5f}"
        print(msg, flush=True)


if __name__ == "__main__":
    main()
