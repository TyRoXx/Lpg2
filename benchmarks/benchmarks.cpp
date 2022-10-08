#include "../lpg2/tokenizer.h"
#include <benchmark/benchmark.h>
#include <cstring>

static void benchmark_store_blob(benchmark::State &state)
{
    for (auto _ : state)
    {
    }
    state.SetBytesProcessed(6666);
}

static void benchmark_tokenizer(benchmark::State &state)
{
    const char *const string = "print(\"Hello\")"
                               "()"
                               "\"Testing\"";
    size_t i = 0;
    for (auto _ : state)
    {
        lpg::scanner s(string);
        for (;;)
        {
            auto t = s.pop();
            if (!t.has_value())
            {
                break;
            }
        }
        ++i;
    }
    state.SetBytesProcessed(static_cast<int64_t>(i * strlen(string)));
}

BENCHMARK(benchmark_store_blob)->Unit(benchmark::kMillisecond);
BENCHMARK(benchmark_tokenizer);

BENCHMARK_MAIN();
