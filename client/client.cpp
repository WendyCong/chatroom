#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <event.h>
#include <iostream>
#include <string>
#include <json/json.h>
#include <event.h>
#include <sys/epoll.h>
#include "../server/public.h"
#include "aes.h"
#include "aes_encryptor.h"
using namespace std;

int sockfd;
pthread_t tid;
#define MAXFD 10
void doLogin();
void doRegister();

typedef struct information
{
	char *name;
	char msg[512];
}info;

void doRecv(char *send)
{
	char buff[1024] = {0};
	int size = recv(sockfd, buff, 1024, 0);
	if(size == 0)
	{
		cout<<"server already close connection"<<endl;
		return;
	}

	Json::Reader reader;
	Json::Value root;
	if(0 == reader.parse(buff, root))
	{
		cout<<"json is invalid!"<<endl;
		return;
	}

	string sendname = root["from"].asString();
	string msg = root["msg"].asString();

	if(sendname.length() != 0)
	{
		cout<<sendname<<":";
	}
	cout<<msg<<endl;
	if(strcmp(msg.c_str(), "quit successful") == 0)
		pthread_exit(NULL);
}

void doSend(info &information)
{
	Json::Value json;
	if(strcmp(information.msg, "quit") == 0)
	{
		json["msgtype"] = MSG_TYPE_QUIT;
		json["name"] = information.name;
	}
	else
	{
		char *p = NULL;//all
		char *q = NULL;//hello
		p = strtok_r(information.msg, ":", &q);

		if(strcmp(p, "all") == 0)
			json["msgtype"] = MSG_TYPE_GROUP;
		else
			json["msgtype"] = MSG_TYPE_TALK;

		json["from"] = information.name;
		json["to"] = p;
		json["msg"] = q;
	}
	send(sockfd, json.toStyledString().c_str(), strlen(json.toStyledString().c_str()), 0);
}

void epoll_add(int epfd, int sockfd, bool isset_et)
{
	struct epoll_event ev;
	ev.data.fd = sockfd;
	ev.events = EPOLLIN;
	if(isset_et)
		ev.events |= EPOLLET;
	int n = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	if(n < 0)
		perror("add error!");
}

void *threadproc(void *lparg)
{
	cout<<"-----------------------------"<<endl;
	cout<<"     welcome to chatroom     "<<endl;
	char *sendname = (char *)lparg;

	info information;
	information.name = sendname;

	int epfd = epoll_create(MAXFD);
	struct epoll_event fds[MAXFD];
	epoll_add(epfd, sockfd, true);
	epoll_add(epfd, 0, true);//将标准输入的事件加入epoll监听队列
	while(1)
	{
		int ret = epoll_wait(epfd, fds, MAXFD, 5000);
		if(ret < 0)
		{
			perror("error");
			continue;
		}

		for(int i=0; i<ret; ++i)
		{
			if(fds[i].events & EPOLLIN)
			{
				if(fds[i].data.fd == sockfd)
					doRecv(sendname);
				else if(fds[i].data.fd == 0)//标准输入的事件
				{
					char message[1024] = {0};
					gets(message);
					if(strcmp(message, "query") == 0)
					{
						Json::Value json;
						json["msgtype"] = MSG_TYPE_QUERY;
						json["sendname"] = sendname;
						send(sockfd, json.toStyledString().c_str(), strlen(json.toStyledString().c_str()), 0);
					}
					else
					{
						strcpy(information.msg, message);
						doSend(information);
					}
				}
			}
		}
	}
}

