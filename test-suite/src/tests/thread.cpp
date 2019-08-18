#include "../test.h"

#define DO_THREAD_TESTS
#if defined DO_THREAD_TESTS

TEST(hardware_concurrency) {
    fmt::print("\n\t\tNumber of processor cores: {}.\n", thread::get_hardware_concurrency());
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

thread_local static s32 g_LocalVar;
static void thread_tls(void *) { g_LocalVar = 2; }

TEST(thread_local_storage) {
    g_LocalVar = 1;

    thread::thread t1(thread_tls, null);
    t1.join();

    assert_eq(g_LocalVar, 1);
}

static thread::mutex g_Mutex;
static s32 g_Count = 0;

static void thread_lock(void *) {
    For(range(10000)) {
        thread::scoped_lock<thread::mutex> _(&g_Mutex);
        ++g_Count;
    }
}

TEST(mutex_lock) {
    g_Count = 0;

    array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock, null)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}

// This causes crashes
static thread::fast_mutex g_FastMutex;

static void thread_lock2(void *) {
    For(range(10000)) {
        thread::scoped_lock<thread::fast_mutex> _(&g_FastMutex);
        ++g_Count;
    }
}

TEST(fast_mutex_lock) {
    g_Count = 0;

    array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock2, null)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}

static thread::condition_variable g_Cond;

static void thread_condition_notifier(void *) {
    thread::scoped_lock<thread::mutex> _(&g_Mutex);
    --g_Count;
    g_Cond.notify_all();
}

static void thread_condition_waiter(void *) {
    thread::scoped_lock<thread::mutex> _(&g_Mutex);
    while (g_Count > 0) {
        g_Cond.wait(g_Mutex);
    }

    assert_eq(g_Count, 0);
}

TEST(condition_variable) {
    g_Count = 40;

    thread::thread t1(thread_condition_waiter, null);

    // These will decrease gCount by 1 when they finish)
    array<thread::thread *> threads;
    For(range(g_Count)) { threads.append(new thread::thread(thread_condition_notifier, null)); }

    t1.join();

    For(threads) {
        it->join();
        delete it;
    }
}

TEST(context) {
    auto *old = Context.Alloc.Function;

    auto osAlloc = allocator{os_allocator, null};

    PUSH_CONTEXT(Alloc, osAlloc) {
        thread::thread t1(
            [&](void *) {
                assert_eq((void *) Context.Alloc.Function, (void *) osAlloc.Function);

                []() {
                    PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
                        assert_eq((void *) Context.Alloc.Function, (void *) Context.TemporaryAlloc.Function);
                        return;
                    }
                }();

                assert_eq((void *) Context.Alloc.Function, (void *) osAlloc.Function);
            },
            null);
        t1.join();
    }

    assert_eq((void *) Context.Alloc.Function, (void *) old);
}
#endif
