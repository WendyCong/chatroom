#include "tcpserver.h"
#include "loginview.h"
#include <json/json.h>
#include <stdio.h>
#include <string.h>
#include "public.h"
#include "mail.h"
#include "aes.h"
#include "aes_encryptor.h"

extern CMySqlServer gMySqlServer;
extern int socketfd[2];
char key[10] = {0};
/////////////////////////////////////////////////
bool LoginModel::checkUserAuth(string name, string pwd, string *mail)
{
	char sql[1024] = {0};
    sprintf(sql, "select * from user;");

    if(mysql_real_query(gMySqlServer.mpcon,sql,strlen(sql)))
        return false;
    
    gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);

    while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
    {
        if(strcmp(name.c_str(), gMySqlServer.mrow[0]) == 0)
        {
            if(strcmp(pwd.c_str(), gMySqlServer.mrow[1]) == 0)
            {
            	*mail = gMySqlServer.mrow[2];
                return true;
            }
        }
    }
    *mail = "";
    return false;
}

bool LoginModel::sendEmail(string mail)
{
    srand(time(0));
	//随机生成6位数字验证码
    char num[10] = {0}; 
    for(int i=0; i<6; ++i)
    {
        int tag = rand()%3;
        if(tag == 0)//0->大写
            key[i] = rand()%26+'A';
        else if(tag == 1)//1->小写
            key[i] = rand()%26+'a';
        else//2->数字
            key[i] = rand()%10+'0';
    }
    cout<<"key:"<<key<<endl;

    CSendEmail email;
    email.setSmtpServer("smtp.163.com");
    email.setAuthName("18292090320@163.com");
    email.setAuthPwd("lch19961024");
    email.setDestEmail(mail);
    email.setEmailText(key);
    if(email.sendEmail())
    {
        cout<<"email send success!"<<endl;
        return true;
    }
    else
    {
        cout<<"email send failure!"<<endl;
        return false;
    }
}

bool LoginModel::checkUserKey(char *keybuff)
{
	if(strcmp(key, keybuff) == 0)
        return true;
    return false;
}

bool LoginModel::addOnline(string name, int clientfd)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into online values('%s','%d');",name.c_str(), clientfd);

    if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) == 0)//插入成功
        return true;
    return false;
}

bool LoginModel::getOfflineMsg(string name, char (*p)[1024])
{
    char sql[1024] = {0};
    sprintf(sql, "select * from offline_msg;");
    if(mysql_real_query(gMySqlServer.mpcon,sql,strlen(sql)) != 0)
        return false;

    gMySqlServer.mpres = mysql_store_result(gMySqlServer.mpcon);

    memset(*p, 0, 1024);
    while(gMySqlServer.mrow = mysql_fetch_row(gMySqlServer.mpres))
    {
        if(strcmp(name.c_str(), gMySqlServer.mrow[0]) == 0)
        {
            strcat(*p, gMySqlServer.mrow[1]);
            strcat(*p,"   ");
        }
    }

    if(strlen(*p) == 0)
        strcpy(*p, "no offline message!");
    return true;
}

bool LoginModel::deleteOfflineMsg(string name)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offline_msg where name='%s';", name.c_str());

    if(mysql_real_query(gMySqlServer.mpcon, sql, strlen(sql)) != 0)
        return false;
    return true;
}

////////////////////////////////////////////////////////////////
void LoginView::process(Json::Value root)
{
    unsigned char key[]={
      0x20, 0x04, 0x56, 0x12,
      0x07, 0x04, 0x56, 0x12,
      0x1b, 0x12, 0xf3, 0xa7,
      0x4c, 0x04, 0x2e, 0x12,
    };
    AesEncryptor en(key);

    _name = root["name"].asString();
    string enstr = root["pwd"].asString();
    _pwd = en.DecryptString(enstr);

	if(_loginModel.checkUserAuth(_name, _pwd, &_mail))
	{
		if(_loginModel.sendEmail(_mail))
        {
            _bemailState = true;

            int clibuff[2] = {0};
            read(socketfd[0], clibuff, 4);
            _clientfd = clibuff[0];
            
            send(_clientfd, "email send success", 18, 0);

            char keybuff[10] = {0};
            recv(_clientfd, keybuff, 10, 0);
            while(!_loginModel.checkUserKey(keybuff))
            {
                send(_clientfd, "input key invalid", 17, 0);
                memset(keybuff, 0, 10);
                recv(_clientfd, keybuff, 10, 0);
            }
            send(_clientfd, "input key vaild", 15, 0);
            _loginModel.addOnline(_name, _clientfd);
            _loginModel.getOfflineMsg(_name,&_msg);
            _loginModel.deleteOfflineMsg(_name);
            _bloginState = true;
            return;
        }
        else
        {
            _bemailState = true;
            _bloginState = false;
            return;
        }
	}
    else
    {
        _bemailState = false;
        _bloginState = false;
        return;
    }
}

string LoginView::response()
{
    Json::Value response;
    response["msgtype"] = MSG_TYPE_ACK;
    if(_bemailState && _bloginState)
    {
        response["msg"] = _msg;
        response["reason"] = "login successful";
    }
    else if(!_bemailState && !_bloginState)
    {
        response["reason"] = "name or pwd is invalid!";
    }
    else if(_bemailState && !_bloginState)
    {
        response["reason"] = "Internet error!send email fail!";
    }
    return response.toStyledString();
}