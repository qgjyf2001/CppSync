#ifndef SYNCFUTURE_H
#define SYNCFUTURE_H

#include <future>

#define THREADPOOL_SIZE 4

namespace cppSync {

template <typename T>
class promise {
public:
    promise()  {
        callback=[this](){
            finished=true;
        };
    }
    promise(promise&& rhs) : m_promise(std::move(rhs.m_promise)) {
        m_promise.set_value(123);
    }
    void set_value(T value) {
        m_promise.set_value(value);
        std::lock_guard<std::mutex> lck(mutex);
        callback();
    }
    std::future<T> get_future() {
        return m_promise.get_future();
    }
    
    bool set_callback(std::function<void()> _callback) {
        std::lock_guard<std::mutex> lck(mutex);
        if (finished) {
            return false;
        } else {
            callback=_callback;
        }
        return true;
    }
private:
    std::function<void()> callback;
    std::mutex mutex;
    bool finished=false;
    std::promise<T> m_promise;
};

}

#endif