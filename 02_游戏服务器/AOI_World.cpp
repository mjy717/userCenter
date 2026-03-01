#include "AOI_World.h"

using namespace std;



int AOI_World::Xwidth()
{

	return maxX - minX;

}

int AOI_World::Ywidth()
{
	return maxY - minY;
}

//构造函数
AOI_World::AOI_World(int _minx, int _maxx, int _miny, int _maxy, int _xcnt, int _ycnt)
{
	this->minX = _minx;
	this->maxX = _maxx;
	this->minY = _miny;
	this->maxY = _maxy;
	this->Xcnt = _xcnt;
	this->Ycnt = _ycnt;

	//预留空间
	this->m_grids.reserve(_xcnt * _ycnt);
	for (int i = 0; i < _xcnt * _ycnt; i++)
	{
		this->m_grids.push_back(new AOI_Grid(i));
	}
}

//析构函数
AOI_World::~AOI_World()
{
	//释放空间
	for (auto it : m_grids)
	{
		delete it;
	}
}

//获取周围玩家
std::list<AOI_Player*> AOI_World::GetSurPlayers(AOI_Player * _player)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//存储周围所有的玩家的容器
	std::list<AOI_Player*> retPlayers;

	//计算当前玩家所在的行和列
	row = (_player->GetY() - this->minY) / ((this->maxY - this->minY) / this->Ycnt);
	col = (_player->GetX() - this->minX) / ((this->maxX - this->minX) / this->Xcnt);

	//周围九个格子的行号和列号的组合
	pair<int, int> row_col[] = {
		make_pair(row - 1, col - 1),
		make_pair(row - 1, col),
		make_pair(row - 1, col +1),

		make_pair(row, col - 1),
		make_pair(row, col),
		make_pair(row, col + 1),

		make_pair(row + 1, col - 1),
		make_pair(row + 1, col),
		make_pair(row + 1, col + 1)
	};

	for (auto it : row_col)
	{
		//判断行号是否合法
		if (it.first < 0 || it.first >= this->Ycnt)
		{
			continue;
		}

		//判断列是否合法
		if (it.second < 0 || it.second >= this->Xcnt)
		{
			continue;
		}

		//计算当前格子数
		index = it.first * this->Xcnt + it.second;

		for (auto p : m_grids[index]->m_players)
		{
			retPlayers.push_back(p);
		}
	}

	return retPlayers;
}

//添加玩家到地图中
void AOI_World::AddPlayer(AOI_Player * _player)
{
	int index = 0;

	//计算玩家所在的格子数
	index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	//判断格子数是否合法
	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "玩家所在的格子数非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.push_back(_player);

}

//从地图中删除玩家
void AOI_World::DelPlayer(AOI_Player * _player)
{
	int index = 0;

	//计算玩家所在的格子数
	index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	//判断格子数是否合法
	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "玩家所在的格子数非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.remove(_player);
}


//判断玩家是否跨格子 如果是跨格子 返回true
bool AOI_World::GridChanged(AOI_Player * _player, int _newX, int _newY)
{
	int oldIndex = 0;
	int newIndex = 0;

	//获取玩家之前所在格子
	oldIndex = Calculate_grid_idx(_player->GetX(), _player->GetY());

	//获取玩家目前所在的格子
	newIndex = Calculate_grid_idx(_newX, _newY);


	return oldIndex != newIndex;
}

//根据玩家的坐标 判断玩家属于第几个格子
int AOI_World::Calculate_grid_idx(int x, int y)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//计算玩家所在的行号
	row = (y - this->minY) / ((this->maxY - this->minY) / this->Ycnt);
	//计算玩家所在的列号
	col = (x - this->minX) / ((this->maxX - this->minX) / this->Xcnt);

	if (row < 0 || row >= this->Ycnt || col < 0 || col >= this->Xcnt)
	{
		cout << "玩家的所在的格子非法" << endl;
		return -1;
	}

	index = row * this->Xcnt + col;

	return index;
}


//-----------------------------------------
//构造函数
AOI_Grid::AOI_Grid(int _gid):iGID(_gid)
{
}

//析构函数
AOI_Grid::~AOI_Grid()
{
}
