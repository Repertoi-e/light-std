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
    thread::Thread t1(thread_ids, 0);
    t1.join();
    thread::Thread t2(thread_ids, 0);
    t2.join();
    thread::Thread t3(thread_ids, 0);
    t3.join();
    For(range(45)) io::cout.write_codepoint(' ');
}

thread_local static s32 g_LocalVar;
static void thread_tls(void *) { g_LocalVar = 2; }

TEST(thread_local_storage) {
    g_LocalVar = 1;

    thread::Thread t1(thread_tls, 0);
    t1.join();

    assert_eq(g_LocalVar, 1);
}

static thread::Mutex g_Mutex;
static s32 g_Count = 0;

static void thread_lock(void *) {
    For(range(10000)) {
        thread::Scoped_Lock<thread::Mutex> _(g_Mutex);
        ++g_Count;
    }
}

TEST(mutex_lock) {
    g_Count = 0;

    Dynamic_Array<thread::Thread *> threads;
    For(range(100)) { threads.add(new thread::Thread(thread_lock, 0)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}

static thread::Fast_Mutex g_FastMutex;

static void thread_lock2(void *) {
    For(range(10000)) {
        thread::Scoped_Lock<thread::Fast_Mutex> _(g_FastMutex);
        ++g_Count;
    }
}

TEST(fast_mutex_lock) {
    g_Count = 0;

    Dynamic_Array<thread::Thread *> threads;
    For(range(100)) { threads.add(new thread::Thread(thread_lock2, 0)); }

    For(threads) {
        it->join();
        delete it;
    }

    assert_eq(g_Count, 100 * 10000);
}

static thread::Condition_Variable g_Cond;

static void thread_condition_notifier(void *) {
    thread::Scoped_Lock<thread::Mutex> _(g_Mutex);
    --g_Count;
    g_Cond.notify_all();
}

static void thread_condition_waiter(void *) {
    thread::Scoped_Lock<thread::Mutex> _(g_Mutex);
    while (g_Count > 0) {
        g_Cond.wait(g_Mutex);
    }

    assert_eq(g_Count, 0);
}

TEST(condition_variable) {
    g_Count = 40;

    thread::Thread t1(thread_condition_waiter, 0);

    // These will decrease gCount by 1 when they finish)
    Dynamic_Array<thread::Thread *> threads;
    For(range(g_Count)) { threads.add(new thread::Thread(thread_condition_notifier, 0)); }

    t1.join();

    For(threads) {
        it->join();
        delete it;
    }
}

TEST(implicit_context) {
    auto *old = CONTEXT_ALLOC.Function;

    PUSH_CONTEXT(Allocator, OS_ALLOC) {
        thread::Thread t1(
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
            0);
        t1.join();
    }

    assert_eq((void *) CONTEXT_ALLOC.Function, (void *) old);
}

#endif