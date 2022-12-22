#include "../test.h"

import lstd.os;
import lstd.thread;
import lstd.atomic;

TEST(hardware_concurrency) {
    print("\n\t\tHardware concurrency: {}.\n", os_get_hardware_concurrency());
    For(range(45)) print(" ");
}

file_scope void thread_ids(void *) { print("\t\tMy thread id is {}.\n", Context.ThreadID); }

TEST(ids) {
    print("\n\t\tMain thread's id is {}.\n", Context.ThreadID);

    thread t1 = create_and_launch_thread(thread_ids);
    wait(t1);

    thread t2 = create_and_launch_thread(thread_ids);
    wait(t2);

    thread t3 = create_and_launch_thread(thread_ids);
    wait(t3);

    For(range(45)) print(" ");
}

thread_local file_scope s32 TLSVar;
file_scope void thread_tls(void *) { TLSVar = 2; }

TEST(thread_local_storage) {
    TLSVar = 1;

    thread t1 = create_and_launch_thread(thread_tls);
    wait(t1);

    assert_eq(TLSVar, 1);
}

file_scope mutex Mutex;
file_scope s32 Count = 0;

file_scope void thread_lock_free(void *) {
    For(range(10000)) {
        atomic_inc(&Count);
    }
}

TEST(lock_free) {
    Count = 0;

    array<thread> threads;
    reserve(threads);
    defer(free(threads.Data));

    For(range(100)) {
        add(threads, create_and_launch_thread(thread_lock_free));
    }

    For(threads) {
        wait(it);
    }

    assert_eq(Count, 100 * 10000);
}

file_scope void thread_lock(void *) {
    For(range(10000)) {
        lock(&Mutex);
        ++Count;
        unlock(&Mutex);
    }
}

TEST(mutex_lock) {
    Count = 0;

    Mutex = create_mutex();

    array<thread> threads;
	reserve(threads);
    defer(free(threads.Data));

    For(range(100)) {
        add(threads, create_and_launch_thread(thread_lock));
    }

    For(threads) {
        wait(it);
    }

    assert_eq(Count, 100 * 10000);
}

file_scope fast_mutex FastMutex;

file_scope void thread_lock2(void *) {
    For(range(10000)) {
        lock(&FastMutex);
        ++Count;
        unlock(&FastMutex);
    }
}

TEST(fast_mutex_lock) {
    Count = 0;

    array<thread> threads;
    reserve(threads);
	defer(free(threads.Data));
    
    For(range(100)) {
        add(threads, create_and_launch_thread(thread_lock2));
    }

    For(threads) {
        wait(it);
    }

    assert_eq(Count, 100 * 10000);
}

file_scope condition_variable Cond;

file_scope void thread_condition_notifier(void *) {
    lock(&Mutex);
    --Count;
    notify_all(&Cond);
    unlock(&Mutex);
}

file_scope void thread_condition_waiter(void *) {
    lock(&Mutex);
    while (Count > 0) {
        wait(&Cond, &Mutex);
    }
    assert_eq(Count, 0);
    unlock(&Mutex);
}

TEST(condition_variable) {
    Count = 40;

    Cond = create_condition_variable();
    defer(free_condition_variable(&Cond));

    thread t1 = create_and_launch_thread(thread_condition_waiter);

    // These will decrease Count by 1 when they finish)
    array<thread> threads;
    reserve(threads);
    defer(free(threads.Data));

    For(range(Count)) {
        add(threads, create_and_launch_thread(thread_condition_notifier));
    }

    wait(t1);

    For(threads) {
        wait(it);
    }

    free_mutex(&Mutex);
}

TEST(context) {
    auto *old = Context.Alloc.Function;

    allocator differentAlloc = {};
    PUSH_ALLOC(differentAlloc) {
        auto threadFunction = [&](void *) {
            assert_eq((void *) Context.Alloc.Function, (void *) differentAlloc.Function);
            []() {
                PUSH_ALLOC(TemporaryAllocator) {
                    assert_eq((void *) Context.Alloc.Function, (void *) TemporaryAllocator.Function);
                    return;
                }
            }();
            assert_eq((void *) Context.Alloc.Function, (void *) differentAlloc.Function);
        };

        thread t1 = create_and_launch_thread(&threadFunction);
        wait(t1);
    }
    assert_eq((void *) Context.Alloc.Function, (void *) old);
}
