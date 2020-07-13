#include "../test.h"

#define DO_THREAD_TESTS  // XXX
#if defined DO_THREAD_TESTS

TEST(hardware_concurrency) {
    fmt::print("\n\t\tHardware concurrency: {}.\n", os_get_hardware_concurrency());
    For(range(45)) fmt::print(" ");
}

static void thread_ids(void *) { fmt::print("\t\tMy thread id is {}.\n", Context.ThreadID); }

TEST(ids) {
    fmt::print("\n\t\tMain thread's id is {}.\n", Context.ThreadID);
    thread::thread t1(thread_ids, null);
    t1.join();
    thread::thread t2(thread_ids, null);
    t2.join();
    thread::thread t3(thread_ids, null);
    t3.join();
    For(range(45)) fmt::print(" ");
}

thread_local static s32 TLSVar;
static void thread_tls(void *) { TLSVar = 2; }

TEST(thread_local_storage) {
    TLSVar = 1;

    thread::thread t1(thread_tls, null);
    t1.join();

    assert_eq(TLSVar, 1);
}

static thread::mutex Mutex;
static s32 Count = 0;

static void thread_lock(void *) {
    For(range(10000)) {
        thread::scoped_lock<thread::mutex> _(&Mutex);
        ++Count;
    }
}

TEST(mutex_lock) {
    Count = 0;

    array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock, null)); }

    For(threads) {
        it->join();
        free(it);
    }
    threads.release();

    assert_eq(Count, 100 * 10000);
}

static thread::fast_mutex FastMutex;

static void thread_lock2(void *) {
    For(range(10000)) {
        thread::scoped_lock<thread::fast_mutex> _(&FastMutex);
        ++Count;
    }
}

TEST(fast_mutex_lock) {
    Count = 0;

    array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock2, null)); }

    For(threads) {
        it->join();
        free(it);
    }
    threads.release();

    assert_eq(Count, 100 * 10000);
}

static thread::condition_variable Cond;

static void thread_condition_notifier(void *) {
    thread::scoped_lock<thread::mutex> _(&Mutex);
    --Count;
    Cond.notify_all();
}

static void thread_condition_waiter(void *) {
    thread::scoped_lock<thread::mutex> _(&Mutex);
    while (Count > 0) {
        Cond.wait(Mutex);
    }

    assert_eq(Count, 0);
}

TEST(condition_variable) {
    Count = 40;

    thread::thread t1(thread_condition_waiter, null);

    // These will decrease Count by 1 when they finish)
    array<thread::thread *> threads;
    For(range(Count)) { threads.append(new thread::thread(thread_condition_notifier, null)); }

    t1.join();

    For(threads) {
        it->join();
        free(it);
    }
    threads.release();
}

TEST(context) {
    auto *old = Context.Alloc.Function;

    auto differentAlloc = Context.TemporaryAlloc;
    WITH_CONTEXT_VAR(Alloc, differentAlloc) {
        auto threadFunction = [&](void *) {
            assert_eq((void *) Context.Alloc.Function, (void *) differentAlloc.Function);
            []() {
                WITH_CONTEXT_VAR(Alloc, Context.TemporaryAlloc) {
                    assert_eq((void *) Context.Alloc.Function, (void *) Context.TemporaryAlloc.Function);
                    return;
                }
            }();
            assert_eq((void *) Context.Alloc.Function, (void *) differentAlloc.Function);
        };

        thread::thread t1(&threadFunction, null);
        t1.join();
    }
    assert_eq((void *) Context.Alloc.Function, (void *) old);
}
#endif
