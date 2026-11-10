# Benchmark notes

Initial worktree: clean. `bench` was created from the recorded initial commit.
Only files under `bench/` are added. Algorithm source, parameters and original
build files remain unchanged. The runner extracts the selected commit with local
`git archive`, so an older upstream revision can be measured without resetting
the working branch or rewriting files. No network is used at run time.

Initial commit: `61b326e9bb6afe2ea4927e7a25832e7a64485367`.
Tested commit: `61b326e9bb6afe2ea4927e7a25832e7a64485367`.

## Timing source and measurement

The four unmodified timing files come from pq-crystals/dilithium
`bba9534a3c5cbef66481cd6518d786e23ae601b5`, `ref/test/` (there is no root
`test/` directory at that revision). They match the Antrag bench copies byte
for byte. RDTSC is used; USE_RDPMC is not defined. The unchanged print_results
function receives iterations+1 timestamps for exactly iterations operations,
subtracts its own counter overhead, and computes the median. Thousand cycles
means RDTSC ticks divided by 1000. Loop/call overhead is included consistently.

System GCC: -O3 -march=native -std=gnu11. Ref additionally disables automatic
loop/SLP vectorization; algorithm SIMD intrinsics are selected only in the
upstream SIMD implementation. Exact flags are recorded per variant in
config.json, metadata.json and build logs. Messages have 32 bytes. Signing and
verification receive serialized byte keys and include all decoding/expansion
performed by the upstream byte API. KeyGen returns serialized keys.
Expanded-key Sign is reported only when a supported API is available.

QUICK=1: 11 timing operations, upstream KAT/self-tests, 100 generated signatures.
Default FULL: 10001 timing operations, upstream KAT/self-tests, 10000 generated
signatures. Only QUICK is executed in cloud. Full-scale runs are for the user's
local machine. No test assertions are disabled. The correctness and length
statistics reuse the same signature pool, using a fixed-seed key per build/set.
All signatures self-verify, cross-verify when another build exists, and are
checked for rejection after changing the message. Equality expectations are
configured explicitly; floating-point implementations can use cross verification
when byte equality is inappropriate. No result is declared complete unless all
required commands finish with zero status. The CSV contains medians and sizes;
raw timer output and correctness evidence are retained.

A bench-only xorshift64* byte stream (seed 1234567, fixed update and byte order in
driver.c) supplies reproducible test entropy where the API accepts an external
provider. This is for correctness vectors, not production randomness. For NIST
APIs the timing provider is the original randombytes implementation, renamed only
in its provider translation unit and routed through a bench wrapper; the algorithm
call sites remain unchanged. Callback APIs use the repository SHAKE generator
with fixed test seeds and OS-seeded timing state as documented below.

Missing system-library headers/libraries are compile/link checked at startup,
before algorithm builds. The runner never installs packages. Bash, Python 3.8+,
Git and system GCC are required; additional apt dependencies are listed below.

## Scheme-specific configuration and format

The paper primary implementation is VecFalcon. The actual repository API is pre-standard FN-DSA derived from c-fn-dsa, not the original round-3 Falcon API. Empty context and RAW message mode are used; current hash_to_point binds the hashed public key/context unless the legacy 0xFF mode is requested, which is not used here. Signatures are padded fixed-size 666/1280, not compressed variable-size wire format. Exact unpadded statistics parse the compressed polynomial after the 1-byte header and 40-byte nonce, independently checked by comp_decode. Scalar ref disables FNDSA_AVX2 and FNDSA_SSE2 and uses the supported software binary64 implementation; opt AVX2 uses its native SIMD/FPU path. FNDSA_SHAKE256X4=1 matches the artifact Makefile in both. Public C API accepts only encoded keys and exposes no expanded signing state. Native known-answer routines and fixtures are included unchanged through a bench wrapper; only the targeted degree KAT is invoked, avoiding the unrelated large native self-test suite. Important KAT limitation: the opt sampler changes seeded transcripts and its original run_tests comments out test_kat. The inherited full signing-transcript hash KAT fails for opt (first SHA3-256 c677b5dd... vs expected a32f07ba...). The scalar targeted signing KAT is required and passes. For opt, unchanged upstream keygen KATs, original-Falcon verification vectors and its enabled self-tests are required, followed by the fixed-seed cross-build checks. No expected hashes or assertions are modified; this incompatible inherited signing KAT is not reported as a passing test.

