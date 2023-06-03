#include"tcpSocket.h"
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<stdio.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>
#include<sys/types.h>

TcpSocket::TcpSocket(){

}
TcpSocket::TcpSocket(int sock):m_socket(sock){
	
}

TcpSocket::~TcpSocket(){}

int TcpSocket::connToHost(const string ip,unsigned short port,int timeout){
	int ret = 0;
	if(port < 0 || port >= 65535 || timeout < 0){
		ret = PARAM_ERROR;
		return ret;
	}	
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket<0){
		ret = errno;
		return ret;
	}
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip.data());

	ret = connectTimeout(&servaddr,(unsigned int)timeout);
	if(ret < 0){
		if(ret == -1 || errno == ETIMEDOUT){
			ret = TIMEOUT_ERROR;
			return ret;
		}else{
			ret = errno;
			return ret;
		}

	}
	return ret;
}

int TcpSocket::sendMsg(string sendData,int timeout){
	int ret = writeTimeout(timeout);
	if(ret == 0){
		int writed = 0;
		int dataLen = sendData.size() + 4;
		unsigned char *netdata = (unsigned char*) malloc(dataLen);
		if(netdata == NULL){
			ret = MALLOC_ERROR;
			return ret;
		}
		int netlen = htonl(sendData.size());
		memcpy(netdata,&netlen,4);
		memcpy(netdata+4,sendData.data(),sendData.size());
		writed = writen(netdata,dataLen);
		if(writed < dataLen){
			if(netdata != NULL){
				free(netdata);
				netdata = NULL;
			}
			return writed;
		}
		if(netdata != NULL){
			free(netdata);
			netdata = NULL;
		}
			
	}else{
		if(ret == -1 && errno == ETIMEDOUT){
			ret = TIMEOUT_ERROR;
		}
	}
	return ret;	
}

string TcpSocket::recvMsg(int timeout){
	int ret = readTimeout(timeout);
	if(ret == 0){
		int netdatalen = 0;
		ret = readn(&netdatalen,4);
		if(ret == -1){
			return string();
		}
		else if(ret < 4){
			return string();
		}

		int n = ntohl(netdatalen);
		char* tmpBuf = (char*)malloc(n + 1);
		if(tmpBuf == NULL){
			ret = MALLOC_ERROR;
			return NULL;
		}
		ret = readn(tmpBuf,n);
	
		if(ret == -1){
			return string();
		}else if(ret < n){
			return string();
		}

		tmpBuf[n] = '\0';
		string data = string(tmpBuf);
		free(tmpBuf);
		return data;
	}else{
		if(ret == -1 && errno == ETIMEDOUT){
			printf("readTimeout");
			return string();
		}else{
			return string();
		}
	}
	return string();
}

void TcpSocket::disConnect(){
	if(m_socket >= 0){
		close(m_socket);
	}
}

// ============================
//
//
int TcpSocket::setNonBlock(int fd){
	int ret = 0;
	int flags = 0;
	flags = fcntl(fd,F_GETFL);
	if(flags == -1){
		return flags;
	}

	flags |= O_NONBLOCK;
	ret = fcntl(fd,F_SETFL,flags);
	return ret;
}

int TcpSocket::setBlock(int fd){
	int ret = 0;
	int flags = fcntl(fd,F_GETFL);
	if(flags == -1)
		return flags;
	flags &= ~O_NONBLOCK;
	ret = fcntl(fd,F_SETFL,flags);
	return ret;
}

int TcpSocket::readTimeout(unsigned int wait_seconds){
	int ret = 0;
	if(wait_seconds > 0){
		fd_set read_fdset;
		struct timeval timeout;

		FD_ZERO(&read_fdset);
		FD_SET(m_socket,&read_fdset);
		
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do{
			ret = select(m_socket+1,&read_fdset,NULL,NULL,&timeout);
		
		}while(ret < 0 && errno == EINTR);
		
		if(ret == 0){
			ret == -1;
			errno = ETIMEDOUT;
		}else if(ret == 1){
			ret = 0;
		}

	}
	return ret;
}

int TcpSocket::writeTimeout(unsigned int wait_seconds){
	int ret = 0;
	if(wait_seconds > 0){
		fd_set write_fdset;
		struct timeval timeout;

		FD_ZERO(&write_fdset);
		FD_SET(m_socket,&write_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do{
			ret = select(m_socket + 1,NULL,&write_fdset,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);

		if(ret == 0){
			ret = -1;
			errno = ETIMEDOUT;
		}else if(ret == 1){
			ret = 0;
		}

	}
	return ret;
}

int TcpSocket::connectTimeout(struct sockaddr_in* addr,unsigned int wait_second){
	int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if(wait_second > 0){
		setNonBlock(m_socket);
	}
	ret = connect(m_socket,(struct sockaddr*)&addr,addrlen);
	if(ret < 0 && errno == EINPROGRESS){
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(m_socket,&connect_fdset);
		timeout.tv_sec = wait_second;
		timeout.tv_usec = 0;
		do{
			ret = select(m_socket + 1,NULL,&connect_fdset,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);
		if(ret == 0){
			errno = ETIMEDOUT;
			ret = -1;
		}else if(ret < 0){
			return -1;
		}else if(ret == 1){
			int err = 0;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(m_socket,SOL_SOCKET,SO_ERROR,&err,&socklen);
			if(sockoptret == -1)
				return -1;
			if(err == 0)
				ret = 0;
			else{
				errno = err;
				ret = -1;
			}
		}

	}
	if(wait_second > 0)
		setBlock(m_socket);
	return ret;
}

int TcpSocket::readn(void *buf,size_t len){
	size_t nleft = len;
	ssize_t nread;
	char* bufp = (char*) buf;
	
	while(nleft > 0){
		nread = read(m_socket,bufp,nleft);
		if(nread < 0){
			if(errno == EINTR)
				continue;
			return -1;
		}else if(nread == 0){
			return len - nleft;
		}

		nleft -= nread;
		bufp += nread;
	}
	return len;
}

int TcpSocket::writen(const void *buf,size_t len){
	size_t nleft = len;
	ssize_t nwrite = 0;
	char* bufp = (char*) buf;

	while(nleft > 0){
		if((nwrite = write(m_socket,bufp,nleft)) < 0){
			if(errno == EINTR)
				continue;
			return -1;
		}else if( nwrite == 0){
			continue;
		}
		nleft -= nwrite;
		bufp += nwrite;
	}
	return len;
}

