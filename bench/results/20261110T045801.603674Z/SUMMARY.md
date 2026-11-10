# Falcon — VecFalcon (pre-standard FN-DSA API) — QUICK

QUICK is a workflow check only. Full-scale figures are measured locally.

CPU: AMD EPYC 9V45 96-Core Processor; affinity: 0. GCC: gcc (Debian 14.2.0-19) 14.2.0.

Tested commit: `61b326e9bb6afe2ea4927e7a25832e7a64485367`. Benchmark commit: `61b326e9bb6afe2ea4927e7a25832e7a64485367`.

Base flags: `-O3 -march=native -std=gnu11`. Each operation: 11 iterations; correctness and lengths: 100 signatures per build/set. Message: 32 bytes. Median units: thousand RDTSC ticks.

| 参数集 | n | q | pk | sig（仓库实际） | 论文 sig | ref KeyGen/Sign/Verify | AVX2 KeyGen/Sign/Verify |
|---|---:|---:|---:|---|---:|---|---|
| 512 | 512 | 12289 | 897 | 666; unpadded avg 654.9500, max 660 | 666 | 9458.175/5375.291/49.269 | 5677.801/334.437/40.039 |
| 1024 | 1024 | 12289 | 1793 | 1280; unpadded avg 1270.9400, max 1278 | 1280 | 36757.967/11980.253/103.661 | 20914.971/662.141/58.031 |



Expanded-key Sign (thousand ticks; `--` means no supported expanded-key API):

| 参数集 | ref | AVX2 |
|---|---:|---:|
| 512 | -- | -- |
| 1024 | -- | -- |

Correctness: all configured upstream KAT/self-tests passed; all generated signatures verified; changed messages rejected. Cross-build/equality details are in correctness.json.

The paper primary implementation is VecFalcon. The actual repository API is pre-standard FN-DSA derived from c-fn-dsa, not the original round-3 Falcon API. Empty context and RAW message mode are used; current hash_to_point binds the hashed public key/context unless the legacy 0xFF mode is requested, which is not used here. Signatures are padded fixed-size 666/1280, not compressed variable-size wire format. Exact unpadded statistics parse the compressed polynomial after the 1-byte header and 40-byte nonce, independently checked by comp_decode. Scalar ref disables FNDSA_AVX2 and FNDSA_SSE2 and uses the supported software binary64 implementation; opt AVX2 uses its native SIMD/FPU path. FNDSA_SHAKE256X4=1 matches the artifact Makefile in both. Public C API accepts only encoded keys and exposes no expanded signing state. Native known-answer routines and fixtures are included unchanged through a bench wrapper; only the targeted degree KAT is invoked, avoiding the unrelated large native self-test suite. Important KAT limitation: the opt sampler changes seeded transcripts and its original run_tests comments out test_kat. The inherited full signing-transcript hash KAT fails for opt (first SHA3-256 c677b5dd... vs expected a32f07ba...). The scalar targeted signing KAT is required and passes. For opt, unchanged upstream keygen KATs, original-Falcon verification vectors and its enabled self-tests are required, followed by the fixed-seed cross-build checks. No expected hashes or assertions are modified; this incompatible inherited signing KAT is not reported as a passing test.

Build flags and commands per variant are in config.json, metadata.json and build logs. All source is extracted locally from the tested commit; algorithm files are unchanged. See bench/NOTES.md.
