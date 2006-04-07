#include "stdafx.h"
#include "RIFFFile.h"
//#include "vfw.h"
#include "AVIStructs.h"

DWORD MakeFourCC (char* lpFourCC)
{
	return (MKFOURCC(lpFourCC[0],lpFourCC[1],lpFourCC[2],lpFourCC[3]));
}

				//////////////////////////
				//  RIFFFILE - Methoden //
				//////////////////////////

RIFFFILE::RIFFFILE(void)
{
}

RIFFFILE::~RIFFFILE(void)
{
}

bool RIFFFILE::LocateData(DWORD dwFourCC,char** lpBuffer,DWORD* dwParentPos,void* lpDest,DWORD dwLength,DWORD dwType)
{
	union
	{
		CHUNKHEADER		chChunkHdr;
		LISTHEADER		lhListHdr;
	};
	DWORD			dwPos=0;
	CHUNKHEADER* 	lpchDest=(CHUNKHEADER*)lpDest;
	LISTHEADER*		lplhDest=(LISTHEADER*)lpDest;
	DWORD*			lpdwDestFourCC;
	bool			GetDataFromFile=false;
	DWORD			dwRead;
	char			Buffer[300];
	int				isizeinfile=0;

	GetDataFromFile=(!lpBuffer);
	if (!GetDataFromFile) GetDataFromFile=(!*lpBuffer);

	lpdwDestFourCC=(dwType==DT_CHUNK)?&(lpchDest->dwFourCC):&(lplhDest->dwFourCC);
	*lpdwDestFourCC=0;

	while ((dwPos<dwLength-7)&(!*lpdwDestFourCC))
	{
		if (GetDataFromFile)
		{
			dwRead=source->Read(&chChunkHdr,8);
		}
		else
		{
			chChunkHdr=*(CHUNKHEADER*)(*lpBuffer);
			(*lpBuffer)+=8; 
		}
		dwPos+=8;
//		irealsize=chChunkHdr.dwLength;
		isizeinfile = chChunkHdr.dwLength + ((chChunkHdr.dwLength%2)?1:0);
		//chChunkHdr.dwLength+=(chChunkHdr.dwLength%2)?1:0;
		if (dwType==DT_CHUNK)
		{
			if (chChunkHdr.dwFourCC==dwFourCC)
			{
			//	int k = chChunkHdr.dwLength;
			//	chChunkHdr.dwLength = irealsize;
				*((CHUNKHEADER*)lpchDest)=chChunkHdr;
			//	chChunkHdr.dwLength = k;
			}
			else
			{
				if (!GetDataFromFile) 
				{
					//(*lpBuffer)+=chChunkHdr.dwLength; 
					(*lpBuffer)+=isizeinfile;
				}
				else
				{
					//source->Seek(source->GetPos()+chChunkHdr.dwLength);
					source->Seek(source->GetPos()+isizeinfile);
				}
				//dwPos+=chChunkHdr.dwLength;
				dwPos+=isizeinfile;
			}		
		}
		else
		{
			ZeroMemory(Buffer,sizeof(Buffer));
			*((DWORD*)Buffer)=lhListHdr.dwListID;

			if  ( ((lhListHdr.dwListID==MakeFourCC("LIST"))&&(dwType==DT_LIST))||
				  ((lhListHdr.dwListID==MakeFourCC("RIFF"))&&(dwType==DT_RIFF)))
			{
				if (GetDataFromFile)
				{
					dwRead=source->Read(&(lhListHdr.dwFourCC),4);
				}
				else
				{
					lhListHdr.dwFourCC=*(DWORD*)(*lpBuffer);
					(*lpBuffer)+=4;
				}
				dwPos+=4;
				*((DWORD*)&(Buffer[4]))=lhListHdr.dwFourCC;
				if (lhListHdr.dwFourCC==dwFourCC)
				{
					*((LISTHEADER*)lplhDest)=lhListHdr;
				}
				else
				{
					if (!GetDataFromFile) 
					{
						(*lpBuffer)+=lhListHdr.dwLength-4;
					}
					else
					{
						//source->Seek(source->GetPos()+lhListHdr.dwLength-4);
						source->Seek(source->GetPos()+isizeinfile-4);
						
					}
					//dwPos+=lhListHdr.dwLength-4;
					dwPos+=isizeinfile-4;
				}
			}
			else
			{
				if (!GetDataFromFile) 
				{
					//(*lpBuffer)+=lhListHdr.dwLength;
					(*lpBuffer)+=isizeinfile;
				}
				else
				{
					//source->Seek(source->GetPos()+lhListHdr.dwLength);
					source->Seek(source->GetPos()+isizeinfile);
				}
				//dwPos+=lhListHdr.dwLength;
				dwPos+=isizeinfile;
			}
		}
	}
	if (dwParentPos) (*dwParentPos)+=dwPos;
	return (*lpdwDestFourCC)?true:false;
}

