#include <coroutine>
#include <cxxabi.h>
#include <future>
#include <iostream>

#include "./include/graph.h"

class Node {
public:
  std::vector<std::shared_ptr<Node>> depends;
  Node(std::function<void()> func) : func(func) {}
  std::function<void()> func;
  void depend(std::shared_ptr<Node> node) { depends.emplace_back(node); }
};
class graph {
public:
  std::vector<std::shared_ptr<Node>> nodes;
  std::shared_ptr<Node> addNode(std::function<void()> func) {
    auto node = std::make_shared<Node>(func);
    nodes.push_back(node);
    return node;
  }
  std::shared_ptr<Node> genDepends(std::function<void()> func,
                                   float connectedRate = 0.1) {
    int index = 0;
    int nodeSize = nodes.size();
    std::vector<std::shared_ptr<Node>> targetNodes;
    for (auto &node : nodes) {
      index += 1;
      bool depended = false;
      for (int i = index; i < nodeSize; i++) {
        if (rand() < RAND_MAX * connectedRate) {
          nodes[i]->depend(node);
          depended = true;
        }
      }
      if (!depended) {
        targetNodes.push_back(node);
      }
    }
    auto node = std::make_shared<Node>(func);
    for (auto &target : targetNodes) {
      node->depend(target);
    }
    nodes.push_back(node);
    return node;
  }
};

decltype(std::chrono::steady_clock::now()) start;

template <typename graphType>
void testGraph(std::shared_ptr<graph> graph_, std::shared_ptr<Node> final) {
  char buffer[64] = {0};
  size_t length = 64;
  __cxxabiv1::__cxa_demangle(typeid(graphType).name(), buffer, &length,
                             nullptr);
  std::string clazz = buffer;
  std::cout << "=======testing class " + clazz + "=======" << std::endl;
  // build run graph
  auto runGraph = std::make_shared<graphType>();
  using nodeType = std::remove_reference<decltype(*runGraph)>::type::Node;
  std::unordered_map<std::shared_ptr<Node>, std::shared_ptr<nodeType>> nodeMap;
  int number = 0;
  for (auto &node : graph_->nodes) {
    auto graphNode =
        runGraph->add_node(node->func)->name("node" + std::to_string(number++));
    nodeMap[node] = graphNode;
  }
  for (auto &node : graph_->nodes) {
    for (auto &depend : node->depends) {
      nodeMap[node]->depend(nodeMap[depend]);
    }
  }

  std::cout << "graph build done" << std::endl;
  // run dag
  auto &finalNode = nodeMap[final];
  start = std::chrono::steady_clock::now();

  if constexpr (std::is_same<graphType, cppSync::graph>::value) {
    (*finalNode)().get_promise()->get_future().get();
  } else if constexpr (std::is_same<graphType,
                                    cppSync::noCoroutine::graph>::value) {
    runGraph->run_graph();
    finalNode->get_promise()->get_future().get();
  } else {
    static_assert("typename not supported");
  }
}
int main() {
  auto graph_ = std::make_shared<graph>();
  const int nodeSize = 10000;
  std::atomic<int> count;
  // gen graph
  for (int i = 0; i < nodeSize; i++) {
    graph_->addNode([index = i, &count]() {
      // std::cout<<"runnig node "+std::to_string(index)<<std::endl;
      for (int i = 0; i < 100000; i++) {
        count++;
      }
    });
  }
  auto final = graph_->genDepends([&count]() {
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    int result = count;
    std::cout << "graph execution done, count:" << result
              << ", timeout:" << duration.count() << "ms" << std::endl;
  });

  testGraph<cppSync::graph>(graph_, final);
  testGraph<cppSync::noCoroutine::graph>(graph_, final);

  return 0;
}