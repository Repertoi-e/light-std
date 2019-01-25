#include <cppu/io/io.hpp>
#include <cppu/format/fmt.hpp>

int main() {
    fmt::print("Hello, world!\n");
    io::cin.read();
}