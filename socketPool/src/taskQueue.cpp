#include"taskQueue.h"

taskQueue::taskQueue(){}
taskQueue::~taskQueue(){}

shared_ptr<Task> taskQueue::takeTask(){
    std::unique_lock<mutex>(m_mutex);
    if(m_queue.size() > 0){
        std::shared_ptr<Task> sp = m_queue.front();
        m_queue.pop();
        return std::move(sp);
    }
    return make_shared<Task>();
}

void taskQueue::addTask(shared_ptr<Task> task){
    
    std::unique_lock<mutex>(m_mutex);
    m_queue.emplace(task);
    
}

void taskQueue::addTask(cb c,void* args){
    
    std::unique_lock<mutex>(m_mutex);//获取锁
    std::shared_ptr<Task> sp = make_shared<Task>(c,args);
    m_queue.emplace(sp);
    
}



