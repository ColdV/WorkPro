#pragma once

#include "../stdafx.h"
#include "../template/Single.h"

class Package : public Single<Package>
{
public:
	Package() {}
	virtual ~Package() {}

	//װ��(ͷ�����Լ���ʾ����С)
	//@param return ����װ����Ĵ�С 
	//�������ֵ����pkgSize��˵�������pkg�ռ䲻��
	//0��ʾװ��ʧ��
	uint32 Pack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize);

	//���
	//@param return �����ѽ���Ĵ�С 
	//�������ֵ����pkgSize��˵�������pkg�ռ䲻��
	//�������ֵС��msgSize��װ���ɹ�����ʣ�µ���������һ����
	//0��ʾ���ʧ��
	uint32 Unpack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize);
};

//#define Pack(msg, msgSize, pkg, pkgSize) \
//Package::Instance().Pack(msg, msgSize, pkg, pkgSize);
//
//#define Unpack(msg, msgSize, pkg, pkgSize) \
//Package::Instance().Unpack(msg, msgSize, pkg, pkgSize);