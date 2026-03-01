#include "GameRole.h"
#include "GameProtocol.h"
#include "msg.pb.h"
#include "GameMsg.h"
#include "RandomName.h"
#include "WorldManager.h"
#include <random>

using namespace std;
using namespace pb;

int GameRole::smRoleCount = 1;

//设置当前时间为引擎种子
default_random_engine g_random_engine(time(nullptr));


GameRole::GameRole()
{
	//ID和名字
	this->mPlayerId = smRoleCount++;
	//this->mPlayerName = "Player_" + to_string(this->mPlayerId);
	this->mPlayerName = RandomName::GetInstance().GetName();

	//坐标随机赋值
	x = 100 + g_random_engine() % 20;
	y = 0;
	z = 100 + g_random_engine() % 20;

	v = 0;

	//满血是1000
	hp = 1000;

	//cout << "x: " << x << " y: " << y << endl;
}


GameRole::~GameRole()
{
	//归还名字
	RandomName::GetInstance().ReleaseName(this->mPlayerName);
}

//添加到kernel的时候被调用
bool GameRole::Init()
{
	//新客户端连接后，向其发送ID和随机生成的玩家姓名
	//创建玩家上线之后消息 发送ID和姓名
	auto *pMsg = MakeLogonSyncPid();

	//消息发送给协议层
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//初始化出生点 发给客户端
	pMsg = MakeInitPosBroadcast();
	//消息发送给协议层
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//获取第一个场景
	mCurrentWorld = WorldManager::GetInstance().GetWorld(1);

	//将当前玩家添加到世界地图中
	//AOI_World::GetWorld()->AddPlayer(this);
	mCurrentWorld->AddPlayer(this);

	//玩家上线 将自己的位置信息发给世界中所有的玩家 同时将对方的信息发给自己
	//auto players = ZinxKernel::Zinx_GetAllRole();

	//将周围玩家的消息发给当前玩家
	pMsg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//将消息发送给周围所有的玩家
	//auto players = AOI_World::GetWorld()->GetSurPlayers(this);
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		//判断是否为自己
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		//当前玩家创建位置消息
		pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);

		//pMsg = role->MakeInitPosBroadcast();
		//ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	}

	return true;
}

//数据处理
UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_poUserData);

	//遍历消息
	for (auto single : msg.m_MsgList)
	{
		switch (single->m_MsgType)
		{
			//同步玩家位置
		case GameSingleTLV::GAME_MSG_NEW_POSTION:
		{
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);

			cout << "x: " << pbMsg->x()
				<< " y: " << pbMsg->y()
				<< " z: " << pbMsg->z() << endl;

			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}
		//聊天消息
		case GameSingleTLV::GAME_MSG_TALK_CONTENT:
		{
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);

			this->ProcTalkContent(pbMsg->content());
			break;
		}

		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest*>(single->m_poGameMsg);

			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());
			break;
		}

		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			auto pbMsg = dynamic_cast<pb::SkillTrigger*>(single->m_poGameMsg);

			this->ProcSkillTrigger(pbMsg);
			break;
		}

		case GameSingleTLV::GAME_MSG_SKILL_CONTACT:
		{
			auto pbMsg = dynamic_cast<pb::SkillContact*>(single->m_poGameMsg);
			this->ProcSkillContact(pbMsg);
			break;
		}

		}
	}

	return nullptr;
}

//玩家下线的时候会调用
void GameRole::Fini()
{
	//玩家下线之后，将下线消息发给当前所有的玩家

	//获取当前所有的玩家
	//auto players = ZinxKernel::Zinx_GetAllRole();

	//获取当前玩家周围的所有玩家
	//auto players = AOI_World::GetWorld()->GetSurPlayers(this);
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		//创建当前玩家下线的消息

		auto pMsg = MakeLogoffSyncPid();
		//发送消息到当前所有的玩家
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

	//当前玩家从世界地图中移除
	//AOI_World::GetWorld()->DelPlayer(this);
	mCurrentWorld->DelPlayer(this);
}


/*创建上线时的id和姓名消息*/
GameMsg * GameRole::MakeLogonSyncPid()
{
	auto pSyncPid = new  SyncPid;

	pSyncPid->set_pid(this->mPlayerId);
	pSyncPid->set_username(this->mPlayerName);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGON_SYNCPID, pSyncPid);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//生成聊天信息
