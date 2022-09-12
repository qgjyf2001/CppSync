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
                return task(std::coroutine_handle<promise_type>::from_promise(*this),promise_);
            }
            std::suspend_never initial_suspend() noexcept {
                return {};
            }
            std::suspend_never final_suspend() noexcept {
                return {};
            }
            void return_void() {
                promise_->set_value();
                return;
            }
            void unhandled_exception() {
                return;
            }
            std::shared_ptr<promise<void>> promise_ = std::make_shared<promise<void>>();
        };
        task(const std::coroutine_handle<promise_type> handler,std::shared_ptr<promise<void>> promise_):handler_(handler),promise_(promise_) {
        }
        auto get_promise() {
            return promise_;
        }
    private:
        std::coroutine_handle<promise_type> handler_;
        std::shared_ptr<promise<void>> promise_;
        
};

std::mutex& getSingletonMutex() {
    static std::mutex mutex;
    return mutex;
}

template <typename T>
struct  awaiter
{
    std::mutex mutex;
    awaiter(std::shared_ptr<promise<T>> promise_) : m_promise(promise_) {
    }
    bool await_ready() {
        return false;

    }
    void await_suspend(std::coroutine_handle<task::promise_type> handler) {
        bool res = m_promise->add_callback([handler](){
            std::lock_guard<std::mutex> lck(getSingletonMutex());
            threadPool::instance<THREADPOOL_SIZE>().addThread([handler](){
                handler.resume();
            });
        });
        if (!res) {
            handler.resume();
        }
    }
    T await_resume() {
        auto val=m_promise->get_future(); 
        return val.get();
    }
private:
    std::shared_ptr<promise<T>> m_promise;
};

}

#endif