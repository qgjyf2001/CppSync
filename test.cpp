// #define BOOST_THREAD_PROVIDES_FUTURE
// #define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
// #define BOOST_THREAD_PROVIDES_FUTURE_UNWRAP
#include <coroutine>
#include <iostream>
#include <future>
#include <unistd.h>

#include "./include/syncTask.h"
#include "./include/sync.h"

// #include <boost/thread/future.hpp>
cppSync::task test1() {
    for (int i=0;i<8;i++) {
        cppSync::promise<int> p;
        auto t = cppSync::async([&](){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 1234;
        });
        std::cout<<"await:"<<__LINE__<<" thread_id:"<<std::this_thread::get_id()<<std::endl;
        co_await cppSync::awaiter(t.get());
        std::cout<<"done:"<<__LINE__<<" thread_id:"<<std::this_thread::get_id()<<std::endl;
    }
}

cppSync::task test2() {
    for (int i=0;i<4;i++) {
        cppSync::promise<int> p;
        auto t = cppSync::async([&](){
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return 1234;
        });
        std::cout<<"await:"<<__LINE__<<" thread_id:"<<std::this_thread::get_id()<<std::endl;
        co_await cppSync::awaiter(t.get());
        std::cout<<"done:"<<__LINE__<<" thread_id:"<<std::this_thread::get_id()<<std::endl;
    }
}

int main() {
    test1();
    test2();
    while (true) {
        sleep(1);
    }
}