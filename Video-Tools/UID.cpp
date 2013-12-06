#include "stdafx.h"
#include "UID.h"
#include "windows.h"
#include "wincrypt.h"

#define UID_TYPE_UNDEFINED 0x00
#define UID_TYPE_UNSIGNED_INT 0x01
#define UID_TYPE_BYTE_ARRAY 0x02

CUID::CUID()
{
	uidType = UID_TYPE_UNDEFINED;
	uidInt = 0;
	uidByteArray = NULL;
	uidByteArraySize = 0;
}

CUID::~CUID()
{
	if (uidByteArray)
		delete uidByteArray;
}

CUID::CUID(unsigned char* uid, size_t size)
{
	uidType = UID_TYPE_BYTE_ARRAY;
	uidByteArray = new unsigned char[size];
	uidByteArraySize = size;
	memcpy(uidByteArray, uid, size);
}

CUID::CUID(char* uid, size_t size)
{
	CUID((unsigned char*)uid, size);
}

CUID::CUID(unsigned __int64 uid)
{
	uidInt = uid;
	uidType = UID_TYPE_UNSIGNED_INT;
}

CUID::CUID(__int64 uid)
{
	CUID(*(unsigned __int64*)&uid);
}

bool CUID::IsUIDIdentical(CUID* uid2)
{
	if (uidType != uid2->uidType)
		return false;

	if (uidType == UID_TYPE_UNSIGNED_INT)
		return !!(uidInt == uid2->uidInt);

	if (uidType == UID_TYPE_BYTE_ARRAY) {
		if (uidByteArraySize != uid2->uidByteArraySize)
			return false;

		for (int j=0; j<uidByteArraySize; j++)
			if (uidByteArray[j] != uid2->uidByteArray[j])
				return false;

		return true;
	}

	return false;
}

void CUID::SetUIDFlags(unsigned __int32 uidFlags)
{
	this->uidFlags = uidFlags;
}

unsigned __int32 CUID::GetUIDFlags()
{
	return this->uidFlags;
}

CUID::operator unsigned __int64()
{
	if (uidType == UID_TYPE_UNSIGNED_INT)
		return uidInt;
	else
		return 0;
}

CUID::operator unsigned char *()
{
	if (uidType == UID_TYPE_BYTE_ARRAY)
		return uidByteArray;

	return NULL;
}

CUID::operator char *()
{
	if (uidType == UID_TYPE_BYTE_ARRAY)
		return (char*)uidByteArray;

	return NULL;
}

/* Reinitialize an UID and reject a value of 0 as well as a
   byte array where all elements are 0
*/
void CUID::ReInit()
{
	if (uidType == UID_TYPE_UNSIGNED_INT) {
		generate_uid((char*)&uidInt, sizeof(uidInt));
		if (uidInt == 0)
			ReInit();
	}

	if (uidType == UID_TYPE_BYTE_ARRAY) {
		generate_uid((char*)uidByteArray, uidByteArraySize);
		for (int i=0; i<uidByteArraySize; i++)
			if (uidByteArray[i])
				return;

		ReInit();
	}
}

CHasUID::CHasUID()
{
	uid = NULL;
}

CHasUID::~CHasUID()
{
	delete uid;
}

void CHasUID::SetUID(CUID* uid)
{
	if (this->uid != NULL)
		delete this->uid;

	this->uid = uid;
}

CUID* CHasUID::GetUID()
{
	return uid;
}

void generate_uid(char* pDest, int len)
{
	memset(pDest, 0, len);

	HCRYPTPROV hProv = 0;
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT|CRYPT_SILENT);
	CryptGenRandom(hProv, len, (unsigned char*)pDest);
	CryptReleaseContext(hProv, 0);
}