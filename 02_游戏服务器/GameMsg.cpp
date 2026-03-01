#include "GameMsg.h"

using namespace pb;
using namespace std;

//构造函数 
GameSingleTLV::GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg)
{
	//消息ID
	m_MsgType = _Type;
	//序列化对象s
	m_poGameMsg = _poGameMsg;
}

//构造函数 反序列化
GameSingleTLV::GameSingleTLV(GameMsgType _Type, std::string _szInputData)
{
	switch (_Type)
	{
		//同步玩家
	case GAME_MSG_LOGON_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//聊天
	case GAME_MSG_TALK_CONTENT:
		m_poGameMsg = new Talk;
		break;

		//新位置
	case GAME_MSG_NEW_POSTION:
		m_poGameMsg = new Position;
		break;

		//技能触发
	case GAME_MSG_SKILL_TRIGGER:
		m_poGameMsg = new SkillTrigger;
		break;

		//技能命中
	case GAME_MSG_SKILL_CONTACT:
		m_poGameMsg = new SkillContact;
		break;

		//切换场景
	case GAME_MSG_CHANGE_WORLD:
		m_poGameMsg = new ChangeWorldRequest;
		break;

		//普通广播
	case GAME_MSG_BROADCAST:
		m_poGameMsg = new BroadCast;
		break;

		//玩家下线
	case GAME_MSG_LOGOFF_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//获取周边玩家
	case GAME_MSG_SUR_PLAYER:
		m_poGameMsg = new SyncPlayers;
		break;

	case GAME_MSG_SKILL_BROAD:
		m_poGameMsg = new SkillTrigger;
		break;

		//技能命中
	case GAME_MSG_SKILL_CONTACT_BROAD:
		m_poGameMsg = new SkillContact;
		break;

		//切换场景响应
	case GAME_MSG_CHANGE_WORLD_RESPONSE:
		m_poGameMsg = new ChangeWorldResponse;
		break;
	}

	if (nullptr != m_poGameMsg)
	{
		//反序列化
		m_poGameMsg->ParseFromString(_szInputData);
		m_MsgType = _Type;
	}
}

//析构函数
GameSingleTLV::~GameSingleTLV()
{
	if (nullptr != m_poGameMsg)
	{
		delete m_poGameMsg;
	}
}

//序列化
std::string GameSingleTLV::Serialize()
{
	string str;

	m_poGameMsg->SerializeToString(&str);
	return str;
}

//构造函数
GameMsg::GameMsg()
{
}

//析构函数
GameMsg::~GameMsg()
{
	for (auto &it : m_MsgList)
	{
		delete it;
		//it = nullptr;
	}
}
