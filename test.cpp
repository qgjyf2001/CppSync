// #define BOOST_THREAD_PROVIDES_FUTURE
// #define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
// #define BOOST_THREAD_PROVIDES_FUTURE_UNWRAP
#include <coroutine>
#include <iostream>
#include <future>
#include <unistd.h>

#include "./include/graph.h"
#include "./include/sync.h"
#include "./include/syncTask.h"

// #include <boost/thread/future.hpp>
cppSync::task test1() {
  for (int i = 0; i < 4; i++) {
    auto t = cppSync::async([&]() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return 1234;
    });
    std::cout << "await:" << __LINE__
              << " thread_id:" << std::this_thread::get_id() << std::endl;
    int res = co_await cppSync::awaiter(t);
    std::cout << "done:" << __LINE__ << " res:" << res
              << " thread_id:" << std::this_thread::get_id() << std::endl;
  }
  co_return;
}

cppSync::task test2() {
  for (int i = 0; i < 8; i++) {
    auto t = cppSync::async(
        [&]() { std::this_thread::sleep_for(std::chrono::seconds(2)); });
    std::cout << "await:" << __LINE__
              << " thread_id:" << std::this_thread::get_id() << std::endl;
    co_await cppSync::awaiter(t);
    std::cout << "done:" << __LINE__
              << " thread_id:" << std::this_thread::get_id() << std::endl;
  }
  co_return;
}

int main() {
  auto task1 = test1()
                   .get_promise()
                   ->then([]() {
                     std::cout << "task1 joined" << std::endl;
                     std::this_thread::sleep_for(std::chrono::seconds(2));
                   })
                   ->then([]() {
                     std::cout << "task1 joined for 2 seconds" << std::endl;
                     std::this_thread::sleep_for(std::chrono::seconds(2));
                   })
                   ->then([]() {
                     std::cout << "task1 joined for 4 seconds" << std::endl;
                     std::this_thread::sleep_for(std::chrono::seconds(2));
                   })
                   ->then([]() {
                     std::cout << "task1 joined for 6 seconds" << std::endl;
                     std::this_thread::sleep_for(std::chrono::seconds(2));
                   })
                   ->then([]() {
                     std::cout << "task1 joined for 8 seconds" << std::endl;
                   });
  auto task2 = test2().get_promise()->then(
      []() { std::cout << "task2 joined" << std::endl; });
  task1->get_future().get();
  task2->get_future().get();
}