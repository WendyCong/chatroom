#ifndef TALKVIEW_H
#define TALKVIEW_H
#include "mysqlserver.h"
#include "view.h"
using namespace std;

class TalkModel
{
public:
	int checkOnline(string name);
	void handleOnlineMsg(int recvfd, string sendname, string msg);
	bool handleOfflineMsg(string sendname, string revcname, string msg);
};

class TalkView : public View
{
public:
	void process(Json::Value root);
    string response();
private:
	string _sendname;
	string _recvname;
	int _recvfd;
	int _sendfd;
	TalkModel _talkModel;
	bool _btalkState;
	string _msg;
};


#endif