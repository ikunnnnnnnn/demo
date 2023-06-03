#pragma once
#include<iostream>
using namespace std;
static const int TIMEOUT = 1000;
class TcpSocket{
	public:
		enum ErrorType {
			PARAM_ERROR = 3001,
			TIMEOUT_ERROR,
			PEERCLOSE_ERROR,
			MALLOC_ERROR
		};
		TcpSocket();
		TcpSocket(int);
		~TcpSocket();


		int connToHost(const string,unsigned short,int timeout = TIMEOUT);
		int sendMsg(string msg,int timeout = TIMEOUT);

		string recvMsg(int timeout = TIMEOUT);

		void disConnect();

	private:
		int setNonBlock(int);

		int setBlock(int);

		int readTimeout(unsigned int wait_seconds);

		int writeTimeout(unsigned int wait_seconds);

		int connectTimeout(struct sockaddr_in* addr,unsigned int wait_second);

		int readn(void* buf,size_t len);

		int writen(const void* buf,size_t len);
	private:
		int m_socket;
};
