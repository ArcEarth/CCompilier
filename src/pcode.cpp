#include "stdafx.h"
#include "pcode.h"

size_t C1::PCode::CodeDome::EmplaceString(const std::string &str_data)
{
	auto offset = DataSegment.size();
	auto size = str_data.size();
	for (size_t i = 0; i < size; i++)
	{
		word data;
		data.i = str_data[i];
		DataSegment.push_back(data);
	}
	word data;
	data.i = 0;
	DataSegment.push_back(data);
	return offset;
}

size_t C1::PCode::CodeDome::EmplaceFloat(float data)
{
	auto offset = DataSegment.size();
	word w;
	w.f = data;
	DataSegment.push_back(w);
	return offset;
}

size_t C1::PCode::CodeDome::EmplaceDataBlcok(size_t size_in_byte, char* data)
{
	auto offset = DataSegment.size();
	if (!data)
		DataSegment.resize(DataSegment.size() + size_in_byte);
	else
	for (size_t i = 0; i < size_in_byte; i++)
	{
		word w;
		w.i = data[i];
		DataSegment.push_back(w);
	}
	return offset;
}

size_t C1::PCode::CodeDome::EmplaceInteger(int data)
{
	auto offset = DataSegment.size();
	word w;
	w.i = data;
	DataSegment.push_back(w);
	return offset;
}

C1::PCode::CodeDome::CodeDome()
{
	SP = CallStorageSize;
}

size_t C1::PCode::CodeDome::EmplaceInstruction(fct f, long l, long a)
{
	return PushInstruction(instruction{ f, l, a });
}

size_t C1::PCode::CodeDome::PushInstruction(const instruction& ir)
{
	auto index = CodeSegment.size();
	if ((ir.f == lar) && (CodeSegment.back().f == lit) && (!ir.l) && (!ir.a))
	{
		CodeSegment.back().f = lod;
		return index-1;
	}	//A optimizing
	if ((ir.f == sar) && (CodeSegment.back().f == lit) && (!ir.l) && (!ir.a))
	{
		CodeSegment.back().f = sto;
		return index-1;
	}	//A optimizing
	if ((ir.f == cal) && (ir.l == 1) && (CodeSegment.back().f == lit) && (CodeSegment.back().l == 0))
	{
		CodeSegment.back().f = cal;
		CodeSegment.back().a += ir.a;
		return index - 1;
	}	//A optimizing
	if ((ir.f == isp) && (!ir.l) && (!ir.a))
	{
		return index;
	}
	CodeSegment.push_back(ir);
	return index;
}
