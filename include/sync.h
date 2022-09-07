#ifndef SYNC_H
#define SYNC_H
#include "syncFuture.h"
namespace cppSync {
    template <typename T,typename... Args>
    auto async(T f,Args... args) {
        using type = decltype(f(std::forward<Args>(args)...));
        std::unique_ptr<promise<type>> promise_ = std::make_unique<promise<type>>();
        std::thread thread([&]() {
            if constexpr(std::is_same<void,type>::value) {
                f(std::forward<Args>(args)...);
                promise_->set_value();
            } else {
                auto ret = f(std::forward<Args>(args)...);
                promise_->set_value(ret);
            }
        });
        thread.detach();
        return promise_;
    }
}
#endif