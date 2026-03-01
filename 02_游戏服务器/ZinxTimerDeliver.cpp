#include "ZinxTimerDeliver.h"

#include <iostream>

using namespace std;

#define ZINX_TIMER_WHEEL_SIZE 8

ZinxTimerDeliver ZinxTimerDeliver::m_single;

//构造函数
ZinxTimerDeliver::ZinxTimerDeliver()
{
	//改变容器中可存储元素的个数 
	m_TimerWheel.resize(ZINX_TIMER_WHEEL_SIZE);
}


//析构函数
ZinxTimerDeliver::~ZinxTimerDeliver()
{

}

//注册任务
bool ZinxTimerDeliver::RegisterProcObject(TimerOutProc & _proc)
{
	int tmo = 0;
	int index = 0;
	int count = 0;
	WheelNode node;

	//获取超时的时间
	tmo = _proc.GetTimerSec();
	if (tmo <= 0)
	{
		return false;
	}

	//计算时间轮子的格子
	//格子数 = (当前刻度 + 超时的时间) % 时间轮子格子数
	index = (m_cur_index + tmo) % m_TimerWheel.size();

	//计算圈数
	//圈数 = 超时的时间 / 时间轮子格子数
	count = tmo / m_TimerWheel.size();

	//圈数
	node.LastCount = count;
	//超时任务指针
	node.pProc = &_proc;
	cout << "注册任务： LastCount: " << node.LastCount << endl;

	//添加到vector容器中
	m_TimerWheel[index].push_back(node);

	return true;
}

//注销任务
void ZinxTimerDeliver::UnRegisterProcObject(TimerOutProc & _proc)
{
	//遍历vector
	for (auto & l : m_TimerWheel)
	{
		for (auto it = l.begin(); it != l.end(); )
		{
			if (it->pProc == &_proc)
			{
				it = l.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

}

//调度实现
IZinxMsg * ZinxTimerDeliver::InternalHandle(IZinxMsg & _oInput)
{

	uint64_t tmo = 0;
	list<WheelNode> registerList;

	BytesMsg *pMsg = dynamic_cast<BytesMsg*>(&_oInput);
	pMsg->szData.copy((char*)&tmo, sizeof(tmo));

	//cout << "tmo: " << tmo << endl;

	for (uint64_t i = 0; i < tmo; i++)
	{
		//当前刻度自增
		m_cur_index = (m_cur_index + 1) % m_TimerWheel.size();

		cout << "刻度: " << m_cur_index << endl;

		//遍历当前格子中所有的任务，判断是否超时
		for (auto it = m_TimerWheel[m_cur_index].begin(); it != m_TimerWheel[m_cur_index].end(); )
		{
			//圈数减1
			it->LastCount--;
			if (it->LastCount < 0)
			{
				//调用超时任务处理函数
				it->pProc->Proc();

				//保存任务
				registerList.push_back(*it);

				it = m_TimerWheel[m_cur_index].erase(it);
			}
			else
			{
				it++;
			}
		}

		//重新注册任务
		for (auto it : registerList)
		{
			RegisterProcObject(*(it.pProc));
		}
	}

	//cout << "hello itcast" << endl;
	return nullptr;
}

AZinxHandler * ZinxTimerDeliver::GetNextHandler(IZinxMsg & _oNextMsg)
{
	return nullptr;
}
