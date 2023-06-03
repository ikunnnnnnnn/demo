#pragma once
#include<iostream>
#include"tcpSocket.h"
#include<memory>
#include<mutex>
#include<queue>

class socketPool{
public:
    socketPool(string ip,unsigned int port,int capacity);
    ~socketPool();  
    std::unique_ptr<TcpSocket> getConnect();//获取连接
    void putConnect(std::unique_ptr<TcpSocket>tcp,bool isValid = true);
    inline bool isEmpty();//连接池是否为空
private:
    void createConnect();//创建连接池
private:
    std::mutex m_mutex;
    int m_capacity; //连接池容量
    std::string m_ip; //点分十进制格式ip地址
    unsigned int m_port;//端口号
    int m_curNum;//当前容量
    std::queue<std::unique_ptr<TcpSocket>> m_sockque;//连接池队列
};