GameMsg * GameRole::MakeTalkBroadcast(std::string _talkContent)
{
	auto pMsg = new BroadCast;

	pMsg->set_pid(this->mPlayerId);
	pMsg->set_username(this->mPlayerName);

	/*根据Tp不同，BroadCast消息会包含：
	  1 聊天内容（Content）
	  2 初始位置(P)
	  4 新位置P
	  */
	pMsg->set_tp(1);
	pMsg->set_content(_talkContent);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);


	return retMsg;
}

/*创建广播出生位置消息*/
GameMsg * GameRole::MakeInitPosBroadcast()
{
	auto pMsg = new BroadCast;

	//赋值
	pMsg->set_pid(this->mPlayerId);
	pMsg->set_username(this->mPlayerName);

	/*根据Tp不同，BroadCast消息会包含：
	  1 聊天内容（Content）
	  2 初始位置(P)
	  4 新位置P
	  */
	pMsg->set_tp(2);

	auto pos = pMsg->mutable_p();
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);
	pos->set_bloodvalue(hp);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

/*创建广播移动后新位置消息*/
GameMsg * GameRole::MakeNewPosBroadcast()
{
	auto pbMsg = new BroadCast;

	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);
	/*根据Tp不同，BroadCast消息会包含：
	  1 聊天内容（Content）
	  2 初始位置(P)
	  4 新位置P
	  */
	pbMsg->set_tp(4);

	auto pos = pbMsg->mutable_p();
	pos->set_x(this->x);
	pos->set_y(this->y);
	pos->set_z(this->z);
	pos->set_v(this->v);

	pos->set_bloodvalue(this->hp);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);

	auto *retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

/*创建下线时的id和姓名消息*/
GameMsg * GameRole::MakeLogoffSyncPid()
{
	auto pbMsg = new SyncPid;

	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGOFF_SYNCPID, pbMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);


	return retMsg;
}

/*创建周围玩家位置消息*/
GameMsg * GameRole::MakeSurPlays()
{
	auto pbMsg = new SyncPlayers;

	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);

	//循环遍历周围玩家
	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		auto p = pbMsg->add_ps();
		p->set_pid(role->mPlayerId);
		p->set_username(role->mPlayerName);

		auto pos = p->mutable_p();
		pos->set_x(role->x);
		pos->set_y(role->y);
		pos->set_z(role->z);
		pos->set_v(role->v);
		pos->set_bloodvalue(role->hp);
	}

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SUR_PLAYER, pbMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);


	return retMsg;
}


/*进入场景确认的消息*/
GameMsg * GameRole::MakeChangeWorldResponse(int srcId, int targetId)
{
	auto pbMsg = new ChangeWorldResponse;

	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_changeres(1);//1表示切换成功
	pbMsg->set_srcid(srcId);
	pbMsg->set_targetid(targetId);

	auto pos = pbMsg->mutable_p();
	pos->set_x(this->x);
	pos->set_y(this->y);
	pos->set_z(this->z);
	pos->set_v(this->v);
	pos->set_bloodvalue(this->hp);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_CHANGE_WORLD_RESPONSE, pbMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);


	return retMsg;
}

//生成技能触发的消息
GameMsg * GameRole::MakeSkillTrigger(pb::SkillTrigger * trigger)
{
	auto pbMsg = new SkillTrigger(*trigger);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, pbMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//生成技能命中的消息
GameMsg * GameRole::MakeSkillContact(pb::SkillContact * contact)
{
	auto pbMsg = new SkillContact(*contact);

	//序列化对象
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_CONTACT_BROAD, pbMsg);

	auto *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//处理玩家移动位置消息
void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{

	//如果玩家的格子数有变化
	if (mCurrentWorld->GridChanged(this, _x, _z))
	{

		//获取旧的周围玩家
		auto oldList = mCurrentWorld->GetSurPlayers(this);

		//从世界地图中删除玩家
		mCurrentWorld->DelPlayer(this);
		//给玩家的位置变量赋值
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->v = _v;

		//获取新的周围玩家
		auto newList = mCurrentWorld->GetSurPlayers(this);

		//玩家的视野消失和出现
		ViewDisappear(oldList, newList);
		ViewAppear(oldList, newList);

		//将玩家添加到世界地图中
		mCurrentWorld->AddPlayer(this);
	}
	else
	{
		//给玩家的位置变量赋值
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->v = _v;
	}


	//将新的位置消息发送给所有的玩家
	//获取当前所有的玩家
	//auto players = ZinxKernel::Zinx_GetAllRole();

	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		//判断是否为自己
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		//创建玩家新位置消息
		auto pMsg = MakeNewPosBroadcast();

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

}

//处理聊天消息
void GameRole::ProcTalkContent(std::string content)
{
	//将聊天消息发给所有的玩家
	auto players = ZinxKernel::Zinx_GetAllRole();

	for (auto r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);

		auto pMsg = MakeTalkBroadcast(content);
		//发送消息给所有的玩家
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}
}

