#include "tcpserver.h"
#include "talkview.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

extern CMySqlServer gMySqlServer;
/////////////////////////////////////
int TalkModel::checkOnline(string name)
{
	char sql[1024] = {0};
	sprintf(sql, "select * from online where name='%s';", name.c_str());

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return false;

	gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);

	while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
	{
		if(strcmp(name.c_str(), gMySqlServer.mrow[0]) == 0)
		{
			return atof(gMySqlServer.mrow[1]);
		}
	}
	return -1;
}

void TalkModel::handleOnlineMsg(int _recvfd, string sendname, string msg)
{
	Json::Value root;
	root["from"] = sendname;
	root["msg"] = msg;

	send(_recvfd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str()), 0);
}

bool TalkModel::handleOfflineMsg(string sendname, string revcname, string msg)
{
	char OfflineMsg[1024] = {0};
	for(int i=0; i<sendname.length(); ++i)
	{
		OfflineMsg[i] = sendname[i];
	}
	strcat(OfflineMsg, ":");

	int len = strlen(OfflineMsg);
	for(int i=0; i<msg.length(); ++i)
	{
		OfflineMsg[i+len] = msg[i];
	}

	char sql[1024] = {0};
	sprintf(sql, "insert into offline_msg values('%s','%s');", revcname.c_str(), OfflineMsg);

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) == 0)
		return true;
	return false;
}

/////////////////////////////////////
void TalkView::process(Json::Value root)
{
	_sendname = root["from"].asString();
	_recvname = root["to"].asString();
	_msg = root["msg"].asString();

	_recvfd = _talkModel.checkOnline(_recvname);
	_sendfd = _talkModel.checkOnline(_sendname);
	if(_recvfd == -1)
	{
		_btalkState = false;
		_talkModel.handleOfflineMsg(_sendname, _recvname, _msg);
	}
	else
	{
		_btalkState = true;
		_talkModel.handleOnlineMsg(_recvfd, _sendname, _msg);
	}
}

string TalkView::response()
{
	Json::Value response;
	response["msgtype"] = MSG_TYPE_ACK;
    if(_btalkState)
        response["msg"] = "send successful";
    else
        response["msg"] = "the user is not online,next online will see";
    return response.toStyledString();
}
