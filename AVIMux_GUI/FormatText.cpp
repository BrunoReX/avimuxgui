#include "stdafx.h"
#include "formattext.h"
#include "math.h"
#include "languages.h"
#include "splitpointsdlg.h"
#include "../dynarray.h"
#include "../strings.h"
#include <vector>

void FormatSize(char* d,__int64 qwSize)
{
	float fSizeOfSource;
	DWORD dwSizeOfSource;
	CString cStr;
	char Buffer[100];

	if ( qwSize>>10 <999) {
		cStr=LoadString(STR_KBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>10),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>10);
		fSizeOfSource = ((float)qwSize) / 1024;
	} else 
	if (qwSize>>20<999) {
		cStr=LoadString(STR_MBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>20),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>20);
		fSizeOfSource = (float)(qwSize) / 1024 / 1024;
	} else {
		cStr=LoadString(STR_GBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>30),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>20);
		fSizeOfSource = ((float)qwSize) / 1024 / 1024 / 1024;
	}

	DWORD	dwLen;
	dwLen = (DWORD)(min(log(fSizeOfSource)/log(10.),2));
	sprintf(Buffer,"%%%d.%df %%s",2,2-max(0,dwLen));
	sprintf(d,Buffer,fSizeOfSource,cStr);
}





__int64 SONChapStr2Millisec(char* c) 
{
	__int64		iRes = 0;
	__int64		i = 0;
	int			iColons = 0;
	char		d;

	while (d=*c++) {
		if (d!=0x20) {
			if (d >= '0' && d <= '9') {
				i=10*i+(d-48);
			} else
			if (d == ':') {
				if (iColons==2) {
					iRes *= 60000;
					iRes += 1000*i+atoi(c+1)*((atoi(c+1)>25)?33:40);
				} else {
					iRes *= 60;
					iRes += i;
				}
				i=0;
				iColons++;
			} else return STRF_ERROR;
		}
	}

	return (iColons==3)?iRes:-1;
}

int String2SplitPointDescriptor(char* s, SPLIT_POINT_DESCRIPTOR* pDesc)
{
	bool bEnd;

	pDesc->cOrgText = new CStringBuffer(s);

	char* c[2];
	c[0] = s;
	c[1] = s;
	while (*c[1] && *c[1]++ != '-');
	if (!*c[1]) bEnd = false; else bEnd = true;

	(*(c[1]-!!bEnd)) = 0;

	if (!pDesc->aChapBegin) pDesc->aChapBegin = new CDynIntArray;
	if (!pDesc->aChapEnd) pDesc->aChapEnd = new CDynIntArray;

	pDesc->iFlags = 0;

	CDynIntArray* a[2] = { pDesc->aChapBegin, pDesc->aChapEnd };
	a[0]->DeleteAll();
	a[1]->DeleteAll();
	__int64* i[2] = { &pDesc->iBegin, &pDesc->iEnd };

	for (int j=0;j<1+!!bEnd;j++) {
		char* cCurr;

		// set begin / end flag
		pDesc->iFlags |= (j)?SPD_END:SPD_BEGIN;
		
		cCurr = getword(&c[j]);
		if (!strcmp(cCurr,"chapter")) {
			cCurr = getword(&c[j]);
			pDesc->iFlags |= (j)?SPD_ECHAP:SPD_BCHAP;
			int  k=0;
			while (cCurr[k]) {
				bool bDone = true;
				while (cCurr[k]) {
					if (cCurr[k] != '.') k++; else {
						cCurr[k] = 0;
						bDone = false;
					}
				}
				if (!strcmp(cCurr, "*")) {
					a[j]->Insert(-2);
				} else {
					a[j]->Insert(atoi(cCurr)-1);
				}
				cCurr+=strlen(cCurr)+1*!bDone;
				k=0;
			}

			// use chapter as point
		} else 
		if (!strcmp(cCurr,"frame"))  {
			cCurr = getword(&c[j]);
			*(i[j]) = atoi(cCurr);
			pDesc->iFlags |= (j)?SPD_EFRAME:SPD_BFRAME;
		} else {
			__int64 ms = Str2Millisec(cCurr);
			if (ms!=-1) {
				*(i[j]) = Str2Millisec(cCurr) * 1000000;
			} else {
				return -1;
			}
		} 
	}

	return 1;
}

int remove_spaces(char* c)
{
	char* from = c;
	char* to = c;

	while (*from) {
		if (*from == ' ') *from++; else *to++ = *from++;
	}

	*to++ = 0;

	return 1;
}

int separate_at(char* in, char* in_sep, char* out1, char* out2, int force_separator = 0)
{
	char* c;

	if (!in || !*in)
		return 0;

	// no comma found -> no cropping string
	if (!(c=strstr(in, in_sep))) {
		strcpy(out1, in);
		out2[0] = 0;

		return force_separator;
	}

	*c++ = 0;

	// input_string ends on a comma -> ignore that
	if (!*c) {
		strcpy(out1, in);
		out2[0] = 0;

		return 1;
	}

	strcpy(out1, in);
	strcpy(out2, c);

	return 1;
}
/*
int separate(char* in, char* separator, std::vector<char*>& dest)
{
	char* c = in;
	int i = 0;

	do  {
		c=strstr(in, separator);
		if (c)
			*c++ = 0;

		dest.push_back(strdup(in));
		in = c;
		i++;
	} while (in);

	return i;
}
/*
void DeleteStringVector(std::vector<char*>& dest)
{
	std::vector<char*>::iterator iter = dest.begin();
	for (; iter != dest.end(); iter++)
		delete (*iter);
}
*/
// in    : input_string of format <resolution_string>,<cropping_string>
// out1  : resolution_string
// out2  : cropping_string
int decompose_entire_string(char* in, char* out1, char* out2)
{
	return separate_at(in, ",", out1, out2, 1);
}