int RIFFFILE::InvalidateCache()
{
	return source->InvalidateCache();
}

				/////////////////////////////
				//  LISTELEMENT - Methoden //
				/////////////////////////////

LISTELEMENT::LISTELEMENT(void)
{
	lpNext = NULL;
	lpLast = NULL;
	dwFourCC = 0;
}

LISTELEMENT::~LISTELEMENT(void)
{

};

DWORD LISTELEMENT::GetSize(DWORD dwFlags)
{
	return (1);
}

void LISTELEMENT::SetNext(LISTELEMENT* _lpNext)
{
	lpNext=_lpNext;
}

void LISTELEMENT::SetFourCC(DWORD _dwFourCC)
{
	dwFourCC=_dwFourCC;
}

void LISTELEMENT::FreeData(DWORD dwFlags)
{
}

void* LISTELEMENT::Store(void* lpDest,DWORD dwFlags)
{
	return lpDest;
}

void LISTELEMENT::StoreToStream(STREAM* lpstrDest,DWORD dwFlags)
{
	return;
}


				//////////////////////
				//  LIST - Methoden //
				//////////////////////

LIST::LIST(void)
{
	lpData = NULL;
	dwLastSize=0;
}

LIST::~LIST(void)
{
}

DWORD LIST::GetSize(DWORD dwFlags)
{
	DWORD dwRes=12;
	dwRes+=(lpData)?lpData->GetSize(LE_CHAIN):0;
	if (dwFlags&LE_CHAIN) dwRes+=(lpNext)?lpNext->GetSize(dwFlags):0;
	return (dwRes);
}

void LIST::FreeData(DWORD dwFlags)
{
	LIST*	lpCurr=(LIST*)this;

	if (dwFlags&LE_CHAIN)
	{
		if (lpNext)
		{
			lpNext->FreeData(LE_CHAIN);
			delete lpNext;
			lpNext=NULL;
		}
		if (lpCurr->lpData) 
		{
			lpData->FreeData(LE_CHAIN);
			delete lpData;
			lpData=NULL;
		}
	}
}

void* LIST::Store(void* lpDest,DWORD dwFlags)
{
	DWORD*	lpdwRes=(DWORD*)lpDest;
	DWORD	dwSize=GetSize(LE_SINGLEELEMENT)-8;

	*lpdwRes++=MakeFourCC("LIST");
	*lpdwRes++=dwSize;
	*lpdwRes++=dwFourCC;

	if (lpData) lpdwRes=(DWORD*)lpData->Store(lpdwRes,LE_CHAIN);
	if ((lpNext)&&(dwFlags&LE_CHAIN)) lpdwRes=(DWORD*)lpNext->Store(lpdwRes,LE_CHAIN);

	return (lpdwRes);
}

void LIST::StoreToStream(STREAM* lpstrDest,DWORD dwFlags)
{
	DWORD	dwSize;
	if (dwLastSize&&(dwFlags&LE_USELASTSIZE)) 
	{
		dwSize=dwLastSize; 
	}
	else
	{
		dwSize=GetSize(LE_SINGLEELEMENT)-8;
	}
	DWORD	dwX;
	if (dwFlags&LE_USELASTSIZE) dwLastSize=dwSize; else dwLastSize=0;

	dwX=MakeFourCC("LIST");
	lpstrDest->Write(&dwX,4);
	lpstrDest->Write(&dwSize,4);
	lpstrDest->Write(&dwFourCC,4);

	if (lpData) lpData->StoreToStream(lpstrDest,dwFlags);
	if ((lpNext)&&(dwFlags&LE_CHAIN)) lpNext->StoreToStream(lpstrDest,dwFlags);

	return;
}


void LIST::SetData(LISTELEMENT* lpNewData)
{
	lpData=lpNewData;
	dwLastSize=0;
}
				///////////////////////
				//  CHUNK - Methoden //
				///////////////////////

CHUNK::CHUNK(void)
{
	lpData = NULL;
	dwSize = 0;
	bValid=false;
	dwNextSize = 0;
}

CHUNK::~CHUNK(void)
{

}

void* CHUNK::GetData(void)
{
	return (lpData);
}

