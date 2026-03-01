#pragma once
#pragma once
#pragma once
#include <deque>
#include <string>

//随机姓名类
class RandomName
{
public:
	static RandomName &GetInstance();
	RandomName();
	~RandomName();

    //加载文件
	void LoadFile();

	//获取一个名字
	std::string GetName();

	//释放名字
	void ReleaseName(std::string szName);

private:
	static RandomName smInstance;
	std::deque<std::string> m_names;
};
