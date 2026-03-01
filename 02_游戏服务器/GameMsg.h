#pragma once
#include "zinx.h"

#include "msg.pb.h"

class GameSingleTLV :public UserData 
{
public:
	//根据游戏的协议需求,定义的消息ID 枚举
	enum GameMsgType {
		GAME_MSG_LOGON_SYNCPID = 1,             //同步玩家id和名字
		GAME_MSG_TALK_CONTENT = 2,              //聊天信息
		GAME_MSG_NEW_POSTION = 3,               //同步玩家位置
		GAME_MSG_SKILL_TRIGGER = 4,             //技能触发
		GAME_MSG_SKILL_CONTACT = 5,             //技能命中
		GAME_MSG_CHANGE_WORLD = 6,              //场景切换

		GAME_MSG_BROADCAST = 200,               //普通广播消息
		GAME_MSG_LOGOFF_SYNCPID = 201,          //玩家离线消息
		GAME_MSG_SUR_PLAYER = 202,              //同步周边玩家消息
		GAME_MSG_SKILL_BROAD = 204,             //技能触发广播
		GAME_MSG_SKILL_CONTACT_BROAD = 205,     //技能命中广播
		GAME_MSG_CHANGE_WORLD_RESPONSE = 206,   //切换场景广播
	} m_MsgType;

	//存储单个protobuf的消息对象
	google::protobuf::Message *m_poGameMsg = NULL;

public:
	//构造函数 序列化
	GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg);

	//构造函数 反序列化
	GameSingleTLV(GameMsgType _Type, std::string _szInputData);

	//析构函数
	~GameSingleTLV();

	//序列化
	std::string Serialize();

};

//存储从客户端发送过来的数据，或者从服务器要发送到客户端的消息
class GameMsg :
	public UserData
{
public:
	GameMsg();
	virtual ~GameMsg();

public:
	std::list<GameSingleTLV*>m_MsgList;
};