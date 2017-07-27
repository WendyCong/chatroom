#ifndef MAIL_H
#define MAIL_H
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
using namespace std;

#define SOCKET_ERROR -1
typedef int SOCKET;

class CSendEmail
{
public:
    //设置邮箱的smtp服务器地址
    void setSmtpServer(string server);
    //设置邮箱的用户名
    void setAuthName(string name);
    //设置邮箱的密码
    void setAuthPwd(string pwd);
    //设置目的邮件地址
    void setDestEmail(string email);
    //设置发送邮件的文本信息
    void setEmailText(string text);
    //发送邮件
    bool sendEmail();
private:
    string smtpServer;
    string username;
    string pwd;
    string destEmail;
    string sendText;
};

#endif