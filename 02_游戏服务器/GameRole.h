#pragma once
#include "zinx.h"

#include "GameMsg.h"
#include "AOI_World.h"

class GameChannel;
class GameProtocol;

class GameRole :
	public Irole,
	public AOI_Player
{
public:
	GameRole();
	virtual ~GameRole();

	// 通过 Irole 继承
	virtual bool Init() override;
	virtual UserData * ProcMsg(UserData & _poUserData) override;
	virtual void Fini() override;

	/*创建上线时的id和姓名消息*/
	GameMsg *MakeLogonSyncPid();
	/*创建广播聊天消息*/
	GameMsg *MakeTalkBroadcast(std::string _talkContent);
	/*创建广播出生位置消息*/
	GameMsg *MakeInitPosBroadcast();
	/*创建广播移动后新位置消息*/
	GameMsg *MakeNewPosBroadcast();
	/*创建下线时的id和姓名消息*/
	GameMsg *MakeLogoffSyncPid();
	/*创建周围玩家位置消息*/
	GameMsg *MakeSurPlays();
	/*进入场景确认的消息*/
	GameMsg *MakeChangeWorldResponse(int srcId, int targetId);
	/*技能触发消息*/
	GameMsg *MakeSkillTrigger(pb::SkillTrigger *trigger);
	/*技能碰撞消息*/
	GameMsg *MakeSkillContact(pb::SkillContact *contact);

	//处理玩家移动的消息
	void ProcNewPosition(float _x, float _y, float _z, float _v);
	void ProcTalkContent(std::string content);
	void ProcChangeWorld(int srcId, int targetWorldId);
	void ProcSkillTrigger(pb::SkillTrigger *trigger);
	void ProcSkillContact(pb::SkillContact *contact);

	//处理AOI视野消失
	void ViewDisappear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);
	//处理AOI视野出现
	void ViewAppear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);

public:
	GameChannel *mChannel = nullptr;
	GameProtocol *mProtocol = nullptr;

	static int smRoleCount;

	//玩家ID
	int mPlayerId;
	//玩家的姓名
	std::string mPlayerName;

	//表示坐标
	float x;
	float y;
	float z;

	//表示朝向
	float v;

	//血量
	int hp;

	//当前场景的指针
	AOI_World *mCurrentWorld = nullptr;

	// 通过 AOI_Player 继承
	virtual int GetX() override;
	virtual int GetY() override;
};

