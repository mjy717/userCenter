#pragma once
#include "zinx.h"


//定时器通道类
class ZinxTimer :
	public Ichannel
{
public:
	ZinxTimer();
	virtual ~ZinxTimer();

	// 通过 Ichannel 继承
	virtual bool Init() override;
	virtual bool ReadFd(std::string & _input) override;
	virtual bool WriteFd(std::string & _output) override;
	virtual void Fini() override;
	virtual int GetFd() override;
	virtual std::string GetChannelInfo() override;
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

private:
	int mFd = -1;
};