DWORD CHUNK::GetSize(DWORD dwFlags)
{
	DWORD dwRes=(dwSize+8)+(dwSize%2);
	if (dwFlags&LE_CHAIN) dwRes+=(lpNext)?lpNext->GetSize(dwFlags):0;
	return (dwRes);
}

void CHUNK::SetData(void* lpNewData, DWORD dwFlags, DWORD dwOffset)
{
	void** lplpNewData = (void**)lpNewData;
	bool   b = false;

	if (lpNewData)
	{
		if (!(dwFlags & CHUNKSD_OVERWRITE)) {
			if (lpData) free(lpData);
			lpData=NULL;
			b = true;
		}

		if (dwSize)
		{
/*			if (dwFlags & CHUNKSD_DONTCOPY)
			{
				realloc(*lplpNewData,dwSize);
				lpData=(CHUNK*)(*lplpNewData);
				(*lplpNewData)=NULL;
			}
			else
			{*/
				if (b) {
					lpData=(CHUNK*)malloc(dwSize + dwSize % 2);
				}
				BYTE*  lpbData = (BYTE*)lpData;
				memcpy(lpbData + dwOffset,lpNewData,dwNextSize);
//			}
		}
	}
}

void CHUNK::IncreaseSizeBy(DWORD dwAdditionalSize, DWORD* dwOldSize)
{
	if (dwOldSize) *dwOldSize = dwSize + (dwSize % 2);

	dwNextSize = dwAdditionalSize;
	dwAdditionalSize += (dwAdditionalSize % 2);

	if (!dwSize) {
		lpData = (CHUNK*)malloc(dwSize+=dwAdditionalSize);
	} else {
		int j = dwSize % 2;
		lpData = (CHUNK*)realloc(lpData, dwSize+=j+dwAdditionalSize);
	}

}

void CHUNK::SetSize(DWORD dwNewSize)
{
	dwSize=dwNewSize;
	dwNextSize = dwSize;

//	dwSize += (dwSize % 2);

	bValid=true;
}

void CHUNK::FreeData(DWORD dwFlags)
{
	CHUNK*			lpCurr=(CHUNK*)this;

	if (dwFlags&LE_CHAIN)
	{
		if (lpCurr->lpNext)
		{
				lpCurr->lpNext->FreeData(LE_CHAIN);
				delete lpCurr->lpNext;
				lpCurr->lpNext=NULL;
		}
		if (lpCurr->GetData()) 
		{
			lpCurr->FreeData(LE_SINGLEELEMENT);
		}
	}
	else
	{
		if (lpData)
		{
			free(lpData);
			dwSize=0;
			bValid=false;
			lpData=NULL;
		}
	}
}

void* CHUNK::Store(void* lpDest,DWORD dwFlags)
{
	DWORD*	lpdwRes=(DWORD*)lpDest;
	
	*lpdwRes++=dwFourCC;
	*lpdwRes++=dwSize;
	if (lpData)	memcpy(lpdwRes,lpData,dwSize); else ZeroMemory(lpdwRes,dwSize);
	lpdwRes=(DWORD*)((DWORD)lpdwRes+dwSize);
	if (dwSize%2) lpdwRes=(DWORD*)((DWORD)lpdwRes+1);

	return (dwFlags&LE_CHAIN)?(lpNext)?lpNext->Store((void*)lpdwRes,LE_CHAIN):lpdwRes:lpdwRes;
}

void CHUNK::StoreToStream(STREAM* lpstrDest,DWORD dwFlags)
{
	DWORD	dwX;
	void*	lpBuffer=NULL;
	
	lpstrDest->Write(&dwFourCC,4);
	lpstrDest->Write(&dwSize,4);
	if (lpData)
	{
		lpstrDest->Write(lpData,dwSize);
	}
	else
	{
		lpBuffer=malloc(dwSize);
		ZeroMemory(lpBuffer,dwSize);
		lpstrDest->Write(lpBuffer,dwSize);
	}
	dwX=0;
	if (dwSize%2) lpstrDest->Write(&dwX,1);

	if ((dwFlags&LE_CHAIN)&&(lpNext)) lpNext->StoreToStream(lpstrDest,dwFlags);

	if (lpBuffer) delete lpBuffer;

	return;
}

void CHUNK::Set(DWORD dwFourCC, DWORD dwSize, void* pData, void* pNext)
{
	SetFourCC(dwFourCC);
	SetSize(dwSize);
	SetData(pData);
	SetNext((LISTELEMENT*)pNext);
}