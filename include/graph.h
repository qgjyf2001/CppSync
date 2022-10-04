#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include <unordered_set>
#include <map>
#include "sync.h"
#include "syncTask.h"

namespace cppSync {

namespace noCoroutine {
    class graph {
public:
    class Node;
    class Node : public std::enable_shared_from_this<Node> {
        private:
            std::vector<std::shared_ptr<Node>> preceed;
            std::function<void()> func;
            graph* graph_;
            std::atomic<int> depends;
            std::shared_ptr<promise<void>> promise_=std::make_shared<promise<void>>();
        public:
        std::string name_;
        Node(graph* graph_,std::function<void()> func,std::string name="unknown"):graph_(graph_),func(func),name_(name) {
            graph_->startNodes.insert(this);
            graph_->runNodes++;
        }
        auto name(std::string name_) {
            this->name_ = name_;
            return shared_from_this();
        }
        void depend(std::shared_ptr<Node> node) {
            node->preceed.push_back(shared_from_this());
            depends++;
            if (graph_->startNodes.find(this)!=graph_->startNodes.end()) {
                graph_->startNodes.erase(this);
            }
        }
        auto get_promise() {
            return promise_;
        }
        void operator()() {
            func();
            promise_->set_value();
            int nodeLeft=--graph_->runNodes;
            if (nodeLeft==0) {
                graph_->promise_->set_value();
            }
            for (auto &node:preceed) {
                int depend_=--node->depends;
                if (depend_==0) {
                    threadPool::instance<THREADPOOL_SIZE>().addThread([node](){
                        (*node)();
                    });
                }
            }
            return;
        }
    };
private:
    std::unordered_set<Node*> startNodes;
    std::atomic<int> runNodes;
    std::shared_ptr<promise<void>> promise_=std::make_shared<promise<void>>();
public:
    template <typename F,typename... Args>
    std::shared_ptr<Node> add_node(F f,Args... args) {
        return std::make_shared<Node>(this,std::bind(f,args...));
    }
    auto run_graph() {
        for (auto &node:startNodes) {
            threadPool::instance<THREADPOOL_SIZE>().addThread([node](){
                        (*node)();
            });
        }
        return promise_;
    }
};
}

class graph {
public:
    class Node;
    class Node : public std::enable_shared_from_this<Node> {
        private:
            std::vector<std::shared_ptr<Node>> depends;
            std::function<void()> func;
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
private:
    std::map<Node*,std::shared_ptr<promise<void>>> mp;
public:
    template <typename F,typename... Args>
    std::shared_ptr<Node> add_node(F f,Args... args) {
        return std::make_shared<Node>(this,std::bind(f,args...));
    }
};

}

#endif