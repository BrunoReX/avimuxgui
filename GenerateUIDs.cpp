#include "stdafx.h"
#include "matroska_ids.h"
#include "windows.h"
#include <wincrypt.h>

void generate_uid(char* pDest, int len)
{
	memset(pDest, 0, len);

	HCRYPTPROV hProv = 0;
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT|CRYPT_SILENT);
	CryptGenRandom(hProv, len, (unsigned char*)pDest);
	CryptReleaseContext(hProv, 0);
}