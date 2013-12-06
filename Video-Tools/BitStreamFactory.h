#ifndef I_BITSTREAM_FACTORY
#define I_BITSTREAM_FACTORY

#include "BitStream.h"

class CBitStreamFactory
{
public:
	template<class T>
	static IBitStream* CreateInstance();
};


#endif