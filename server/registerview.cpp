#include "tcpserver.h"
#include "registerview.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

extern CMySqlServer gMySqlServer;
extern int socketfd[2];
////////////////////////////////////////////////////////////////
bool RegisterModel::checkUserName(string name)
{
	char sql[1024]={0};
    sprintf(sql, "select * from user;");

    if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)//query error
        return false;

    gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);
	
    while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
    {
        if(strcmp(name.c_str(), gMySqlServer.mrow[0]) == 0)
            return false;
    }
	return true;	
}

bool RegisterModel::registerUserName(string name, string pwd, string mail)
{
	char sql[1024] = {0};
	sprintf(sql, "insert into user values('%s','%s', '%s');",name.c_str(), pwd.c_str(), mail.c_str());
	
	if(mysql_real_query(gMySqlServer.mpcon,sql,strlen(sql)) != 0)
        return false;
    return true;
}

//////////////////////////////////////////////
void RegisterView::process(Json::Value root)
{
	_name = root["name"].asString();
	_pwd = root["pwd"].asString();
	_mail = root["mail"].asString();
	if(_registerModel.registerUserName(_name, _pwd, _mail))
		_bnameState = true;
	else
		_bnameState = false;
}

string RegisterView::response()
{
	Json::Value response;
    response["msgtype"] = MSG_TYPE_ACK;
	if(_bnameState)
		response["reason"] = "register sussessful";
	else
		response["reason"] = "use name has exist or key error!";
	return response.toStyledString();
}