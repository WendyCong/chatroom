#include "tcpserver.h"
#include "groupview.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

extern CMySqlServer gMySqlServer;
////////////////////////////////////////
void GroupModel::getOnline(string sendname,int (*p)[1024],int *num)
{
	char sql[1024] = {0};
	sprintf(sql, "select * from online;");

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return;

	gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);
	*num = 0;
	int i = 0;
	int own = 0;
	while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
	{
		*num += 1;
		if(strcmp(sendname.c_str(), gMySqlServer.mrow[0]) == 0)
			own = atof(gMySqlServer.mrow[1]);//除去自己的fd
		else
			(*p)[i++] = atof(gMySqlServer.mrow[1]);
	}
	(*p)[(*num)-1] = own;
}
bool GroupModel::handleGroupMsg(int (*p)[1024], int *num, string sendname, string msg)
{
	Json::Value root;
	root["from"] = sendname;
	root["msg"] = msg;

	for(int i=0; i<(*num)-1; ++i)
	{
		send((*p)[i], root.toStyledString().c_str(), strlen(root.toStyledString().c_str()), 0);
	}
	return true;
}

////////////////////////////////////////
void GroupView::process(Json::Value root)
{
	_sendname = root["from"].asString();
	_recvname = root["to"].asString();
	_msg = root["msg"].asString();

	_groupModel.getOnline(_sendname, &_recvfd, &_num);
	_sendfd = _recvfd[_num-1];

	if(_num > 1 && _groupModel.handleGroupMsg(&_recvfd, &_num, _sendname, _msg))
		_bgroupState = true;
	else
		_bgroupState = false;
}
string GroupView::response()
{
	Json::Value response;
	response["msgtype"] = MSG_TYPE_ACK;
    if(_bgroupState)
        response["msg"] = "send successful";
    else
        response["msg"] = "no another user online";
    return response.toStyledString();
}