#pragma once
#include "zinx.h"

#include <vector>
#include <list>

#include <iostream>

//超时任务抽象类
class TimerOutProc {
public:
	TimerOutProc() {}
	virtual ~TimerOutProc() {};
	virtual void Proc() = 0;                //超时的时候会去调用的接口
	virtual int GetTimerSec() = 0;          //获取当前任务的超时间隔，一秒触发一次就返回1
};

//节点类型
struct WheelNode {
	int LastCount = -1;						//当前任务的圈数
	TimerOutProc *pProc = NULL;				//超时任务的指针
};


//继承自处理者类,用来作为一个超时任务的调度器,其中维护一个时间轮子
class ZinxTimerDeliver :public AZinxHandler
{
public:
	ZinxTimerDeliver();
	~ZinxTimerDeliver();

	//单例的获取函数
	static ZinxTimerDeliver &GetInstance() {
		return m_single;
	}
	//注册一个Timer处理类对象
	bool RegisterProcObject(TimerOutProc &_proc);
	//注销一个Timer处理类对象
	void UnRegisterProcObject(TimerOutProc &_proc);

	// 通过 AZinxHandler 继承
	virtual IZinxMsg * InternalHandle(IZinxMsg & _oInput) override;
	virtual AZinxHandler * GetNextHandler(IZinxMsg & _oNextMsg) override;

private:
	//单例
	static ZinxTimerDeliver m_single;

	//当前轮子刻度
	int m_cur_index = 0;

	//时间轮向量，每个格子中放一个list，list元素是圈数和定时器节点
	std::vector<std::list<WheelNode>> m_TimerWheel;
};
