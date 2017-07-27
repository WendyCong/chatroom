#include "tcpserver.h"
#include "quitview.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

extern CMySqlServer gMySqlServer;
///////////////////////////////////
bool QuitModel::checkOnline(string name)
{
	char sql[1024] = {0};
	sprintf(sql, "select * from online;");

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return false;

	gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);
	while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
	{
		if(strcmp(name.c_str(), gMySqlServer.mrow[0]) == 0)
			return true;
	}
	return false;
}
bool QuitModel::deleteOnline(string name)
{
	char sql[1024] = {0};
    sprintf(sql, "delete from online where name='%s';", name.c_str());

    if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return false;
    return true;
}

////////////////////////////////////
void QuitView::process(Json::Value root)
{
	_name = root["name"].asString();
	if(_offlineModle.checkOnline(_name) && _offlineModle.deleteOnline(_name))
		_buserState = true;
	else
		_buserState = false;
}
string QuitView::response()
{
	Json::Value response;
	response["msgtype"] = MSG_TYPE_ACK;
	if(_buserState)
		response["msg"] = "quit successful";
	else
		response["msg"] = "quit faild";
	return response.toStyledString();
}