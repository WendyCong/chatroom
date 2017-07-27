#ifndef QUERYVIEW_H
#define QUERYVIEW_H
#include "mysqlserver.h"
#include "view.h"
using namespace std;

class QueryModel
{
public:
	bool checkOnline(string name);
	bool handleQuery(char (*p)[1024]);
};

///////////////////////////////////
class QueryView : public View
{
public:
	void process(Json::Value root);
    string response();
private:
	string _name;
	bool _buserState;
	QueryModel _queryModel;
	char _recv[1024];
};


#endif