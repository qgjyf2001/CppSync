#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include "sync.h"
#include "syncTask.h"

namespace cppSync {

class graph {
    class Node {
        private:
            std::vector<std::shared_ptr<Node>> depends;
            std::function<void()> func;
            bool call_once=false;
            std::mutex mutex,call_mutex;
            bool done;
            std::condition_variable consumer;
        public:
        std::string name_;
        Node(std::function<void()> func,std::string name="unknown"):func(func),name_(name) {
        }
        Node* name(std::string name_) {
            this->name_ = name_;
            return this;
        }
        void depend(std::shared_ptr<Node> node) {
            depends.push_back(node);
        }
        task operator()() {
            bool called=false;
            {
                std::lock_guard<std::mutex> lck(call_mutex);
                if (!call_once) {
                    call_once=true;
                } else {
                    called=true;
                }
            }
            if (called) {
                std::unique_lock<std::mutex> lck(mutex);
                if (!done) {
                    consumer.wait(lck,[&](){
                        return done;
                    });
                }
                co_return;
            }

            std::vector<std::shared_ptr<promise<void>>> vec;
            for (auto i:depends) {
                vec.push_back((*i)().get_promise());
            }

            for (auto promise_:vec) {                
                co_await awaiter(promise_.get());
            }
            co_await awaiter(cppSync::async([](std::function<void()>&& func){
                func();
            },std::move(func)).get());
            done=true;
            consumer.notify_all();
            co_return;
        }
    };
public:
    template <typename F,typename... Args>
    std::shared_ptr<Node> add_node(F f,Args... args) {
        return std::make_shared<Node>(std::bind(f,args...));
    }
};

}

#endif