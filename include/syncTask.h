#ifndef SYNCTASK_H
#define SYNCTASK_H

#include <coroutine>
#include <iostream>
#include <future>
#include "threadPool.h"
#include <unistd.h>

#include "syncFuture.h"

namespace cppSync {

class task {
    public:
        struct  promise_type 
        {
            task get_return_object() {
                return task(std::coroutine_handle<promise_type>::from_promise(*this));
            }
            std::suspend_never initial_suspend() noexcept {
                return {};
            }
            std::suspend_never final_suspend() noexcept {
                return {};
            }
            void return_void() {
                return;
            }
            void unhandled_exception() {
                return;
            }
        };
        task(const std::coroutine_handle<promise_type> handler):handler_(handler) {

        }
    private:
        std::coroutine_handle<promise_type> handler_;
        
};

std::mutex& getSingletonMutex() {
    static std::mutex mutex;
    return mutex;
}

template <typename T>
struct  awaiter
{
    std::mutex mutex;
    awaiter(promise<T> *promise_) : m_promise(promise_) {

    }
    bool await_ready() {
        return false;

    }
    void await_suspend(std::coroutine_handle<task::promise_type> handler) {
        bool res = m_promise->set_callback([handler](){
            std::lock_guard<std::mutex> lck(getSingletonMutex());
            threadPool::instance<THREADPOOL_SIZE>().addThread([handler](){
                handler.resume();
            });
        });
        if (!res) {
            handler.resume();
        }
    }
    void await_resume() {
        
    }
private:
    promise<T> *m_promise;
    
};

}

#endif