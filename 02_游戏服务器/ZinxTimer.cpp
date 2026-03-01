#include "ZinxTimer.h"
#include "ZinxTimerDeliver.h"
#include <stdint.h>
#include <sys/timerfd.h>

#include <iostream>

using namespace std;

//构造函数
ZinxTimer::ZinxTimer()
{
}


//析构函数
ZinxTimer::~ZinxTimer()
{
}

//产生一个每隔1秒钟超时的定时器
bool ZinxTimer::Init()
{
	int ret = -1;

	struct itimerspec tmo = { {1, 0}, {1, 0} };

	//创建定时器
	mFd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (-1 == mFd)
	{
		perror("timerfd_create");
		return false;
	}

	//设置定时器超时时间
	ret = timerfd_settime(mFd, 0, &tmo, NULL);
	if (-1 == ret)
	{
		perror("timerfd_settime");
		return false;
	}

	return true;
}

//超时的次数
bool ZinxTimer::ReadFd(std::string & _input)
{
	uint64_t val;
	int ret = -1;

	//读数据
	ret = read(mFd, (char*)&val, sizeof(val));
	if (ret != sizeof(val))
	{
		perror("read");
		return false;
	}

	_input.append((char *)(&val), sizeof(val));

	return true;
}

bool ZinxTimer::WriteFd(std::string & _output)
{
	return false;
}

//释放资源
void ZinxTimer::Fini()
{
	if (mFd >= 0)
	{
		close(mFd);
	}
}

int ZinxTimer::GetFd()
{
	return mFd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return std::string("timer");
}

AZinxHandler * ZinxTimer::GetInputNextStage(BytesMsg & _oInput)
{
	//uint64_t val = 0;

	//_oInput.szData.copy((char*)&val, sizeof(val), 0);

	//cout << "超时次数: " << val << endl;


	return &ZinxTimerDeliver::GetInstance();
}
