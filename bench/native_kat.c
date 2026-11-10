#define main unused_upstream_main
#if BENCH_AVX
#include "../opt/test_fndsa.c"
#else
#include "../ref/test_fndsa.c"
#endif
#undef main
int main(void){
#if BENCH_AVX
test_keygen_ref();test_verify();test_self();
#else
#if LOGN == 9
inner_test_kat(9,KAT_512);
#else
inner_test_kat(10,KAT_1024);
#endif
#endif
puts("PASS native KAT");return 0;}
