#include "WorldManager.h"
#include "AOI_World.h"

WorldManager WorldManager::smManager;

WorldManager & WorldManager::GetInstance()
{
	// TODO: 在此处插入 return 语句
	return smManager;
}


AOI_World * WorldManager::GetWorld(int id)
{
	return mVecWorld[id];
}


//构造函数
WorldManager::WorldManager()
{
	mVecWorld.reserve(3);

	mVecWorld[0] = nullptr;

	//第一个场景
	mVecWorld[1] = new AOI_World(85, 410, 75, 400, 10, 20);
	mVecWorld[1]->mWorldId = 1;

	//第二个场景 战斗场景
	mVecWorld[2] = new AOI_World(0, 140, 0, 140, 1, 1);
	mVecWorld[2]->mWorldId = 2;
}

//析构函数
WorldManager::~WorldManager()
{
	for (auto l : mVecWorld)
	{
		if (nullptr != l)
		{
			delete l;
		}
	}
}
