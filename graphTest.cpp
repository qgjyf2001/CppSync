#include <coroutine>
#include <iostream>
#include <future>
#include <unistd.h>

#include "./include/graph.h"

int main() {
    
    auto graph=std::make_shared<cppSync::graph>();
    auto node1=graph->add_node([](){
        std::cout<<"node1"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    auto node2=graph->add_node([](){
        std::cout<<"node2"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    auto node3=graph->add_node([](){
        std::cout<<"node3"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    auto node4=graph->add_node([](){
        std::cout<<"node4"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    auto node5=graph->add_node([](){
        std::cout<<"node5"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    auto node6=graph->add_node([](){
        std::cout<<"node6"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    node1->name("node1");
    node2->name("node2");
    node3->name("node3");
    node4->name("node4");
    node5->name("node5");
    node6->name("node6");
    
    node3->depend(node2);
    node3->depend(node1);
    node4->depend(node1);
    node4->depend(node3);
    node5->depend(node2);
    node5->depend(node3);
    node6->depend(node5);
    node6->depend(node4);
    (*node4)().get_promise()->then([](){
        std::cout<<"node4 done"<<std::endl;
    });
    (*node6)().get_promise()->then([](){
        std::cout<<"graph execution done"<<std::endl;
    });
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}