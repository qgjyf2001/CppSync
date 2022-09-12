## c++20协程实践

基于gcc-11尝试c++20协程，可使用指定大小的线程池对协程进行调度（THREADPOOL_SIZE宏），测试demo见test.cpp

基于协程编写的一套简单的图执行框架，测试demo见graphTest.cpp

## 运行环境

sudo apt-get install g++-11

curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -

sudo apt-get install bazel

## 编译方式

bash build.sh
