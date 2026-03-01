#include "GameProtocol.h"

#include "GameChannel.h"
#include "GameRole.h"
#include "GameMsg.h"

using namespace std;

GameProtocol::GameProtocol()
{
}


GameProtocol::~GameProtocol()
{
}

//将原始的数据进行解析，解决粘包问题
UserData * GameProtocol::raw2request(std::string _szInput)
{
	int msgId = 0;
	int msgLen = 0;
	string strContent;

	GameMsg *retMsg = nullptr;

	//追加到缓冲区
	mLastBuf.append(_szInput);

	//消息格式: 消息长度(4) + 消息ID(4) + 消息内容
	while (mLastBuf.size() >= 8)
	{
		//获取消息长度
		msgLen = mLastBuf[0] |
			mLastBuf[1] << 8 |
			mLastBuf[2] << 16 |
			mLastBuf[3] << 24;

		msgId = mLastBuf[4] |
			mLastBuf[5] << 8 |
			mLastBuf[6] << 16 |
			mLastBuf[7] << 24;

		if (mLastBuf.size() - 8 >= msgLen)
		{
			//获取消息内容
			strContent = mLastBuf.substr(8, msgLen);

			//从缓冲区中移除消息
			mLastBuf.erase(0, 8 + msgLen);

			//cout << "msgLen: " << msgLen << " msgId: " << msgId << " content:" << strContent << endl;
			cout << "msgLen: " << msgLen << " msgId: " << msgId << endl;

			//进行反序列化
			GameSingleTLV* tlv = new GameSingleTLV((GameSingleTLV::GameMsgType)msgId, strContent);

			//tlv->m_poGameMsg->PrintDebugString();
			if (retMsg == nullptr)
			{
				retMsg = new GameMsg;
			}

			retMsg->m_MsgList.push_back(tlv);
		}
		else
		{
			//接收包不完整 缺包
			break;
		}
	}

	return retMsg;
}

//将协议层的数据做封装 发送到通道层
std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	int msgId = 0;
	int msgLen = 0;

	string * retBuf = nullptr;

	GameMsg *pMsg = dynamic_cast<GameMsg*>(&_oUserData);

	for (auto it : pMsg->m_MsgList)
	{
		string buf;

		if (nullptr == retBuf)
		{
			retBuf = new string;
		}

		//it->m_poGameMsg->SerializeToString(&buf);
		buf = it->Serialize();

		//消息类型
		msgId = it->m_MsgType;
		//消息长度
		msgLen = buf.size();

		//消息长度 + 消息类型 + 消息内容  msgLen = 0x04030201
		retBuf->push_back(msgLen & 0xff);			// 01
		retBuf->push_back((msgLen >> 8) & 0xff);	// 02
		retBuf->push_back((msgLen >> 16) & 0xff);	// 03
		retBuf->push_back((msgLen >> 24) & 0xff);	// 04
	
		retBuf->push_back(msgId & 0xff);			// 01
		retBuf->push_back((msgId >> 8) & 0xff);	// 02
		retBuf->push_back((msgId >> 16) & 0xff);	// 03
		retBuf->push_back((msgId >> 24) & 0xff);	// 04
	
		retBuf->append(buf);
	}


	return retBuf;
}

Irole * GameProtocol::GetMsgProcessor(UserDataMsg & _oUserDataMsg)
{
	return mRole;
}

Ichannel * GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	return mChannel;
}
