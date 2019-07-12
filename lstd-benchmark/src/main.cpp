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
    For(state) { printf("Hello, world! %.*f %x\r", 2, b, a); }
}

static void lstd_cout_buffer_writer(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        stack_dynamic_buffer<512> formatBuffer;
        auto writer = io::buffer_writer<512>(&formatBuffer);

        auto f = fmt::format_context(&writer, "Hello, world! {:.{}f} {:x}\r", {fmt::make_fmt_args(b, 2, a)});

        auto handler = fmt::format_handler(&f);
        fmt::parse_format_string<false>(&f.Parse, &handler);
        writer.flush();

        io::cout.write(formatBuffer.Data, formatBuffer.ByteLength);
        io::cout.flush();
    }
}

static void lstd_cout_directly_to_cout(benchmark::State &state) {
    s32 a = 5000;
    f32 b = 1.622f;
    For(state) {
        auto f = fmt::format_context(&io::cout, "Hello, world! {:.{}f} {:x}\r", {fmt::make_fmt_args(b, 3, a)});

        auto handler = fmt::format_handler(&f);
        fmt::parse_format_string<false>(&f.Parse, &handler);
        io::cout.flush();
    }
}

BENCHMARK(stl_cout);
BENCHMARK(lstd_cout_buffer_writer);     // ->Iterations(10000000000);
BENCHMARK(lstd_cout_directly_to_cout);  // ->Iterations(10000000000);
BENCHMARK(c_printf);

BENCHMARK_MAIN();