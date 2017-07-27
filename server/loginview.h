#ifndef LOGINVIEW_H
#define LOGINVIEW_H
#include "mysqlserver.h"
//#include "memcached.h"
#include "view.h"
using namespace std;

class LoginModel
{
public:
    bool checkUserAuth(string name, string pwd, string *mail);
    bool sendEmail(string mail);
    bool checkUserKey(char *keybuff);
    bool addOnline(string name,int clientfd);
    bool getOfflineMsg(string name,char (*p)[1024]);
    bool deleteOfflineMsg(string name);
private:
    //CMemcached _memcached;
};

class LoginView : public View
{
public:
    void process(Json::Value root);
    string response();
private:
    string _name;
    string _pwd;
    string _mail;
    LoginModel _loginModel;
    bool _bloginState;
    bool _bemailState;
    int _clientfd;
    char _msg[1024];
};

#endif