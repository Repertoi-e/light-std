#include <benchmark/benchmark.h>

#include <iomanip>
#include <iostream>

#include <lstd/io.h>
#include <lstd/io/fmt.h>

static void stl_cout(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        // What is this, really
        // "+1.2340000000:002a:+3.1300000000:str:3e8X"
        std::cout << std::fixed << std::setprecision(10) << 1.234 << ':' << std::setfill('0') << std::setw(4) << 42
                  << ':' << std::setprecision(2) << std::showpos << 3.13 << ':' << "str" << ':' << std::hex << 1000
                  << 'X' << '\r';
    }
}

static void c_printf(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        // "1.2340000000:0042:+3.13:str:0x3e8:X:%"
        printf("%.10f:%04d:%+.2f:%s:%x:%c:%%\r", 1.234, 42, 3.13, "str", 1000, 'X');
    }
}

static void lstd_fmt_cout(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        // "1.2340000000:0042:+3.13:str:0x3e8:X:%"
        fmt::print("{:0.10f}:{:04}:{:+g}:{}:{}:{:c}:%\r", 1.234, 42, 3.13, "str", (void *) 1000, 'X');
    }
}

BENCHMARK(stl_cout);
BENCHMARK(c_printf);
BENCHMARK(lstd_fmt_cout);  // ->Iterations(10000000000);

BENCHMARK_MAIN();