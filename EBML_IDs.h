#ifndef I_EBMLIDS
#define I_EBMLIDS

typedef struct {
	char* cName;
	char* cID;
} EID_DESCRIPTOR;

#pragma warning(disable:4305)
#pragma warning(disable:4309)

extern const int VSizeInt_Len[256];
extern const __int64 undefined_lengths[];

const char EID_EBML[] = { 0x1A, 0x45, 0xDF, 0xA3 };
const char EID_EBMLVersion[] = { 0x42, 0x86 };
const char EID_EBMLReadVersion[] = { 0x42, 0xF7 };
const char EID_EBMLMaxIDLength[] = { 0x42, 0xF2 };
const char EID_EBMLMaxSizeLength[] = { 0x42, 0xF3 };
const char EID_DocType[] = { 0x42, 0x82 };
const char EID_DocTypeVersion[] = { 0x42, 0x87 };
const char EID_DocTypeReadVersion[] = { 0x42, 0x85 };

const int EBMLTYPE_UNKNOWN				= -0x01;
const int EBMLTYPE_EBML					= 0x01;
const int EBMLTYPE_EBMLVersion			= 0x02;
const int EBMLTYPE_DOCTYPE				= 0x03;
const int EBMLTYPE_EBMLReadVersion		= 0x04;
const int EBMLTYPE_EBMLMaxIDLength		= 0x05;
const int EBMLTYPE_EBMLMaxSizeLength	= 0x06;
const int EBMLTYPE_DOCTYPEVERSION		= 0x07;
const int EBMLTYPE_DOCTYPEREADVERSION	= 0x08;

#endif