#pragma once
#include "ZinxTCP.h"

class GameProtocol;
class GameRole;

//通道层
class GameChannel :
	public ZinxTcpData
{
public:
	GameChannel(int _fd);
	virtual ~GameChannel();

	// 通过 ZinxTcpData 继承
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

public:
	GameProtocol *mProtocol = nullptr;
	GameRole *mRole = nullptr;
};


//TCP数据通道类工厂
class TcpConnFactory :public IZinxTcpConnFact
{
public:
	// 通过 IZinxTcpConnFact 继承
	virtual ZinxTcpData * CreateTcpDataChannel(int _fd) override;

};
