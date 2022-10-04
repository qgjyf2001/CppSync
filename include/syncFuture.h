#ifndef SYNCFUTURE_H
#define SYNCFUTURE_H

#include <functional>
#include <future>
#include <vector>

#define THREADPOOL_SIZE 4

namespace cppSync {

template <typename T>
class promise {
public:
    promise()  {
      callback.push_back([this]() { finished = true; });
      future_ = m_promise.get_future();
    }
    promise(promise&& rhs) : m_promise(std::move(rhs.m_promise)) {
    }
    void set_value(T value) {
        m_promise.set_value(value);
        std::lock_guard<std::mutex> lck(mutex);
        for (auto &callback_ : callback)
          callback_();
    }
    std::shared_future<T> get_future() { return future_; }

    template <typename F, typename... Args>
    bool add_callback(F _callback, Args... args) {
      std::lock_guard<std::mutex> lck(mutex);
      if (finished) {
        return false;
      } else {
        callback.push_back(std::bind(_callback, args...));
      }
      return true;
    }
    template <typename F, typename... Args> auto then(F f, Args... args) {
      using type = decltype(f(std::forward<Args>(args)...));
      auto promise_ = std::make_shared<promise<type>>();
      auto callback = [&](std::shared_ptr<promise<type>> promise__) {
        if constexpr (std::is_same<void, type>::value) {
          f(std::forward<Args>(args)...);
          promise__->set_value();
        } else {
          auto ret = f(std::forward<Args>(args)...);
          promise__->set_value(ret);
        }
      };
      if (!add_callback(callback, promise_)) {
        callback(promise_);
      }
      return promise_;
    }

private:
  std::vector<std::function<void()>> callback;
  std::mutex mutex;
  bool finished = false;
  std::shared_future<T> future_;
  std::promise<T> m_promise;
};

template <> class promise<void> {
public:
  promise() {
    callback.push_back([this]() { finished = true; });
    future_ = m_promise.get_future();
  }
  promise(promise &&rhs) : m_promise(std::move(rhs.m_promise)) {}
  void set_value() {
    m_promise.set_value();
    std::lock_guard<std::mutex> lck(mutex);
    for (auto &callback_ : callback)
      callback_();
  }
  std::shared_future<void> get_future() { return future_; }

  template <typename F, typename... Args>
  bool add_callback(F _callback, Args... args) {
    std::lock_guard<std::mutex> lck(mutex);
    if (finished) {
      return false;
    } else {
      callback.push_back(std::bind(_callback, args...));
    }
    return true;
  }

  template <typename F, typename... Args> auto then(F f, Args... args) {
    using type = decltype(f(std::forward<Args>(args)...));
    auto promise_ = std::make_shared<promise<type>>();
    auto callback = [&](std::shared_ptr<promise<type>> promise__) {
      if constexpr (std::is_same<void, type>::value) {
        f(std::forward<Args>(args)...);
        promise__->set_value();
      } else {
        auto ret = f(std::forward<Args>(args)...);
        promise__->set_value(ret);
      }
    };
    if (!add_callback(callback, promise_)) {
      callback(promise_);
    }
    return promise_;
  }

private:
  std::vector<std::function<void()>> callback;
  std::mutex mutex;
  bool finished = false;
  std::shared_future<void> future_;
  std::promise<void> m_promise;
};
}

#endif