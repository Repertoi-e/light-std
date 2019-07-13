#include <lstd/basic.h>
#include <lstd/io.h>

#include <lstd/io/fmt.h>

int main() {
    Context.init_temporary_allocator(2_KiB);
    // PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
    //     auto *my_buffer = new byte[1500];
    //     delete[] my_buffer; // Does nothing, temp allocator supports only free all
    //
    //     Context.TemporaryAlloc.free_all();
    // }
    Context.release_temporary_allocator();

    string me = "123";
    string at = "1235";
    me.append("hi");

    // Note:
    // The memory leak below is happening because of a drawback of the user type policy
    // and because assignment in C++ doesn't call the destructor (the user type policy disallows assignment overloading)
    me = at;  // @Leak

    fmt::print(
        u8">> {!U} Hello, this is my formatter running! {!CORNFLOWER_BLUE} {:.^20f} {:=+010X}, {!} {!CORNFLOWER_BLUE;BG} "
        u8"{!tBRIGHT_BLACK;BU} {{}}, {:a} {:a} {:a} {:f} {!}\n",
        0x0p-1, 20, 0x1ffp10, 0x1.p0, 0xf.p-1, 0xa.bp10);

    table<const byte *, s32> hashMap;
    *hashMap["hello"] = 2;
    *hashMap["c++"] = 1111;
    *hashMap["hello"] = 3;

    hashMap.put("25982350238095", -235923859);

    for (auto [k, v] : hashMap) {
        fmt::print("{}, {}\n", *k, *v);
    }

    io::cin.read_line(&me);
    io::cin.ignore();
}