#include <iostream>

#include <lstd/basic.h>

int main() {
    byte one[20]{};
    byte two[20]{};

    std::cout << compare_memory(one + 1, two, 15) << '\n';
    std::cout << compare_memory_constexpr(one + 1, two, 15) << '\n';

    one[10] = 1;
    std::cout << compare_memory(one, two + 2, 15) << '\n';
    std::cout << compare_memory_constexpr(one, two + 2, 15) << '\n';

    two[10] = 1;
    one[11] = 1;
    std::cout << compare_memory(one + 1, two + 3, 15) << '\n';
    std::cout << compare_memory_constexpr(one + 1, two + 3, 15) << '\n';

    byte buffer[1000];
    byte buffer_zero[1000]{};

    std::cout << compare_memory(buffer, buffer_zero, 1000) << '\n';
    std::cout << compare_memory_constexpr(buffer, buffer_zero, 1000) << '\n';

    fill_memory(buffer, 0, 1000);

    std::cout << compare_memory(buffer, buffer_zero, 1000) << '\n';

    init_temporary_allocator(2000);
    PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
        auto *my_buffer = new byte[1500];
        delete my_buffer;

        Context.TemporaryAlloc.free_all();
    }

    auto *buffer_number_ten_trillion_in_this_main_function = new (Context.TemporaryAlloc) byte[200];
    auto *buffer_im_running_out_of_names = new (Malloc) byte[20000];

    release_temporary_allocator();
}