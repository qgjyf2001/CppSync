#ifndef SYNC_H
#define SYNC_H
#include "syncFuture.h"
#include "threadPool.h"
namespace cppSync {
    template <typename T,typename... Args>
    auto async(T f,Args... args) {
        using type = decltype(f(std::forward<Args>(args)...));
        std::shared_ptr<promise<type>> promise_ =
            std::make_shared<promise<type>>();
        threadPool::instance<THREADPOOL_SIZE>().addThread([&]() {
          if constexpr (std::is_same<void, type>::value) {
            f(std::forward<Args>(args)...);
            promise_->set_value();
          } else {
            auto ret = f(std::forward<Args>(args)...);
            promise_->set_value(ret);
          }
        });
        return promise_;
    }
}
#endif