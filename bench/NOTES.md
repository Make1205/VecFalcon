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

Historical role (superseded below): VecFalcon was previously primary; c-fn-dsa is now primary. The actual repository API is pre-standard FN-DSA derived from c-fn-dsa, not the original round-3 Falcon API. Empty context and RAW message mode are used; current hash_to_point binds the hashed public key/context unless the legacy 0xFF mode is requested, which is not used here. Signatures are padded fixed-size 666/1280, not compressed variable-size wire format. Exact unpadded statistics parse the compressed polynomial after the 1-byte header and 40-byte nonce, independently checked by comp_decode. Historical main ref (now supplemental soft) disables FNDSA_AVX2 and FNDSA_SSE2 and uses the supported software binary64 implementation; opt AVX2 uses its native SIMD/FPU path. FNDSA_SHAKE256X4=1 matches the artifact Makefile in both. Public C API accepts only encoded keys and exposes no expanded signing state. Native known-answer routines and fixtures are included unchanged through a bench wrapper; only the targeted degree KAT is invoked, avoiding the unrelated large native self-test suite. Important KAT limitation: the opt sampler changes seeded transcripts and its original run_tests comments out test_kat. The inherited full signing-transcript hash KAT fails for opt (first SHA3-256 c677b5dd... vs expected a32f07ba...). The scalar targeted signing KAT is required and passes. For opt, unchanged upstream keygen KATs, original-Falcon verification vectors and its enabled self-tests are required, followed by the fixed-seed cross-build checks. No expected hashes or assertions are modified; this incompatible inherited signing KAT is not reported as a passing test.

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

Before any algorithm build, the runner also compile/links a standard C
header/runtime/libm probe. Failure reports apt package build-essential
(including libc6-dev). Exact extra per-build flags are printed near the
SUMMARY heading as well as retained in metadata and logs. This final
preflight/report change leaves the measured driver and QUICK values unchanged.


## Current protocol and floating-point backend (2026-11-10)

This section supersedes the old ref/vectorization/timing descriptions above.
All modifications are inside bench/; no algorithm file, table, sampler, encoding
or RNG call site has been modified. Tested revisions remain pinned. See
[PROTOCOL.md](PROTOCOL.md) for the common byte-key rule from the LITHIUM audit;
its separate local 3% numerical calibration is not claimed to have passed.

System GCC uses -O3 -march=native -std=c99 -D_DEFAULT_SOURCE plus the recorded
backend macros/options. _DEFAULT_SOURCE is required to declare getentropy and
O_CLOEXEC used by the upstream Linux sysrng.c under strict C99; GNU C previously
enabled these declarations implicitly. No extra -fno-tree-vectorize or
-fno-tree-slp-vectorize is applied to the new builds. The source Makefiles use
Clang/O2 or their artifact defaults, but those defaults are not required by the
API; GCC/O3 follows the requested common protocol.

Each timing round generates fresh byte-form keys, signs a 32-byte zero message
from the byte sk (including decode and expansion), and verifies from the byte pk
(including decode and hashing). Twenty complete warmup triples are discarded.
Timing uses the upstream OS RNG, with no preexpanded key or cached matrix
supplied by bench. The four Dilithium bba9534 timing files are unchanged;
RDTSC intervals are passed to unchanged print_results for overhead subtraction
and odd-sample medians in thousand ticks. No claim of equivalence with COPA's
59-byte/RDTSCP/instrumented source workload is made.

Main ref: FNDSA_AVX2=0, FNDSA_SSE2=1, FNDSA_NEON=0; -msse2 -mno-avx
-mno-avx2 -mno-avx512f -mno-fma. It uses upstream hardware IEEE binary64 through
SSE2 intrinsics, including packed operations in FFT. It is a non-AVX2 x86
fallback, **not a pure-C portable double implementation**. The pinned sources
have no FALCON_FPNATIVE selector: disabling SSE2 selects uint64_t fpr software
emulation, not hardware double. The other native hardware paths use NEON or
RISC-V inline assembly. Providing a pure-C hardware-double backend would require
implementation changes, which are prohibited. The chosen existing SSE2 fallback
is reported explicitly in metadata and SUMMARY rather than described as scalar
portable C. This limitation awaits the user's backend preference if strict
no-intrinsics portability remains mandatory.

