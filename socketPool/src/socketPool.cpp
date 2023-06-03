#include"socketPool.h"
#include<iostream>
using namespace std;

socketPool::socketPool(std::string ip,unsigned int port,int capacity)
    :m_ip(ip),
    m_port(port),
    m_capacity(capacity),
    m_curNum(capacity)
{
    createConnect();
}

void socketPool::createConnect(){
    cout<<"当前连接池数量"<<m_sockque.size()<<endl;
    cout<<"容量"<<m_curNum<<endl;
    if(m_sockque.size() > m_curNum){
        return;
    }
    std::unique_ptr<TcpSocket> tcpsock = std::make_unique<TcpSocket>();
    int ret = tcpsock->connToHost(m_ip,m_port);
    if(ret == 0){
        m_sockque.emplace(std::move(tcpsock));
    }else{
        perror("connect error");
    }
    createConnect();    
}

inline bool socketPool::isEmpty(){
    return m_sockque.empty();
}

void socketPool::putConnect(std::unique_ptr<TcpSocket>tcp,bool isValid = true){
    std::unique_lock<std::mutex> lock(m_mutex);//获取锁
    if(isValid){
        m_sockque.emplace(std::move(tcp));
        cout<<"连接可用"<<endl;
    }else{
        perror("连接不可用");
        tcp->disConnect();
        m_curNum = m_sockque.size() + 1;//创建一个新的连接
        createConnect();
    }
    cout<<"connect size"<<m_sockque.size()<<endl;
}


std::unique_ptr<TcpSocket> socketPool::getConnect(){
    if(m_sockque.size() == 0){
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    unique_ptr<TcpSocket> socket = std::move(m_sockque.front());
    m_sockque.pop();
    cout<<"连接数量"<<m_sockque.size()<<endl;
    return std::move(socket);
}

 socketPool::~socketPool(){
    unique_lock<std::mutex> lock(m_mutex);
    while(!m_sockque.empty()){
        std::unique_ptr<TcpSocket> ptr = std::move(m_sockque.front());
        ptr->disConnect();
        m_sockque.pop();
    }
 }