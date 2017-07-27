#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <map>
#include <string>
#include "view.h"
using namespace std;

class Controller
{
public:
    //注册所有现存的视图
    Controller();
    void process(string json, int fd);
private:
    map<string, View*> _viewMap;
};

#endif