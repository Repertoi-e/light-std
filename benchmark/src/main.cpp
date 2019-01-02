#include <benchmark/benchmark.h>

#include <cppu/io/io.hpp>
#include <iomanip>
#include <iostream>

#include <fmt/format.h>

static void stl_cout(benchmark::State& state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        std::cout << "Hello, world! " << std::fixed << std::setprecision(2) << b << ' ' << std::hex << a << '\r';
    }
}

static void cpp_fmt(benchmark::State& state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        fmt::print("Hello, world! {:.{}} {:x}\r", b, 2, a);
    }
}

static void c_printf(benchmark::State& state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        printf("Hello, world! %.*f %x\r", 2, b, a);
    }
}

static void cppu_cout(benchmark::State& state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        cppu::fmt::print("Hello, world! {:.{}} {:x}\r", b, 2, a);
    }
}
// BENCHMARK(cppu_cout)->Iterations(10000000000);
BENCHMARK(cppu_cout);
BENCHMARK(c_printf);
BENCHMARK(cpp_fmt);
BENCHMARK(stl_cout);

BENCHMARK_MAIN();