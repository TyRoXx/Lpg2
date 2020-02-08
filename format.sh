#!/usr/bin/env bash
find benchmarks tests -iname '*.h' -o -iname '*.cpp' | xargs clang-format-3.9 -i
