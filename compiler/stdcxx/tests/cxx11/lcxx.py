#!/usr/bin/env python3
"""Run libc++ 8 tests (.pass.cpp) against stdcxx with pathCC -fpsclang.

usage: lcxx.py [--libcxx DIR] [--stage DIR] [-std c++11] [-O O0] [-j N]
               [-o results.json] [-v] [--host] PATH...

PATH is a test or directory, relative to libc++'s test/std (for example
utilities/tuple) or absolute.

  --libcxx  libc++ 8.0.1 sources (env LIBCXX_SRC; see fetch-libcxx.sh)
  --stage   the build's stage directory, BUILD/Xcompiler (env PATH64_STAGE)
  --host    compile with the host clang++ (hostcc.sh) instead of psclang:
            the same headers and runtime, a different code generator
  -v        print each failure

Results: PASS, COMPILE (does not compile), RUN (fails or crashes),
TIMEOUT, UNSUPPORTED (libc++'s UNSUPPORTED/REQUIRES markers exclude it for
this -std), XFAIL (libc++ marks it XFAIL for clang-3 or clang-4, as
psclang is Clang 4.0.1).
"""
import argparse, concurrent.futures as cf, json, os, re, signal, subprocess, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))

def run_command(command, timeout):
    # The driver spawns compiler/linker children. Kill the whole test process
    # group on timeout, rather than leaving those processes running in CI.
    with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          universal_newlines=True, errors="replace",
                          start_new_session=True) as process:
        try:
            output, _ = process.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            process.communicate()
            raise
        return subprocess.CompletedProcess(command, process.returncode, output)

def unsupported(src, std):
    head = open(src, errors="replace").read(4000)
    for m in re.finditer(r"//\s*UNSUPPORTED:(.*)", head):
        items = [x.strip() for x in m.group(1).split(",")]
        if std in items:
            return True
    for m in re.finditer(r"//\s*REQUIRES:(.*)", head):
        items = [x.strip() for x in m.group(1).split(",")]
        if any(x.startswith("c++") for x in items) and std not in items:
            return True
        if any(x.startswith("locale.") for x in items):
            return True
    return False

def xfail(src):
    head = open(src, errors="replace").read(4000)
    for m in re.finditer(r"//\s*XFAIL:(.*)", head):
        items = [x.strip() for x in m.group(1).split(",")]
        if "clang-4" in items or "clang-3" in items:
            return True
    return False

def run_one(a, src, work):
    rel = os.path.relpath(src, a.tests)
    st, msg = run_one0(a, src, rel, work)
    if xfail(src) and st not in ("PASS", "UNSUPPORTED"):
        st = "XFAIL"
    return rel, st, msg

def run_one0(a, src, rel, work):
    if unsupported(src, a.std):
        return "UNSUPPORTED", ""
    exe = os.path.join(work, rel.replace("/", "__"))
    if a.host:
        cmd = [os.path.join(HERE, "hostcc.sh"), "-std=" + a.std, "-" + a.O,
               "-I", a.support, src, "-o", exe]
    else:
        cmd = [os.path.join(a.stage, "bin", "pathCC"), "-fpsclang",
               "-std=" + a.std, "-" + a.O, "-I", a.support, src, "-o", exe,
               "-lpthread", "-lm"]
    try:
        p = run_command(cmd, a.compile_timeout)
    except subprocess.TimeoutExpired:
        return "TIMEOUT", "compilation exceeded %g seconds" % a.compile_timeout
    if p.returncode != 0 or not os.path.exists(exe):
        msg = [l for l in p.stdout.splitlines() if "error" in l or "###" in l]
        return "COMPILE", re.sub(r"\x1b\[[0-9;]*m", "", "\n".join(msg[:3]))
    try:
        r = run_command([exe], a.timeout)
        res = ("PASS" if r.returncode == 0 else "RUN", r.stdout[-300:])
    except subprocess.TimeoutExpired:
        res = ("TIMEOUT", "")
    os.unlink(exe)
    return res

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--libcxx", default=os.environ.get("LIBCXX_SRC", ""))
    ap.add_argument("--stage", default=os.environ.get("PATH64_STAGE", ""))
    ap.add_argument("-std", default="c++11")
    ap.add_argument("-O", default="O0")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 4)
    ap.add_argument("-o")
    ap.add_argument("-v", action="store_true")
    ap.add_argument("--host", action="store_true")
    ap.add_argument("--strict", action="store_true", help="exit nonzero for unexpected test failures")
    ap.add_argument("--compile-timeout", type=float, default=120,
                    help="per-test compilation limit in seconds (default: 120)")
    ap.add_argument("--timeout", type=float, default=30, help="per-test runtime limit in seconds (default: 30)")
    ap.add_argument("paths", nargs="+")
    a = ap.parse_args()
    if a.timeout <= 0 or a.compile_timeout <= 0 or a.j <= 0:
        ap.error("timeouts and -j must be positive")
    if a.libcxx and not os.path.isdir(os.path.join(a.libcxx, "test", "std")):
        ap.error("set --libcxx or LIBCXX_SRC to the libc++ 8.0.1 sources")
    if not a.libcxx and any(not os.path.isabs(p) for p in a.paths):
        ap.error("relative reference paths require --libcxx or LIBCXX_SRC; original tests may use absolute paths")
    if not a.host and not os.path.exists(os.path.join(a.stage, "bin", "pathCC")):
        ap.error("set --stage or PATH64_STAGE to BUILD/Xcompiler")
    a.tests = os.path.join(a.libcxx, "test", "std") if a.libcxx else HERE
    a.support = os.path.join(a.libcxx, "test", "support") if a.libcxx else HERE
    os.environ["PATH64_STAGE"] = a.stage
    srcs = []
    for p in a.paths:
        p = p if os.path.isabs(p) else os.path.join(a.tests, p)
        if not os.path.exists(p):
            ap.error("test path does not exist: " + p)
        if os.path.isfile(p):
            srcs.append(p)
        else:
            for d, _, fs in os.walk(p):
                srcs += [os.path.join(d, f) for f in fs if f.endswith(".pass.cpp")]
    srcs = sorted(set(srcs))
    if not srcs:
        ap.error("no .pass.cpp tests found")
    res = {}
    with tempfile.TemporaryDirectory(prefix="lcxx-") as work:
        with cf.ThreadPoolExecutor(a.j) as ex:
            for rel, st, msg in ex.map(lambda s: run_one(a, s, work), srcs):
                res[rel] = (st, msg)
                if a.v and st not in ("PASS", "UNSUPPORTED", "XFAIL"):
                    print("%-8s %s\n         %s" % (st, rel,
                          msg.replace("\n", "\n         ")[:400]))
    counts = {}
    for st, _ in res.values():
        counts[st] = counts.get(st, 0) + 1
    run = sum(v for k, v in counts.items() if k not in ("UNSUPPORTED", "XFAIL"))
    print("%s %s: %d/%d pass %s" % (a.std, a.O, counts.get("PASS", 0), run, counts))
    if a.o:
        json.dump(res, open(a.o, "w"), indent=1)
    if a.strict and any(st not in ("PASS", "UNSUPPORTED", "XFAIL") for st, _ in res.values()):
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
