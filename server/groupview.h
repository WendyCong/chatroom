#ifndef GROUPVIEW_H
#define GROUPVIEW_H
#include "mysqlserver.h"
#include "view.h"
using namespace std;

class GroupModel
{
public:
	void getOnline(string sendname, int (*p)[1024], int *num);
	bool handleGroupMsg(int (*p)[1024], int *num, string sendname, string msg);
};

class GroupView : public View
{
public:
	void process(Json::Value root);
    string response();
private:
	string _sendname;
	string _recvname;
	GroupModel _groupModel;
	bool _bgroupState;
	int _sendfd;
	int _recvfd[1024];//存放在线用户的clientfd
	int _num;//在线人数
	string _msg;
};


#endif