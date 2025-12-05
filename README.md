# Vectorized Falcon-Sign

This is the artifact corresponding to the paper ["Vectorized Falcon-Sign Implementations using SSE2, AVX2, AVX-512, NEON, and RVV" (IACR TCHES 2026)](https://eprint.iacr.org/2025/1867).

## Directory Structure and Basic Project Organization

Directory structure:
- `help/`: some helper scripts
- `opt/`: optimized code implementations, supporting three target platforms
- `profiling/`: benchmarks for some subroutines, such as BaseSampler, FFT/iFFT
- `ref/`: reference implementation, derived from the public domain C-FN-DSA project (https://github.com/pornin/c-fn-dsa at commit id 96e3b92)

The three target platforms used in our paper:
- The Intel i7-11700K CPU (Rocket Lake microarchitecture) operating at 3.6 GHz. Hyper-Threading and Turbo Boost are disabled. Ubuntu 24.04 with GCC 13.3.0.
- The Cortex-A72 processor in Raspberry Pi 4B running at 1.5 GHz. Ubuntu 20.04 with Clang 10.0.0.
- The SpacemiT X60 core in Milk-V Jupiter operating at 2.0 GHz, supporting the RV64GCBV instruction set with vector extension v1.0 (VLEN = 256 bits) and bit-manipulation extension v1.0.0. Bianbu 1.0.15 (Linux kernel 6.1.15) with GCC 13.2.0. The Bianbu 1.0.15 firmware can be found at [jupiter-bianbu-build v1.0.15](https://github.com/milkv-jupiter/jupiter-bianbu-build/releases/tag/v1.0.15).

To precisely reproduce the performance data reported in our paper, ensure your hardware and software environment is as consistent as possible with our experimental environment (Section 2 of our paper).

Each of the directories `opt/`, `profiling/`, and `ref/` contains three different Makefile files for compiling code for different platforms, namely `Makefile`, `Makefile.armv8a`, and `Makefile.rv`. You should specify the correct Makefile for compilation.
For example, in the `profiling/` directory:
- On the Intel i7-11700K, you can compile using: `make all -j`
- On the ARM Cortex-A72, you can compile using: `make all -j -f Makefile.armv8a`
- On the SpacemiT X60, you can compile using: `make all -j -f Makefile.rv`

## Regarding obtaining CPU clock cycles

### Summary

On the three platforms mentioned in this project, you will need to perform some configuration to obtain the CPU clock cycle. You can find detailed explanations in the comments section of `ref/speed_fndsa.c`:

- For Intel CPU: Run the command as root: `echo 2 > /sys/bus/event_source/devices/cpu/rdpmc` and run `sysctl -w kernel.perf_event_paranoid=-1`
- For AArch64: Follow the instructions at https://github.com/jerinjacobk/armv8_pmu_cycle_counter_el0
- For RISC-V: Simply put, you need to run the executable file using `perf stat ./speed_fndsa`. For example, you can find the command in `opt/Makefile.rv`: `perf stat ./out/speed_fndsa_rv64gc >>speed_fndsa_x60.txt 2>/dev/null`

### For RISC-V

If you are using the Bianbu 1.0.15 mentioned above, you don't need to worry about this issue. However, if you are using a newer version, you may encounter the following problem.

Regarding obtaining CPU clock cycles on RISC-V, in `opt/speed_fndsa.c` you will see this code: `__asm__ __volatile__("rdcycle %0" : "=r"(x));` This will produce an error in newer Linux kernel versions (e.g., 6.6.63) because `rdcycle` causes "Illegal Instruction", see https://forum.banana-pi.org/t/how-to-enable-rdinstret-and-rdcycle-on-bananapi-bpi-f3/19212 and https://github.com/camel-cdr/rvv-bench-results/issues/1. Therefore, it is recommended that you use the `perf` tool to obtain the CPU clock cycles.

In fact, the relevant speed tests under `profiling/` in this project are based on the `perf` tool, see `profiling/cpucycles.c`. Therefore, if you need to modify the method for obtaining clock cycles in `opt/speed_fndsa.c`, you can refer to the relevant speed tests in `profiling/`.

## Reproducing the Results in the Paper

### Table 3

To reproduce "Table 3: The performance profiling of Falcon-1024’s signature generation", you first need to install `gperftools`, which can be done with the following commands:

```bash
# Install the dependencies
sudo apt install build-essential autoconf libtool
git clone https://github.com/gperftools/gperftools.git
cd gperftools
./autogen.sh
./configure
make
sudo make install
```

After running the above commands, the libraries will be installed in `/usr/local/lib`.
Then install the `pprof` tool. We recommend setting up a Go language environment first, then installing `pprof` via `go install github.com/google/pprof@latest`, and ensure its path is added to your PATH.

For the Intel i7-11700K, go to the `ref/` directory: 
```bash
make all -j
make run_profiling
```

You will then get multiple txt files, such as `gperf_sign_core_1024_avx2.txt`, which contain the profiling results of the AVX2 version.

For the SpacemiT X60, the steps are similar: first install `gperftools` and `pprof`, then run `make run_profiling -f Makefile.rv` in the `ref/` directory.

The `help/process_gperf.ipynb` file might be helpful in processing the above profiling results.

### Table 4 and Table 5

To reproduce "Table 4: Benchmark results of various BaseSampler implementations" and "Table 5: Benchmark results of FFT/iFFT implementations on SpacemiT X60.", the main work is in the `profiling/` directory.

For the Intel i7-11700K:
```bash
make all -j
make run_speed
```

You will then get `speed_gaussian0_11700k.txt`, which contains the experimental results of BaseSampler for SSE2, AVX2, and AVX-512F instruction sets.

If you want to test the correctness of our BaseSampler implementation, run `make run_test`. If no output from the `diff` command is observed, it indicates that the test passed.

For the Cortex-A72:
```bash
make all -j -f Makefile.armv8a
make run_speed -f Makefile.armv8a
```

The file `speed_gaussian0_cortex_a72.txt` you get contains the experimental results of BaseSampler for the NEON instruction set.

For the SpacemiT X60:
```bash
make all -j -f Makefile.rv
make run_speed -f Makefile.rv
```

The file `speed_gaussian0_x60.txt` you get contains the experimental results of BaseSampler for the RISC-V instruction set.
The file `speed_fft_rv64d_x60.txt` you get contains the experimental results of FFT/iFFT for the RISC-V instruction set.

### Table 6

To reproduce "Table 6: Benchmark results of Falcon-{512,1024}’s signature generation (`sign_core` subroutine) on three target platforms (8 distinct instruction set configurations).", the main work is in the `ref/` and `opt/` directories.

First, reproduce the results of the reference implementations in the `ref/` directory.

For the Intel i7-11700K:
```bash
make all -j
make run_speed
```
You will then get `speed_fndsa_11700k.txt`, which contains the experimental results of the reference implementations for the `sign_core` subroutine for SSE2, AVX2, and AVX-512F instruction sets.

For the Cortex-A72:
```bash
make all -j -f Makefile.armv8a
make run_speed -f Makefile.armv8a
```

The file `speed_fndsa_cortex_a72.txt` you get contains the experimental results of the reference implementations for the `sign_core` subroutine for the NEON instruction set.

For the SpacemiT X60:
```bash
make all -j -f Makefile.rv
make run_speed -f Makefile.rv
```

The file `speed_fndsa_x60.txt` you get contains the experimental results of the reference implementations for the `sign_core` subroutine for the RISC-V instruction set.

Then, reproduce the results of our optimized implementations in the `opt/` directory. The commands and the filenames of the files you get are the same as those in the `ref/` directory, so they are not repeated here.

### Others

In Section 7, we mentioned: "For implementations using NEON, the performance improvement is 17% compared to the reference implementation. If we exclude the 4-way hybrid Keccak and optimized FFT/iFFT, the improvement reduces to 9%. Integrating our BaseSampler with the 4-way hybrid Keccak results in a 13% improvement over the reference implementation."

If you want to reproduce the result on Cortex-A72 for "If we exclude the 4-way hybrid Keccak and optimized FFT/iFFT, the improvement reduces to 9%":
In the `opt/` directory, change `Makefile.armv8a` to ` -DFNDSA_NEON_HYBRID_SHA3=0 -DFNDSA_NEON_FFT_OPT=0`, then:

```bash
make clean -f Makefile.armv8a
make all -j -f Makefile.armv8a
make run_speed -f Makefile.armv8a
```

If you want to reproduce the result on Cortex-A72 for "Integrating our BaseSampler with the 4-way hybrid Keccak results in a 13% improvement over the reference implementation":
In the `opt/` directory, change `Makefile.armv8a` to ` -DFNDSA_NEON_HYBRID_SHA3=1 -DFNDSA_NEON_FFT_OPT=0`, then:

```bash
make clean -f Makefile.armv8a
make all -j -f Makefile.armv8a
make run_speed -f Makefile.armv8a
```

In Section 7, we mentioned: "All four versions on RISC-V show significant improvements. ... Without the optimized Keccak, the improvement is 41% compared to the reference implementation."

If you want to reproduce the above result on SpacemiT X60:
In the `opt/` directory, change `Makefile.rv` to `-DKECCAK_OPT=0`, then:

```bash
make clean -f Makefile.rv
make all -j -f Makefile.rv
make run_speed -f Makefile.rv
```

Section 7 mentions "our implementation using AVX2 increases the code size by approximately 2.7 KB compared to the reference implementation"
To reproduce this result, run the following commands in the `ref/` and `opt/` directories respectively, and then compare the results:

```bash
nm out/speed_fndsa_avx2 --print-size --size-sort --radix=d | \
awk '{$1=""}1' | \
awk '{sum+=$1 ; print $0} END{print "Total size =", sum, "bytes =", sum/1024, "kB"}' > speed_fndsa_avx2_symbols_size.txt
```

## Acknowledgement

We thank the artifact evaluation reviewers of TCHES 2026 for their valuable feedback.
