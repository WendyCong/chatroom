#include "tcpserver.h"
#include "queryview.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

extern CMySqlServer gMySqlServer;
////////////////////////////////////////////
bool QueryModel::checkOnline(string name)
{
	char sql[1024] = {0};
	sprintf(sql, "select * from online;", name.c_str());

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return false;

	gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);
	while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
	{
		if(strcmp(name.c_str(), gMySqlServer.mrow[0]))
			return true;
	}
	return false;
}

bool QueryModel::handleQuery(char (*p)[1024])
{
	char sql[1024] = {0};
	sprintf(sql, "select * from online;");

	if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
		return false;

	memset(*p, 0 ,1024);
	gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);

	strcat(*p, "online user:");
	while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
	{
		strcat(*p, gMySqlServer.mrow[0]);
		strcat(*p, ";");
	}
	return true;
}

///////////////////////////////////////////////
void QueryView::process(Json::Value root)
{
	_name = root["from"].asString();

	if(_queryModel.checkOnline(_name) && _queryModel.handleQuery(&_recv))
		_buserState = true;
	else
		_buserState = false;
}
string QueryView::response()
{
	Json::Value response;
	response["mystype"] = MSG_TYPE_ACK;
	if(_buserState)
		response["msg"] = _recv;
	else
		response["msg"] = _recv;
	return response.toStyledString();
}