#ifndef I_CRC32
#define I_CRC32

const unsigned int CRC32_POLYNOM  = 0x04C11DB7;
const unsigned int CD_ECD_POLYNOM = 0x8001801B;
const unsigned int CD_ECD_POLYNOM_BW = 0xB1081008;

//unsigned __int32 CRC32(void* pData, int iLength, unsigned int iPolynom);
unsigned __int32 CRC32(unsigned char* pData, int iLength);
void CRC32_ogg(unsigned char* pData, int iLength);


#endif
