#pragma once

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>

void FutexWait(std::atomic<int> *value, int expected_value) {
    syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expected_value, nullptr, nullptr, 0);
}

void FutexWake(std::atomic<int> *value, int count) {
    syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

int Cmpxchg(std::atomic<int> *val, int expected, int desired) {
    std::atomic_compare_exchange_strong(val, &expected, desired);
    return expected;
}

class Mutex {
public:
    Mutex() : val_(0) {
    }

    void Lock() {
        int c;
        if ((c = Cmpxchg(&val_, 0, 1)) != 0) {
            do {
                if (c == 2 || Cmpxchg(&val_, 1, 2) != 0) {
                    FutexWait(&val_, 2);
                }
            } while ((c = Cmpxchg(&val_, 0, 2)) != 0);
        }
    }

    void Unlock() {
        if (val_-- != 1) {
            val_ = 0;
            FutexWake(&val_, 1);
        }
    }

private:
    std::atomic<int> val_;
};
