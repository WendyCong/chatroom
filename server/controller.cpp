#include "tcpserver.h"
#include "controller.h"
#include "loginview.h"
#include "registerview.h"
#include "quitview.h"
#include "talkview.h"
#include "groupview.h"
#include "queryview.h"
#include <json/json.h>
#include <string.h>
#include "public.h"

Controller gController;

Controller::Controller()
{
    _viewMap.insert(make_pair("login", new LoginView()));
	_viewMap.insert(make_pair("register", new RegisterView()));
	_viewMap.insert(make_pair("talk", new TalkView()));
    _viewMap.insert(make_pair("group", new GroupView()));
    _viewMap.insert(make_pair("query", new QueryView()));
    _viewMap.insert(make_pair("quit", new QuitView()));
    cout<<"controller init success!"<<endl;
}

void Controller::process(string json, int fd)
{
    Json::Reader reader;
    Json::Value root;
    
    if(0 == reader.parse(json.c_str(), root))
    {
        cerr<<"json format invalid:"<<json<<endl;
        return;
    }

    int msgtype = root["msgtype"].asInt();

    string result;
    switch(msgtype)
    {
        case MSG_TYPE_LOGIN:
        {
            map<string, View*>::iterator it = _viewMap.find("login");
            it->second->process(root);
            result = it->second->response();
			break;
        }
		case MSG_TYPE_REGISTER:
        {
            map<string, View*>::iterator it = _viewMap.find("register");
            it->second->process(root);
            result = it->second->response();
			break;
        }
		case MSG_TYPE_EXIT:
		{
            close(fd);
            cout<<"one client exit"<<endl;
			break;
		}
        case MSG_TYPE_TALK:
        {
            map<string, View*>::iterator it = _viewMap.find("talk");
            it->second->process(root);
            result = it->second->response();
            break;
        }
        case MSG_TYPE_GROUP:
        {
            map<string, View*>::iterator it = _viewMap.find("group");
            it->second->process(root);
            result = it->second->response();
            break;
        }
        case MSG_TYPE_QUERY:
        {
            map<string, View*>::iterator it = _viewMap.find("query");
            it->second->process(root);
            result = it->second->response();
            break;
        }
        case MSG_TYPE_QUIT:
        {
            map<string, View*>::iterator it = _viewMap.find("quit");
            it->second->process(root);
            result = it->second->response();
            break;
        }
        default:
            break;
    }
    send(fd, result.c_str(), strlen(result.c_str()), 0);
}