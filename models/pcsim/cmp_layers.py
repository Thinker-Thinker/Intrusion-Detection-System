"""Compare a dumping TinyEngine build with TFLite intermediate tensors, matching layers by output shape in order."""
import struct, subprocess, sys, numpy as np, tensorflow as tf
def compare(tflite, exe, x_int8, n_out):
    itp = tf.lite.Interpreter(tflite, experimental_preserve_all_tensors=True); itp.allocate_tensors()
    ii = itp.get_input_details()[0]; itp.set_tensor(ii["index"], x_int8[None]); itp.invoke()
    td = itp.get_tensor_details()
    ops = [o for o in itp._get_ops_details() if o["op_name"] not in ("DELEGATE", "PAD", "RESHAPE")]
    err = subprocess.run([exe, str(x_int8.size), str(n_out)], input=x_int8.tobytes(), capture_output=True).stderr
    dumps = []; i = 0
    while i < len(err):
        k, n, w, h, c = struct.unpack("5i", err[i:i + 20]); fn = err[i + 20:i + 68].split(b"\0")[0].decode()
        dumps.append((fn, n, np.frombuffer(err[i + 68:i + 68 + n], np.int8))); i += 68 + n
    j = 0
    for fn, n, v in dumps:
        while j < len(ops) and int(np.prod(td[ops[j]["outputs"][0]]["shape"])) != n: j += 1
        if j >= len(ops): print(fn, "no TFLite match"); break
        t = itp.get_tensor(ops[j]["outputs"][0]).ravel(); d = np.abs(v.astype(int) - t.astype(int))
        flag = "" if d.max() <= 2 else "   <-- differs"
        print(f"{ops[j]['op_name'][:18]:18s} {fn[:44]:44s} n={n:6d} max {d.max():3d} identical {100*(d==0).mean():5.1f}%{flag}")
        j += 1
