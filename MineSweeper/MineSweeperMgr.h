#pragma once

#include<map>
#include<vector>
#include<set>
#include <time.h>

struct Point
{
	Point()
	{
		x = 0;
		y = 0;
	}

	Point(int nX, int nY) :x(nX), y(nY) 
	{
	}

	bool operator < (const Point& point) const
	{
		if (y < point.y)
			return true;

		if (y == point.y && x < point.x)
			return true;

		return false;
	}

	bool operator == (const Point& point) const
	{
		if (x == point.x && y == point.y)
			return true;

		return false;
	}

	int x;
	int y;
};


struct MapLevelCnf
{
	int Level;
	int MaxMineCount;
	Point MaxPoint;
};

static const MapLevelCnf MapConfig[] =
{
	{1, 10, {10, 10}},
	{2,	40, {20, 20}},
	{3, 90, {30, 30}},
};


inline const MapLevelCnf* GetMapConfig(int nMapLevel)
{
	if (0 >= nMapLevel || sizeof(MapConfig) / sizeof(MapConfig[0]) < nMapLevel)
	{
		printf("don't fount map config by map level:%d\n", nMapLevel);
		return NULL;
	}

	return &MapConfig[nMapLevel - 1];
}


#define	LANDMINE	-1		//��


//��ͼ����״̬
enum MineSweeperStatus
{
	MSS_DEFAULT		= 0,	//Ĭ��û��״̬
	MSS_OPEN		= 1,	//��״̬
	MSS_FLAG		= 2,	//���״̬
};


//��������
struct PointInfo
{
	PointInfo()
	{
		Status	= MSS_DEFAULT;
		Content = 0;
	}

	int		Status;		//״̬
	int		Content;	//��������
};


//���Ӷ���
enum PointAction
{
	PA_OPEN		= 1,	//�򿪸���
	PA_FLAG,			//��Ǹ���
};

//ɨ�׹�����
class MineSweeperMgr
{
public:

	//ɨ�׵�ͼ ��������
	typedef	std::map<Point, PointInfo> MineSweepMap;

	MineSweeperMgr();
	virtual ~MineSweeperMgr();

	//������ͼ
	int CreateMap(int nMaxX, int nMaxY, int nMaxMine = 0);

	int CreateMap(int nMapLevel);

	//�����ͼ
	void PrintMap(bool IsShow = false);

	//�������
	int OnClickPoint(const Point& point, int nAction);

private:

	//��������и����е�����
	void CalcNumber();

	//��ȡ������Χ�ĸ���
	void GetPointAroundMine(const Point& rePoint, std::vector<Point>& vPoint);

	//������
	int OpenPoint(const Point& point);

private:
	Point					m_max_point;		//�������
	int						m_max_mine;			//��ͼ���������
	MineSweepMap			m_mine_sweeper_map;	//��ͼ����������
	std::set<Point>			m_mine_points;		//�����׵�����
	int						m_open_point;		//��ǰ�Ѵ򿪵���������
	int						m_start_time;		//��ʼʱ��
	int						m_end_time;			//����ʱ��
};