#include <benchmark/benchmark.h>

static void benchmark_store_blob(benchmark::State &state)
{
    for (auto _ : state)
    {
    }
    state.SetBytesProcessed(6666);
}

BENCHMARK(benchmark_store_blob)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
