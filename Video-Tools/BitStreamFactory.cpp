#include "stdafx.h"
#include "BitStreamFactory.h"

template<>
IBitStream* CBitStreamFactory::CreateInstance<CBitStream2>()
{
	return new CBitStream2();
}

template<>
IBitStream* CBitStreamFactory::CreateInstance<BITSTREAM>()
{
	return new BITSTREAM();
}