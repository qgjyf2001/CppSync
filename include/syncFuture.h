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
    }
    void set_value(T value) {
        m_promise.set_value(value);
        std::lock_guard<std::mutex> lck(mutex);
        callback();
    }
    std::future<T> get_future() {
        return m_promise.get_future();
    }

    template <typename F,typename... Args>
    bool set_callback(F _callback,Args... args) {
        std::lock_guard<std::mutex> lck(mutex);
        if (finished) {
            return false;
        } else {
            callback=std::bind(_callback,args...);
        }
        return true;
    }
    template <typename F,typename... Args>
    auto then(F f,Args... args) {
        using type = decltype(f(std::forward<Args>(args)...));
        auto promise_ = std::make_shared<promise<type>>();
        auto callback=[&](std::shared_ptr<promise<type>> promise__){
            if constexpr(std::is_same<void,type>::value) {
                f(std::forward<Args>(args)...);
                promise__->set_value();
            } else {
                auto ret = f(std::forward<Args>(args)...);
                promise__->set_value(ret);
            }
        };
        if (!set_callback(callback,promise_)) {
            callback(promise_);
        }
        return promise_;
    }
private:
    std::function<void()> callback;
    std::mutex mutex;
    bool finished=false;
    std::promise<T> m_promise;
};

template <>
class promise<void> {
public:
    promise()  {
        callback=[this](){
            finished=true;
        };
    }
    promise(promise&& rhs) : m_promise(std::move(rhs.m_promise)) {
    }
    void set_value() {
        m_promise.set_value();
        std::lock_guard<std::mutex> lck(mutex);
        callback();
    }
    std::future<void> get_future() {
        return m_promise.get_future();
    }
    
    template <typename F,typename... Args>
    bool set_callback(F _callback,Args... args) {
        std::lock_guard<std::mutex> lck(mutex);
        if (finished) {
            return false;
        } else {
            callback=std::bind(_callback,args...);
        }
        return true;
    }

    template <typename F,typename... Args>
    auto then(F f,Args... args) {
        using type = decltype(f(std::forward<Args>(args)...));
        auto promise_ = std::make_shared<promise<type>>();
        auto callback=[&](std::shared_ptr<promise<type>> promise__){
            if constexpr(std::is_same<void,type>::value) {
                f(std::forward<Args>(args)...);
                promise__->set_value();
            } else {
                auto ret = f(std::forward<Args>(args)...);
                promise__->set_value(ret);
            }
        };
        if (!set_callback(callback,promise_)) {
            callback(promise_);
        }
        return promise_;
    }
private:
    std::function<void()> callback;
    std::mutex mutex;
    bool finished=false;
    std::promise<void> m_promise;
};

}

#endif