// decompose resolution string to width, height and separator
int decompose_resolution(char* in, char* out_w, char* out_h, char* out_separator)
{
	if (!in || !*in)
		return 0;

	char* sep = NULL;
	if ((sep = strstr(in, "x")) || (sep = strstr(in, "X")) || (sep = strstr(in, ":"))) {
		out_separator[0] = *sep;

		*sep++ = 0;
		strcpy(out_w, in);
		strcpy(out_h, sep);

		return 1;
	}

	return 0;
}

// decompose cropping string to top-left, bottom-right, [/(
int decompose_cropping_1(char* in, char* topleft, char* bottomright, char* type)
{
	if (!separate_at(in, "-", topleft, bottomright))
		return 0;

	if (topleft[0] == '[') {
		if (topleft[strlen(topleft)-1] != ']' ||
			bottomright[0] != '[' ||
			bottomright[strlen(bottomright)-1] != ']')
			return 0;

		*type = '[';
		return 1;
	}

	if (topleft[0] == '(') {
		if (topleft[strlen(topleft)-1] != ')' ||
			bottomright[0] != '(' ||
			bottomright[strlen(bottomright)-1] != ')')
			return 0;

		*type = '(';
		return 1;
	}

	return 0;
}

int Str2Resolution(char* c, int in_x, int in_y, RESOLUTION* r, RESOLUTION* r_out)
{

	char res[128]; memset(res, 0, sizeof(res));
	char crop[128]; memset(crop, 0, sizeof(crop));
	char res_w[128]; memset(res_w, 0, sizeof(res_w));
	char res_h[128]; memset(res_h, 0, sizeof(res_h));
	char res_sep[16]; memset(res_sep, 0, sizeof(res_sep));
	char crop_tl[128]; memset(crop_tl, 0, sizeof(crop_tl));
	char crop_br[128]; memset(crop_br, 0, sizeof(crop_br));
	char crop_t[16]; memset(crop_t, 0, sizeof(crop_t));
	char crops[4][128]; memset(crops, 0, sizeof(crops));

	if (!r_out)
		r_out = r;
	else
		memcpy(r_out, r, sizeof(*r));

	remove_spaces(c);

	if (!decompose_entire_string(c, res, crop))
		return STRF_ERROR;	


	if (crop[0]) {

		if (!decompose_cropping_1(crop, crop_tl, crop_br, crop_t))
			return STRF_ERROR;

		crop_br[strlen(crop_br)-1] = 0;
		crop_tl[strlen(crop_tl)-1] = 0;

		if (!separate_at(crop_tl+1, ",", crops[0], crops[1]))
			return STRF_ERROR;

		if (!separate_at(crop_br+1, ",", crops[2], crops[3]))
			return STRF_ERROR;


		if (crop_t[0] == '[') {
			r_out->rcCrop.left   = atoi(crops[0]);
			r_out->rcCrop.top    = atoi(crops[1]);
			r_out->rcCrop.right  = atoi(crops[2]);
			r_out->rcCrop.bottom = atoi(crops[3]);
		}

		if (crop_t[0] == '(') {
			r_out->rcCrop.left   = atoi(crops[0]);
			r_out->rcCrop.top    = atoi(crops[1]);
			r_out->rcCrop.right  = in_x - atoi(crops[2]);
			r_out->rcCrop.bottom = in_y - atoi(crops[3]);
		}
	}

	if (res[0]) {
		if (!decompose_resolution(res, res_w, res_h, res_sep))
			return STRF_ERROR;

		if (res_sep[0] != ':') {
			if ((atoi(res_h) == 0 || atoi(res_w) == 0) && res_sep[0] != ':')
				return STRF_ERROR;
		} else {
			if (atof(res_h) < 0.001 || atof(res_w) < 0.001)
				return STRF_ERROR;
		}

		if (res_sep[0] == 'x' || res_sep[0] == 'X') {
			r_out->iWidth = atoi(res_w);
			r_out->iHeight = atoi(res_h);
		} else if (res_sep[0] == ':') {
			r_out->iWidth = (int)(in_y*atof(res_w)/atof(res_h));
			r_out->iHeight= in_y;
		}
	} else {
		r_out->iWidth = r->iWidth;
		r_out->iHeight= r->iHeight;
	}

	return STRF_OK;


/*	int i,j;
	i = 0; j = 0;
	bool bAspect_Ratio = false;
	char s[2][20];
	ZeroMemory(s,sizeof(s));

	while (*c) {
		if (*c == 'x' || *c == 'X') {
			i = 1; j = 0;
		} else
		if (*c == ':') {
			bAspect_Ratio = true;
			i = 1;
			j = 0;
		} else
		if ((*c >= '0' && *c <= '9') || *c == '.' || *c == ',') {
			s[i][j++] = *c;
			
		} else {
			return STRF_ERROR;
		}
		*c++;
	}

	if (atof(s[0])>0.001 && atof(s[1])>0.001) {

		if (!bAspect_Ratio) {
			r->iWidth = atoi(s[0]);
			r->iHeight = atoi(s[1]);
		} else {
			if (atof(s[1]) < 0.0001) strcpy(s[1],"1");
			r->iWidth = (int)(r->iHeight*atof(s[0])/atof(s[1]));
		}
	}

	return (i == 1 && r->iHeight && r->iWidth)?STRF_OK:STRF_ERROR;
	*/


}