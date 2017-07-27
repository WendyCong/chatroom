#ifndef QUITVIEW_H
#define QUITVIEW_H
#include "mysqlserver.h"
#include "view.h"
using namespace std;

class QuitModel
{
public:
	bool checkOnline(string name);
	bool deleteOnline(string name);
};

class QuitView : public View
{
public:
	void process(Json::Value root);
    string response();
private:
	string _name;
	bool _buserState;
	QuitModel _offlineModle;
};

#endif
