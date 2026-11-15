# Upstream

These sources come unchanged, in the commit that added this file, from
clang2whirl, the Clang-based WHIRL front end by XC5 Limited and
Futurewei Technologies:

* repository: https://gitee.com/open64ark/open64
* directory: `osprey/clang2whirl`
* commit: `1c83f8844888bd85403c9d258bfadb6f1befe79a` (2020-11-11)
* license: GPL version 2 (see each file's header)

Upstream targets Clang 7.0.1 / 11.0.0 and the upstream Open64 WHIRL.
Later commits in this tree port it to Clang 4.0.1 and to Path64's WHIRL;
see `doc/frontend/clang-feasibility.md`, sections 7 and 8. Changes made
for the port are marked "Path64" in comments. `c2w_path64.h/.cxx` hold
the upstream-Open64 interfaces Path64 lacks. `driver.cxx` and
`cc1_main.cxx` are Clang 4.0.1's own driver sources (NCSA license).
