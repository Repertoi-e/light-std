#include <lstd/thread.hpp>

#include "../test.hpp"

#define DO_THREAD_TESTS
#if defined DO_THREAD_TESTS

TEST(hardware_concurrency) {
    io::cout.write_fmt("\n\t\tNumber of processor cores: {}.\n", thread::get_hardware_concurrency());
    For(range(45)) io::cout.write_codepoint(' ');
}

static void thread_ids(void *) { io::cout.write_fmt("\t\tMy thread id is {}.\n", thread::this_thread::get_id()); }

TEST(ids) {
    io::cout.write_fmt("\n\t\tMain thread's id is {}.\n", thread::this_thread::get_id());
    thread::thread t1(thread_ids, null);
    t1.join();
    thread::thread t2(thread_ids, null);
    t2.join();
    thread::thread t3(thread_ids, null);
    t3.join();
    For(range(45)) io::cout.write_codepoint(' ');
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
        thread::scoped_lock<thread::mutex> _(g_Mutex);
        ++g_Count;
    }
}

TEST(mutex_lock) {
    g_Count = 0;

    dynamic_array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock, null)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}

// This causes crashes
#if 0 
static thread::fast_mutex g_FastMutex;

static void thread_lock2(void *) {
    For(range(10000)) {
        thread::scoped_lock<thread::fast_mutex> _(g_FastMutex);
        ++g_Count;
    }
}

TEST(fast_mutex_lock) {
    g_Count = 0;

    dynamic_array<thread::thread *> threads;
    For(range(100)) { threads.append(new thread::thread(thread_lock2, null)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}
#endif

static thread::condition_variable g_Cond;

static void thread_condition_notifier(void *) {
    thread::scoped_lock<thread::mutex> _(g_Mutex);
    --g_Count;
    g_Cond.notify_all();
}

static void thread_condition_waiter(void *) {
    thread::scoped_lock<thread::mutex> _(g_Mutex);
    while (g_Count > 0) {
        g_Cond.wait(g_Mutex);
    }

    assert_eq(g_Count, 0);
}

TEST(condition_variable) {
    g_Count = 40;

    thread::thread t1(thread_condition_waiter, null);

    // These will decrease gCount by 1 when they finish)
    dynamic_array<thread::thread *> threads;
    For(range(g_Count)) { threads.append(new thread::thread(thread_condition_notifier, null)); }

    t1.join();

    For(threads) {
        it->join();
        delete it;
    }
}

TEST(implicit_context) {
    auto *old = CONTEXT_ALLOC.Function;

    PUSH_CONTEXT(Allocator, OS_ALLOC) {
        thread::thread t1(
            [](void *) {
                assert_eq((void *) CONTEXT_ALLOC.Function, (void *) OS_ALLOC.Function);

                []() {
                    PUSH_CONTEXT(Allocator, TEMPORARY_ALLOC) {
                        assert_eq((void *) CONTEXT_ALLOC.Function, (void *) TEMPORARY_ALLOC.Function);
                        return;
                    }
                }();

                assert_eq((void *) CONTEXT_ALLOC.Function, (void *) OS_ALLOC.Function);
            },
            null);
        t1.join();
    }

    assert_eq((void *) CONTEXT_ALLOC.Function, (void *) old);
}

#endif