#pragma once

#include "stdafx.h"

class Field : public NonCopyable
{
public:
	static const uint32 MAX_SIZE = 1024 * 1024 * 10;	//�������(10M)

	Field() :
		m_value(NULL),
		m_length(0)
	{
	}


	virtual ~Field()
	{
		if (m_value)
			delete[] m_value;

		m_length = 0;
	}


	//���������
	int Fill(const char* value, uint32 len)
	{
		if (MAX_SIZE <= len || 0 != m_length)
			return -1;

		m_value = new char[len + 1]{0};	//ĩβ׷��'\0'
		if (!m_value)
			return -1;

		memcpy(m_value, value, len);

		m_length = len;

		return 0;
	}


	/////////////////////��������ת��/////////////////////

	float Float() const { return Value() ? (float)atof(Value()) : 0; }

	double Double() const { return Value() ? atof(Value()) : 0; }

	bool Bool() const { return Value() ? atoi(Value()) != 0 : false; }

	uint8 Uint8() const { return Value() ? (uint8)atoi(Value()) : 0; }

	int8 Int8() const { return Value() ? (int8)atoi(Value()) : 0; }

	uint16 Uint16() const { return Value() ? (uint16)atoi(Value()) : 0; }

	int16 Int16() const { return Value() ? (int16)atoi(Value()) : 0; }

	uint32 Uint32() const { return Value() ? (uint32)strtoul(Value(), NULL, 10) : 0; }

	int32 Int32() const { return Value() ? (int32)atoi(Value()) : 0; }
	
	uint64 Uint64() const { return Value() ? strtoull(Value(), NULL, 10) : 0; }

	int64 Int64() const { return Value() ? atoll(Value()) : 0; }

	const char* String() const { return Value(); }

	uint32 Length() const { return m_length; }

private:
	const char* Value() const { return m_value ? NULL : &m_value[0]; }

private:
	char* m_value;
	uint32 m_length;
};



class DBResult : public NonCopyable
{
public:
	static const int DEFAULT_RESERVER = 128;		//Ĭ��Ԥ�����С
	static const Field s_nullfield;				//��Field����

	struct FieldInfo
	{
		std::string name;		//����
		uint32 index;			//���±�
	};

	typedef std::vector<std::vector<Field*>>	DBResultVec;
	typedef std::map<std::string, FieldInfo>	Name2FieldInfoMap;

	DBResult();
	virtual ~DBResult();

	//��ȡ���������
	uint32 GetRowCount() const { return m_result.size(); }

	//����ָ��������������ȡ��
	const Field& GetFieldByName(uint32 row, const std::string& name) const;

	//��ָ���������������
	//@parma row:�б�
	//@param field:�б�
	//@param value:��ֵ
	//@parma len:��ֵ����
	int AppendField(uint64 row, uint32 field, const char* value, uint32 len);

	//�����������еĹ�ϵ
	bool BuildName2Field(const std::string& name, const FieldInfo& field);

	//�ͷſռ�
	void Clear();

private:
	//����������ȡ���±�
	int Name2Index(const std::string& name) const
	{
		auto it = m_name2info.find(name);

		return it == m_name2info.end() ? -1 : it->second.index;
	}

private:
	DBResultVec m_result;
	Name2FieldInfoMap m_name2info;
};
