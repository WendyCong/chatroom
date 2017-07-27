#ifndef REGISTERVIEW_H
#define REGISTERVIEW_H
#include "mysqlserver.h"
#include "view.h"
#include <string>
using namespace std;

class RegisterModel
{
public:
	bool checkUserName(string name);
	bool registerUserName(string name, string pwd, string mail);
};

class RegisterView : public View
{
public:
	void process(Json::Value root);
    string response();
private:
	string _name;
	string _pwd;
	string _mail;
	RegisterModel _registerModel;
	bool _bnameState;
	int _clientfd;
};



#endif