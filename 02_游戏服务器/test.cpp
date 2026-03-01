#include <iostream>

#include "zinx.h"
#include "ZinxTimerDeliver.h"
#include "ZinxTimer.h"
#include "ZinxTCP.h"
#include "GameChannel.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#include "msg.pb.h"

using namespace std;

class TimeOut :public TimerOutProc
{

	// 通过 TimerOutProc 继承
	virtual void Proc() override
	{
		auto players = ZinxKernel::Zinx_GetAllRole();

		if (players.size() == 0)
		{
			//退出框架
			ZinxKernel::Zinx_Exit();
		}

	}
	virtual int GetTimerSec() override
	{
		return 30;
	}
};

//守护进程 + 监视进程
void Daemon()
{
	int ret = -1;
	int status = 0;
	pid_t pid = -1;

	int fd = -1;



	//打开或者创建一个文件
	fd = open("./game.log", O_RDWR | O_CREAT | O_APPEND, 0644);
	if (-1 == fd)
	{
		perror("open");
		return;
	}

	//文件描述符重定向
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	close(fd);

	//创建子进程 父进程退出
	pid = fork();
	if (-1 == pid)
	{
		perror("fork");
		return;
	}
	else if (pid > 0)
	{
		//父进程退出
		exit(0);
	}

	//子进程
	//创建会话
	ret = setsid();
	if (-1 == ret)
	{
		perror("setsid");
		return;
	}


	//父进程： 监视进程  子进程： 执行任务的进程
	while (1)
	{
		pid = fork();
		if (-1 == pid)
		{
			perror("fork");
			break;
		}
		else if (pid > 0)
		{
			wait(&status);

			//表示正常退出
			if (WIFEXITED(status))
			{
				if (WEXITSTATUS(status) == 0)
				{
					exit(0);
				}
			}
			else
			{
				printf("子进程非正常退出，创建新的子进程\n");
				//非正常退出
				continue;
			}
		}
		else
		{
			//子进程执行任务
			break;
		}
	}
}

//监视标准输入
int main(int argc, char **argv)
{
	if ((2 == argc) && ("daemon" == string(argv[1])))
	{
		Daemon();
	}

	TimeOut tOut;

	//注册
	ZinxTimerDeliver::GetInstance().RegisterProcObject(tOut);

	ZinxTimer *timer = new ZinxTimer;


	ZinxTCPListen *tcpListen = new ZinxTCPListen(10086, new TcpConnFactory);


	//1. 初始化  调用epoll_create
	ZinxKernel::ZinxKernelInit();

	//2. 添加通道 上树
	ZinxKernel::Zinx_Add_Channel(*timer);
	ZinxKernel::Zinx_Add_Channel(*tcpListen);

	//3. 执行 调用epoll_wait
	ZinxKernel::Zinx_Run();

	//4. 释放资源 调用close
	ZinxKernel::ZinxKernelFini();


	/*
	下面是Google给出的建议原文翻译：

		性能提示
		在性能上，我们还可以做一些额外的工作，使得 Protocol Buffers 更加高效，其中关键的一个问题就是内存管理。
		我们可以尽量的复用 Protocol Buffers message 类，我们对 message 进行了 clear 之后，message 的内存并不会被释放而可以直接重新使用（也就避免了再次分配内存的开销）。message 的内存的使用情况可以通过函数 SpaceUsed 获取。
		另外，你也可以尝试使用其他的内存分配器，例如 Google’s tcmalloc，它对多线程中分配大量小对象有很好的优化。

	*/
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}