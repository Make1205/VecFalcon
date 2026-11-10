# Falcon backup — VecFalcon (older pre-standard FN-DSA) — QUICK

QUICK is a workflow check only. Full-scale figures are measured locally.

CPU: AMD EPYC 9V74 80-Core Processor; affinity: 0. GCC: gcc (Debian 14.2.0-19) 14.2.0.

Tested commit: `61b326e9bb6afe2ea4927e7a25832e7a64485367`. Benchmark commit: `d4b8a04b746d9b96812cb97c1465734b5546c7cf`.

Base flags: `-O3 -march=native -std=c99 -D_DEFAULT_SOURCE`. Each operation: 11 iterations; correctness and lengths: 100 signatures per build/set. Message: 32 bytes. Median units: thousand RDTSC ticks.

Ref backend: Upstream SSE2 hardware binary64, FNDSA_AVX2=0, FNDSA_SSE2=1; not a pure-C no-intrinsics backend (none exists on x86 in this source).

Timing sample population: fresh KeyGen/Sign/Verify triple per round, 20 discarded warmup triples; upstream OS entropy.

Exact per-build options (in addition to base flags and local include paths):

* 512 ref: `-DLOGN=9 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=0 -DFNDSA_SSE2=1 -DBENCH_AVX=0 -msse2 -mno-avx -mno-avx2 -mno-avx512f -mno-fma`; link `-Wl,--gc-sections`.
* 512 avx2: `-DLOGN=9 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=1 -DFNDSA_SSE2=1 -mavx2 -mno-avx512f -DBENCH_AVX=1`; link `-Wl,--gc-sections`.
* 512 soft: `-DLOGN=9 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=0 -DFNDSA_SSE2=0 -DBENCH_AVX=0 -mno-avx -mno-avx2 -mno-avx512f -mno-fma`; link `-Wl,--gc-sections`.
* 1024 ref: `-DLOGN=10 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=0 -DFNDSA_SSE2=1 -DBENCH_AVX=0 -msse2 -mno-avx -mno-avx2 -mno-avx512f -mno-fma`; link `-Wl,--gc-sections`.
* 1024 avx2: `-DLOGN=10 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=1 -DFNDSA_SSE2=1 -mavx2 -mno-avx512f -DBENCH_AVX=1`; link `-Wl,--gc-sections`.
* 1024 soft: `-DLOGN=10 -DFNDSA_SHAKE256X4=1 -DFNDSA_NEON=0 -ffunction-sections -fdata-sections -DFNDSA_AVX2=0 -DFNDSA_SSE2=0 -DBENCH_AVX=0 -mno-avx -mno-avx2 -mno-avx512f -mno-fma`; link `-Wl,--gc-sections`.

| 参数集 | n | q | pk | sig（仓库实际） | 论文 sig | ref KeyGen/Sign/Verify | AVX2 KeyGen/Sign/Verify |
|---|---:|---:|---:|---|---:|---|---|
| 512 | 512 | 12289 | 897 | 666; unpadded avg 654.9500, max 660 | 666 | 11224.329/485.393/61.151 | 7645.377/476.969/61.749 |
| 1024 | 1024 | 12289 | 1793 | 1280; unpadded avg 1270.9400, max 1278 | 1280 | 43953.571/932.827/119.391 | 28445.247/929.551/95.939 |



Expanded-key Sign (thousand ticks; `--` means no supported expanded-key API):

| 参数集 | ref | AVX2 |
|---|---:|---:|
| 512 | -- | -- |
| 1024 | -- | -- |

Supplemental software binary64 (not the main ref; thousand ticks):

| 参数集 | soft KeyGen/Sign/Verify |
|---|---|
| 512 | 11365.015/7874.333/62.087 |
| 1024 | 41586.843/16564.781/122.277 |

Actual encoded length by backend (internal encoding parser; padding excluded):

| 参数集 | Backend | Samples | Mean bytes | Max bytes |
|---|---|---:|---:|---:|
| 512 | ref | 100 | 654.9500 | 660 |
| 512 | avx2 | 100 | 655.1900 | 659 |
| 512 | soft | 100 | 654.9500 | 660 |
| 1024 | ref | 100 | 1270.9400 | 1278 |
| 1024 | avx2 | 100 | 1270.6600 | 1279 |
| 1024 | soft | 100 | 1270.9400 | 1278 |

Byte equality diagnostics (differences do not waive cross-verification):

| 参数集 | Backend pair | Equal | Differing signatures | First differing sample (1-based) |
|---|---|---|---:|---:|
| 512 | ref / avx2 | False | 100 | 1 |
| 512 | ref / soft | True | 0 | -- |
| 512 | avx2 / soft | False | 100 | 1 |
| 1024 | ref / avx2 | False | 100 | 1 |
| 1024 | ref / soft | True | 0 | -- |
| 1024 | avx2 / soft | False | 100 | 1 |

All ordered backend cross-verification pairs passed, including the supplemental software backend. First-difference signatures, seeds, message, offsets and hashes are retained under diagnostics/ when a difference occurs.

Correctness: all configured upstream KAT/self-tests passed; all generated signatures verified; changed messages rejected. Cross-build/equality details are in correctness.json.

The paper primary implementation is VecFalcon. The actual repository API is pre-standard FN-DSA derived from c-fn-dsa, not the original round-3 Falcon API. Empty context and RAW message mode are used; current hash_to_point binds the hashed public key/context unless the legacy 0xFF mode is requested, which is not used here. Signatures are padded fixed-size 666/1280, not compressed variable-size wire format. Exact unpadded statistics parse the compressed polynomial after the 1-byte header and 40-byte nonce, independently checked by comp_decode. Scalar ref disables FNDSA_AVX2 and FNDSA_SSE2 and uses the supported software binary64 implementation; opt AVX2 uses its native SIMD/FPU path. FNDSA_SHAKE256X4=1 matches the artifact Makefile in both. Public C API accepts only encoded keys and exposes no expanded signing state. Native known-answer routines and fixtures are included unchanged through a bench wrapper; only the targeted degree KAT is invoked, avoiding the unrelated large native self-test suite. Important KAT limitation: the opt sampler changes seeded transcripts and its original run_tests comments out test_kat. The inherited full signing-transcript hash KAT fails for opt (first SHA3-256 c677b5dd... vs expected a32f07ba...). The scalar targeted signing KAT is required and passes. For opt, unchanged upstream keygen KATs, original-Falcon verification vectors and its enabled self-tests are required, followed by the fixed-seed cross-build checks. No expected hashes or assertions are modified; this incompatible inherited signing KAT is not reported as a passing test. Revision 2026-11-10: c-fn-dsa is primary and VecFalcon backup. Main ref now uses upstream SSE2 hardware binary64 with AVX2 disabled; it contains SSE2 intrinsics and is not a pure-C portable double path. The source has no FALCON_FPNATIVE switch. Software binary64 is supplemental only. GCC -O3 -march=native plus backend-required options; byte keys, 32-byte messages, fresh key/sign/verify triples, 20 warmup triples and unchanged Dilithium timing files. Every backend pair cross-verifies; equality and first differences are recorded separately. Original local FULL mismatch evidence remains pending until the original vectors are inspected.

Build flags and commands per variant are in config.json, metadata.json and build logs. All source is extracted locally from the tested commit; algorithm files are unchanged. See bench/NOTES.md.