## Native-source compilation changes

No algorithm source patches. All source copies come from the tested commit.
Additional source selection, preprocessor options and include paths appear in
config.json. Configuration-only exceptions, if any, are documented below.

## Timing file checksums

* cpucycles.c: `31cc4c16b1e896b2cf7cc154aa21430329c0047cf9898a1865f9bfe9904f25e4`
* cpucycles.h: `7f1e2b68cf3e3c76076364af3c5350de61442450c477aec809da8de10e131838`
* speed_print.c: `533fc7bd12204078a5390cfb58c05d550b214850876609f8e98f7377b9d4f1b3`
* speed_print.h: `3946b0226c63125664eb443a20d43563e6d1ba7fa3be5c1b952cd29d1477b2ef`

Additional apt dependencies: none beyond build-essential, python3 and git.

The requested upstream revision equals initial HEAD. The public C API is the
pre-standard FN-DSA API in ref/fndsa.h / opt/fndsa.h, not the original Falcon
round-3 API. The top-level README says ref derives from c-fn-dsa 96e3b92; the
embedded README records 1cdc9c5. These provenance strings differ, so tested Git
commit 61b326e is the authoritative implementation identity. Both signing and
verification use empty context and FNDSA_HASH_ID_RAW; hash_to_point binds nonce,
hashed public key, raw/prehash flag, context length and message. The available
legacy 0xFF mode omits that binding but is not used for timing.

Reference code is built from ref/ with FNDSA_AVX2=0,FNDSA_SSE2=0,FNDSA_NEON=0,
compiler vectorization disabled. This invokes the upstream portable software
binary64 arithmetic, not the Makefile's SSE2 reference baseline. AVX2 is built
from opt/ with FNDSA_AVX2=1,FNDSA_SSE2=1 and AVX-512 disabled. Both use the
artifact's FNDSA_SHAKE256X4=1. This float-backend distinction is part of the
reported ref/AVX2 comparison. No arithmetic or sampler implementation is changed.

Native KAT limitation: the inherited opt full signing-transcript KAT does not
match the optimized sampler's seeded output. First computed SHA3-256 was
c677b5dd5432250fcaac76d517f838472c7e4d17238d1486b5feb3732f95689d,
versus inherited expected
a32f07baf6b7ff6bc7c3c4f8c638871ff8c4803b0e54bedb9363f5672011077b.
The upstream opt run_tests() already comments out test_kat, test_sampler and
other original scalar-sampler fixtures. opt/sign_sampler.c implements batched
vector sampling and consumes the PRNG differently; seeded signature equality
with ref is therefore not required. No expected answer is replaced or relaxed.
The original failing transcript is retained separately as evidence.

For ref, the unchanged degree-specific inner_test_kat and its expected array
are mandatory. For opt, unchanged test_keygen_ref, test_verify (original Falcon
known-answer signatures), and the upstream enabled test_self are mandatory.
The external native_kat.c wrapper includes the original test translation unit
with main renamed and invokes those original functions. Function/data sections
and linker garbage collection omit unrelated large tests; test bodies and
assertions are unchanged. This validates available compatible KAT components
and cross-verification, not the incompatible inherited whole signing transcript.
The SUMMARY explicitly retains this limitation. Additional 100-signature QUICK
pools from both backends successfully cross-verify; whole vectors differ.

Signatures are fixed-size padded 666/1280 byte values. The bench parses the
MSB-first signed compressed coefficient stream after the header and 40-byte
nonce, and independently checks its length using comp_decode and decoded
absolute coefficient magnitudes. It does not strip zero bytes heuristically.
The native C API exposes only encoded keys; there is no reusable expanded-key
sign API to report. Sign includes byte-key decoding/reconstruction and Verify
includes byte public-key decoding exactly as requested.
