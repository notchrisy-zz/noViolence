#pragma once

class INetChannel
{
public:
	__int32 vtable; //0x0000 
	void* msgbinder1; //0x0004 
	void* msgbinder2; //0x0008 
	void* msgbinder3; //0x000C 
	void* msgbinder4; //0x0010 
	unsigned char m_bProcessingMessages; //0x0014 
	unsigned char m_bShouldDelete; //0x0015 
	char pad_0x0016[0x2]; //0x0016
	__int32 m_nOutSequenceNr; //0x0018 
	__int32 m_nInSequenceNr; //0x001C 
	__int32 m_nOutSequenceNrAck; //0x0020 
	__int32 m_nOutReliableState; //0x0024 
	__int32 m_nInReliableState; //0x0028 
	__int32 m_nChokedPackets; //0x002C 
};//Size=0x4294


class CClientState
{
public:
	char pad_0000[156];             //0x0000
	INetChannel* m_NetChannel;          //0x009C

	void ForceFullUpdate()
	{
		*reinterpret_cast<int*>(std::uintptr_t(this) + 0x0174) = -1;
	}
};
