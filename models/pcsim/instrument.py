"""Copy a TinyEngine gen dir and make invoke() dump every layer's output to stderr (for layer-by-layer checks)."""
import re, shutil, sys
src_dir, dst_dir = sys.argv[1], sys.argv[2]
shutil.rmtree(dst_dir, ignore_errors=True); shutil.copytree(src_dir, dst_dir)
p = f"{dst_dir}/codegen/Source/genModel.c"; src = open(p).read()
head, rest = src.split("void invoke(float* labels){", 1); body, tail = rest.split("\n}\n", 1)
out, k = [], 0
for ln in body.split("\n"):
    out.append(ln)
    m = re.match(r"^\s*([a-zA-Z_0-9]+)\((.*)\);\s*$", ln)
    if not m or "buffer0" not in ln: continue
    fn, args = m.group(1), [x.strip() for x in m.group(2).split(",")]
    if fn == "add_fpreq": out.append(f"dbg_dump({k},\"{fn}\",{args[9]},{args[0]},0,0,0);")
    elif fn == "upsample_byte":
        w, h, c, f = int(args[1]), int(args[2]), int(args[3]), int(args[5]); out.append(f"dbg_dump({k},\"{fn}\",{args[4]},{w*h*c*f*f},{w*f},{h*f},{c});")
    else:
        o = re.findall(r"&buffer0\[(\d+)\],(\d+),(\d+),(\d+)", m.group(2))
        if not o: out.append(f"/* no dump {fn} */"); continue
        o = o[-1]; out.append(f"dbg_dump({k},\"{fn}\",&buffer0[{o[0]}],{o[1]}*{o[2]}*{o[3]},{o[1]},{o[2]},{o[3]});")
    k += 1
s = head + ("#include <stdio.h>\n#include <string.h>\nstatic void dbg_dump(int k,const char*fn,signed char*p,int n,int w,int h,int c){"
            "char name[48]={0};strncpy(name,fn,47);int hdr[5]={k,n,w,h,c};fwrite(hdr,4,5,stderr);fwrite(name,1,48,stderr);fwrite(p,1,n,stderr);}\n"
            "void invoke(float* labels){") + "\n".join(out) + "\n}\n" + tail
open(p, "w").write(s); print(k, "dumps")
