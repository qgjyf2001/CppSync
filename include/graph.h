#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include <map>
#include "sync.h"
#include "syncTask.h"

namespace cppSync {

class graph {
    class Node;
    std::map<Node*,std::shared_ptr<promise<void>>> mp;
    class Node : public std::enable_shared_from_this<Node> {
        private:
            std::vector<std::shared_ptr<Node>> depends;
            std::function<void()> func;
            std::condition_variable consumer;
            graph* graph_;
        public:
        std::string name_;
        Node(graph* graph_,std::function<void()> func,std::string name="unknown"):graph_(graph_),func(func),name_(name) {
        }
        auto name(std::string name_) {
            this->name_ = name_;
            return shared_from_this();
        }
        void depend(std::shared_ptr<Node> node) {
            depends.push_back(node);
        }
        task operator()() {
            for (auto i:depends) {
                if (graph_->mp.find(i.get())==graph_->mp.end()) {
                    auto task_=(*i)();
                    graph_->mp[i.get()]=task_.get_promise();
                }
            }
            for (auto i:depends) {
                co_await awaiter(graph_->mp[i.get()]);
            }
            co_await awaiter(cppSync::async([](std::function<void()>&& func){
                func();
            },std::move(func)));
            co_return;
        }
    };
public:
    template <typename F,typename... Args>
    std::shared_ptr<Node> add_node(F f,Args... args) {
        return std::make_shared<Node>(this,std::bind(f,args...));
    }
};

}

#endif