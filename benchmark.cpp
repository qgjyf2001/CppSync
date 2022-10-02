#include <coroutine>
#include <iostream>
#include <future>

#include "./include/graph.h"

class Node {
public:
    std::vector<std::shared_ptr<Node>> depends;
    Node(std::function<void()> func):func(func) {

    }
    std::function<void()> func;
    void depend(std::shared_ptr<Node> node) {
        depends.emplace_back(node);
    }
};
class graph {
public:
    std::vector<std::shared_ptr<Node>> nodes;
    std::shared_ptr<Node> addNode(std::function<void()> func) {
        auto node=std::make_shared<Node>(func);
        nodes.push_back(node);
        return node;
    }
    std::shared_ptr<Node> genDepends(std::function<void()> func,float connectedRate=0.1) {
        int index=0;
        int nodeSize=nodes.size();
        std::vector<std::shared_ptr<Node>> targetNodes;
        for (auto &node:nodes) {
            index+=1;
            bool depended=false;
            for (int i=index;i<nodeSize;i++) {
                if (rand()<RAND_MAX*connectedRate) {
                    nodes[i]->depend(node);
                    depended=true;
                }
            }
            if (!depended) {
                targetNodes.push_back(node);
            }
        }
        auto node=std::make_shared<Node>(func);
        for (auto& target:targetNodes) {
            node->depend(target);
        }
        nodes.push_back(node);
        return node;
    }
};

int main() {
    auto graph_=std::make_shared<graph>();
    const int nodeSize=10000;
    std::atomic<int> count;
    // gen graph
    for (int i=0;i<nodeSize;i++) {
        graph_->addNode([index=i,&count](){
            //std::cout<<"runnig node "+std::to_string(index)<<std::endl;
            for (int i=0;i<100000;i++) {
                count++;
            }
        });
    }
    decltype(std::chrono::steady_clock::now()) start;  
    auto final=graph_->genDepends([&start,&count]() {
        auto end=std::chrono::steady_clock::now();
        std::chrono::duration<double,std::milli> duration=end-start;
        int result=count;
        std::cout<<"graph execution done, count:"<<result<<", timeout:"<<duration.count()<<"ms"<<std::endl;
    });

    // build async graph
    auto asyncGraph=std::make_shared<cppSync::graph>();
    std::unordered_map<std::shared_ptr<Node>,std::shared_ptr<cppSync::graph::Node>> nodeMap;
    int number=0;
    for (auto &node:graph_->nodes) {
        auto graphNode=asyncGraph->add_node(node->func)->name("node"+std::to_string(number++));
        nodeMap[node]=graphNode;
    }
    for (auto &node:graph_->nodes) {
        for (auto &depend:node->depends) {
            nodeMap[node]->depend(nodeMap[depend]);
        }
    }

    std::cout<<"graph build done"<<std::endl;
    //run dag
    auto &finalNode=nodeMap[final];
    start=std::chrono::steady_clock::now();
    (*finalNode)().get_promise()->get_future().get();
    return 0;
}