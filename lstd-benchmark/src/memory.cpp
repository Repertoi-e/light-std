#include <benchmark/benchmark.h>

#include <vector>

#include <lstd/containers.hpp>

static void stl_vector(benchmark::State& state) {
    For(state) {
        std::vector<s32> vec;
        For(lstd::range(1000)) { vec.push_back(it); }
    }
}

static void cpp_dynamic_array(benchmark::State& state) {
    For(state) {
        lstd::Dynamic_Array<s32> vec;
        For(lstd::range(1000)) { vec.add(it); }
    }
}

BENCHMARK(stl_vector);
BENCHMARK(cpp_dynamic_array);