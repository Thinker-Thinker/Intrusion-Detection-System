#!/usr/bin/env python3
"""
Make TinyEngine's float requantisation round to nearest instead of truncating toward zero.

TinyEngine's *_fpreq kernels compute (int)(acc * scale), which drops up to one step per layer, always in the
same direction. Over SCRFD's 60 layers that drifted outputs by >100 int8 steps vs TFLite. Rounding fixes it.

    python apply_rounding.py <dir> [<dir> ...]     # TinyEngine/src/kernels and each generated codegen/Source
"""
import re, sys, pathlib
MACRO = "#ifndef TE_RND\n#define TE_RND(x) ((x) >= 0.0f ? (x) + 0.5f : (x) - 0.5f)\n#endif\n"
p1 = re.compile(r"\(q31_t\)\s*\(\(float\)\s*([A-Za-z_]\w*)\s*\*\s*([^;()]+?)\)")              # (q31_t) ((float) s * k)
p2 = re.compile(r"^(\s*)([A-Za-z_]\w*)\s*=\s*\(float\)\s*([A-Za-z_]\w*)\s*\*\s*([^;]+);", re.M)  # s = (float) s * k;
total = 0
for d in sys.argv[1:]:
    for f in pathlib.Path(d).rglob("*.c"):
        s = f.read_text(errors="ignore")
        if "TE_RND(" in s and "#define TE_RND" in s: continue
        s2, n1 = p1.subn(r"(q31_t)TE_RND((float)\1 * \2)", s)
        s2, n2 = p2.subn(r"\1\2 = (q31_t)TE_RND((float)\3 * \4);", s2)
        if n1 + n2:
            f.write_text(MACRO + s2); total += n1 + n2; print(f"{f.name}: {n1 + n2}")
print("replacements:", total)
