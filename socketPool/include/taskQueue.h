#pragma once
#include<iostream>
#include<string>
#include<queue>
#include<mutex>

#include<memory>
using namespace std;

struct Task{
    Task(){
        func = nullptr;
        arg = nullptr;
    }
    void* (*func)(void*);
    void *arg;
};

typedef void* (*cb)(void*);//回调函数

class taskQueue{
public:
    taskQueue();
    ~taskQueue();

    void addTask(cb,void*);
    void addTask(shared_ptr<Task> task);

    shared_ptr<Task> takeTask();

    size_t getTaskNum(){
        return m_queue.size(); 
    }

private:
    mutex m_mutex;
    std::queue<std::shared_ptr<Task>> m_queue;
};