#ifndef PUBLIC_H
#define PUBLIC_H

typedef enum
{
	MSG_TYPE_LOGIN,//登录
	MSG_TYPE_REGISTER,//注册
	MSG_TYPE_EXIT,
	MSG_TYPE_QUIT,
    MSG_TYPE_ACK,//应答
    MSG_TYPE_TALK,//单聊
	MSG_TYPE_GROUP,//群聊
	MSG_TYPE_QUERY,//查询在线人员
}EnMsgType;

#endif