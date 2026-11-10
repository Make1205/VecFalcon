
Verify diagnostic split (thousand TSC ticks; instrumented, separate from the main Verify metric):

| Set | Build | vzeroupper before call | PK decode/preparation | Remaining core | Instrumented total | CPU dispatch (included in core) |
|---|---|---|---:|---:|---:|---:|
| 512 | ref | False | 9.489 | 51.947 | 61.281 | 0.000 |
| 512 | avx2 | False | 2.755 | 61.463 | 64.817 | 22.801 |
| 512 | avx2 | True | 2.703 | 59.955 | 62.815 | 21.995 |
| 1024 | ref | False | 18.849 | 101.035 | 119.833 | 0.000 |
| 1024 | avx2 | False | 5.329 | 94.717 | 99.995 | 22.983 |
| 1024 | avx2 | True | 5.225 | 94.509 | 99.709 | 22.151 |

PK phase brackets original mqpoly_decode through initial representation conversion and PK NTT. Core is the same-sample remainder, including PK/message hashing, dispatch, signature decoding and verification arithmetic. Linker hooks add boundary overhead; these diagnostic medians do not replace the main public API median, and medians need not add. The clean probe inserts only vzeroupper before the timed API. Static ISA counts are in *-isa.json.

Sign diagnostic (encoded-key API, fixed seed corpus; not a replacement for the main timing):

| Set | Build | Instrumented Sign kcycles | CPU detection kcycles | Mean detection calls/sign | Max |
|---|---|---:|---:|---:|---:|
| 512 | ref | 604.967 | 0.000 | 0.0000 | 0 |
| 512 | avx2 | 568.931 | 24.595 | 1.0000 | 1 |
| 1024 | ref | 1109.549 | 0.000 | 0.0000 | 0 |
| 1024 | avx2 | 967.485 | 25.609 | 1.0000 | 1 |
