#include <benchmark/benchmark.h>

#include <iomanip>
#include <iostream>

#include <lstd/io.h>
#include <lstd/io/fmt.h>

static void stl_cout(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        std::cout << "Hello, world! " << std::fixed << std::setprecision(2) << b << ' ' << std::hex << a << '\r';
    }
}

static void c_printf(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        printf(
            "Hello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! "
            "\rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \r%.*fHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! "
            "\rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \r "
            "%x\r",
            2, b, a);
    }
}

static void lstd_fmt_cout(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        fmt::print(
            "Hello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! "
            "\rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \r{:.{}f}Hello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! "
            "\rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, "
            "world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \rHello, world! \r "
            "{:x}\r",
            b, 2, a);
    }
}

BENCHMARK(stl_cout);
BENCHMARK(c_printf);
BENCHMARK(lstd_fmt_cout);  // ->Iterations(10000000000);

BENCHMARK_MAIN();