The original software configuration remains as soft, only in a supplemental
SUMMARY table. The avx2 backend's source and selector options are retained;
all builds adopt the fresh-triple timing protocol. Supplemental soft also
participates in all six ordered cross-verification pairs per degree. Keygen is
integer fixed-point; hardware-versus-emulated binary64 primarily concerns Sign.
Neither package requires GMP or an external floating-point library.

For comparison: Antrag ref uses native double/libm with its algorithm AVX2/FMA
selectors disabled; HAWK ref keygen and signing are integer/fixed-point and use
neither floating point nor GMP. Those repositories are read-only in this task.
The three ref labels therefore describe different actual arithmetic backends.

QUICK=1 uses 11 timing samples and 100 signatures per backend/degree, plus the
unchanged configured native KAT/self-tests. FULL uses 10001 and 10000 locally.
Actual encoded lengths still use the existing signature-bit parser checked
against native comp_decode, never heuristic removal of trailing zero bytes.
All per-backend means/maxima and first byte differences appear in SUMMARY.

Byte equality is recorded for every backend pair, but cross-verification is
the default acceptance criterion. Native KAT fixture values/assertions remain
strict. STRICT_EQUAL=1 optionally also fails on a byte difference **after** all
cross-verification checks and diagnostic records have been saved. All normal
runs keep byte-difference evidence, not merely a pass/fail equality boolean.

When a signature differs, diagnostics/<set>-<pair>/equality.json contains its
first zero-based index and one-based sample, signing/keygen seed, message,
complete signatures, hashes and byte offset; left/right.vectors retain a single
replayable case with the encoded keys. No differing sample has been observed
in the current c-fn-dsa QUICK corpus. This does not resolve an earlier FULL
failure on a different machine/corpus size.


## VecFalcon is the backup implementation

Tested source remains 61b326e9bb6afe2ea4927e7a25832e7a64485367.
The repository README identifies ref/ as derived from pornin/c-fn-dsa 96e3b92.
The Makefile's CFLAGS_SSE2 explicitly selects FNDSA_AVX2=0,FNDSA_SSE2=1;
this is the existing hardware binary64 fallback used for the new main ref.
No FALCON_FPNATIVE macro exists here. FNDSA_SHAKE256X4=1 is retained in all
backends as in the artifact's build; Makefile lines 46-59 explicitly warn that
this PRNG changes seeded transcripts and that reproducibility should not be
relied on until the standard is finalized. The opt sampler also changes RNG
batching, so equality is descriptive and every ordered cross-verification pair
is mandatory. The software ref is no longer the main timing column.

The main table still compares paper padded lengths 666/1280 with actual padded
wire lengths; compressed polynomial lengths excluding unused padding are
reported separately. The encoded-key-only API does not expose expanded Sign.
The source is older pre-standard FN-DSA, not the same algorithm/encoding version
as the primary c-fn-dsa July-2026 update. No cross-repository byte equality or
wire compatibility is claimed.

The existing opt full signing-transcript KAT incompatibility remains visible:
its computed c677b5dd... transcript differs from inherited a32f07ba... expected,
and upstream opt/run_tests already disables that test. We retain the original
failure log and run the enabled unmodified keygen/verification/self-tests;
we do not alter an expected hash or claim that incompatible transcript KAT
passed. The hardware SSE2 ref and supplemental software backend run the strict
degree-specific native signing KATs. This is an inherited opt test limitation,
not a new benchmark fix.

The final QUICK ref-backend-proof.json inspects algorithm and driver object
files: SSE hardware FP is present and no YMM/ZMM/FMA instruction occurs there.
The unchanged speed_print object uses base -march=native and may be compiler
auto-vectorized, including AVX instructions on the cloud CPU; this is post-
measurement statistics outside the timed algorithm call, not selection of an
AVX2 algorithm backend. No extra auto-vectorization suppression is used.


## FP contraction policy update (2026-11-10)

The shared rule in PROTOCOL.md now requires -ffp-contract=off for all actual
floating-point builds. The local c-fn-dsa index-3461 mismatch was traced to
compiler contraction (15 FMA -> 0; equality restored), not a source defect.
All runs now audit benchmark-owned objects and the linked driver with objdump
and save *-fma-audit.json. Missing objdump fails preflight: apt install binutils.
Both hardware ref and AVX2, plus supplemental soft, now always use the flag. No explicit x86 FMA intrinsics were found; every FMA count must be zero. VecFalcon is backup; c-fn-dsa is the paper primary. Previous optional FP_CONTRACT descriptions are superseded.