//切换场景的操作
void GameRole::ProcChangeWorld(int srcId, int targetWorldId)
{
	//从原来的场景中下线
	mCurrentWorld->DelPlayer(this);

	//告诉周围玩家我下线消息
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		auto pMsg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

	//获取当前的场景
	if (1 == targetWorldId)
	{
		//坐标随机赋值
		x = 100 + g_random_engine() % 20;
		y = 0;
		z = 100 + g_random_engine() % 20;

		v = 0;

		//满血是1000
		hp = 1000;
	}
	else if (2 == targetWorldId)
	{
		//坐标随机赋值
		x = 10 + g_random_engine() % 100;
		y = 0;
		z = 10 + g_random_engine() % 100;

		v = 0;
	}

	//上线新的场景
	mCurrentWorld = WorldManager::GetInstance().GetWorld(targetWorldId);
	mCurrentWorld->AddPlayer(this);

	//告诉当前世界中所有其它玩家自己的消息
	players = mCurrentWorld->GetSurPlayers(this);
	for (auto r : players)
	{
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);
		auto pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

	//发送新场景发送响应
	auto pMsg = MakeChangeWorldResponse(srcId, targetWorldId);
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//获取周围玩家
	pMsg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
}

//技能触发
void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	if (this->mPlayerId != trigger->pid())
	{
		return;
	}

	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);

	//将触发的技能广播给所有周围的玩家
	for (auto r : players)
	{
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);
		auto pMsg = MakeSkillTrigger(trigger);
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}
}

//技能命中处理
void GameRole::ProcSkillContact(pb::SkillContact * contact)
{
	//被攻击者ID
	int targetId = contact->targetpid();
	cout << "srcPid: " << contact->srcpid() << " targetId: " << contact->targetpid() << endl;

	//出错检查
	if (this->mPlayerId != contact->srcpid())
	{
		return;
	}

	GameRole *targetRole = nullptr;
	
	//查找被攻击的角色
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		if (role->mPlayerId == targetId)
		{
			targetRole = role;
			break;
		}
	}

	if (nullptr == targetRole)
	{
		return;
	}

	//计算受伤害的值
	int attackHp = 300 + g_random_engine() % 300;
	targetRole->hp -= attackHp;

	auto pos = contact->mutable_contactpos();
	pos->set_bloodvalue(targetRole->hp);

	//将被攻击者的血量广播给所有的周围玩家
	players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		auto pMsg = MakeSkillContact(contact);

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);	
	}

	//判断玩家血量是否为0或者小于0
	if (targetRole->hp <= 0)
	{
		//切换场景 切换到第一个场景
		targetRole->ProcChangeWorld(targetRole->mCurrentWorld->mWorldId, 1);
	}
}

//视野消失
void GameRole::ViewDisappear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//旧的玩家集合 - 新的玩家集合 = 从当前玩家的视野中消失
	//集合计算差集必须要有序
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	//排序
	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	vector<AOI_Player*> diff;

	//计算差集
	std::set_difference(oldVec.begin(), oldVec.end(), newVec.begin(), newVec.end(),
		std::inserter(diff, diff.begin()));

	//diff集合中所有的玩家下线
	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		//当前玩家将下线的消息发给对方
		auto pMsg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);

		
		//对方将下线的消息发送给自己
		pMsg = role->MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
	}
}

//玩家出现
void GameRole::ViewAppear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//新的玩家集合 - 旧的玩家集合 = 在当前玩家中出现
	//集合计算差集必须有序
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	//排序
	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//计算差集
	vector<AOI_Player*> diff;
	std::set_difference(newVec.begin(), newVec.end(), oldVec.begin(), oldVec.end(),
		std::inserter(diff, diff.begin()));

	//diff集合中的所有玩家上线
	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		//当前玩家将上线的消息发给对方
		auto pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);

		//对方将上线的消息发给自己
		pMsg = role->MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
	}
}

int GameRole::GetX()
{
	return this->x;
}

//在3D中z表示二维中y
int GameRole::GetY()
{
	return this->z;
}