void doLogin()
{
	char name[20] = {0};
	char pwd[20] = {0};
	char key[20] = {0};

	cout<<"name:";
	gets(name);
	cout<<"pwd:";
	gets(pwd);
	cout<<"*****please wait for a moment*****"<<endl;

	//AES秘钥是一个16字节的char数组
	unsigned char keys[]={
      0x20, 0x04, 0x56, 0x12,
      0x07, 0x04, 0x56, 0x12,
      0x1b, 0x12, 0xf3, 0xa7,
      0x4c, 0x04, 0x2e, 0x12,
    };
    AesEncryptor en(keys);
    string enstr = en.EncryptString(pwd);

	Json::Value json;
	json["msgtype"] = MSG_TYPE_LOGIN;
	json["name"] = name;
	json["pwd"] = enstr;
	send(sockfd, json.toStyledString().c_str(), strlen(json.toStyledString().c_str()), 0);

	char buff[1024] = {0};
	recv(sockfd, buff, 1023, 0);
	if(strcmp(buff, "email send success") == 0)
	{
		cout<<"key:";
		gets(key);
		send(sockfd, key, strlen(key), 0);
		memset(buff, 0, sizeof(buff));
		recv(sockfd, buff, 1023, 0);
		while(1)
		{
			if((strcmp(buff, "input key invalid") == 0))
			{
				cout<<"input key invaild!please input again!"<<endl;
				cout<<"key:";
				gets(key);
				send(sockfd, key, strlen(key), 0);
				memset(buff, 0, sizeof(buff));
				recv(sockfd, buff, 1023, 0);
			}
			else if((strcmp(buff, "input key valid") == 0))
				break;
			break;
		}
	}
	else
	{
		Json::Reader reader;
		Json::Value root;
		if(0 == reader.parse(buff, root))
		{
			cout<<"recv invalid!"<<endl;
			return;
		}
		string reason = root["reason"].asString();
		cout<<"*****"<<reason<<"*****"<<endl;
		return;
	}


	memset(buff, 0, sizeof(buff));
	recv(sockfd, buff, 1023, 0);
	Json::Reader reader;
	Json::Value root;
	if(0 == reader.parse(buff, root))
	{
		cout<<"recv invalid!"<<endl;
		return;
	}
	string reason = root["reason"].asString();
	string msg = root["msg"].asString();
	cout<<"*****"<<reason<<"*****"<<endl;
	cout<<"*****"<<msg<<"*****"<<endl;

	if(strcmp(reason.c_str(), "login successful") == 0)
	{
		int ret = pthread_create(&tid, NULL, threadproc, (void *)name);
		if(ret != 0)
		{
			cout<<"create thread error"<<endl;
			return;
		}
		int n = pthread_join(tid, NULL);
		if(n != 0)
			cout<<"pthread_join error"<<endl;
	}
}

void doRegister()
{
	char name[20] = {0};
	char pwd[20] = {0};
	char mail[30] = {0};
	cout<<"name:";
	gets(name);
	cout<<"pwd:";
	gets(pwd);
	cout<<"email:";
	gets(mail);

	Json::Value json;
	json["msgtype"] = MSG_TYPE_REGISTER;
	json["name"] = name;
	json["pwd"] = pwd;
	json["mail"] = mail;
	send(sockfd, json.toStyledString().c_str(), strlen(json.toStyledString().c_str()), 0);
	cout<<name<<" "<<pwd<<" "<<mail<<endl;
	
	char buff[1024] = {0};
	recv(sockfd, buff, 1023, 0);

	Json::Reader reader;
	Json::Value root;
	if(0 == reader.parse(buff, root))
	{
		cout<<"recv invalid!"<<endl;
		return;
	}
	cout<<"*****"<<root["reason"].asString()<<"*****"<<endl;
}

void doExit()
{
	Json::Value json;
	json["msgtype"] = MSG_TYPE_EXIT;
	send(sockfd, json.toStyledString().c_str(), strlen(json.toStyledString().c_str()), 0);
	close(sockfd);
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		cout<<"command args is invaild! format: a./out ip port"<<endl;
		exit(-1);
	}

	unsigned short port = atoi(argv[2]);
	char ip[20] = {0};
	strcpy(ip, argv[1]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		cerr<<"clientfd create fail! errno:"<<errno<<endl;
        exit(0);
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);

	if(-1 == connect(sockfd, (struct sockaddr*)&server, sizeof(server)))
	{	
		cerr<<"connect fail! errno:"<<errno<<endl;
		exit(0);
	}	

	int choice = 0;
	bool tag = true;
	while(tag)
	{
		cout<<"=============="<<endl;
		cout<<"1.login"<<endl;
		cout<<"2.register"<<endl;
		cout<<"3.exit"<<endl;
		cout<<"=============="<<endl;
		cout<<"choice:";
		cin>>choice;
		getchar();  //  \n

		switch(choice)
		{
		case 1:
			doLogin();
			break;
		case 2:
			doRegister();
			break;
		case 3:
			cout<<"bye bye~~~"<<endl;
			doExit();
			tag = false;
			break;
		default:
			cout<<"input error! please input again!"<<endl;
			break;
		}
	}
	return 0;
}