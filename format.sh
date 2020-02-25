#!/usr/bin/env bash
find benchmarks lpg2 tests -iname '*.h' -o -iname '*.cpp' | xargs clang-format-3.9 -i
