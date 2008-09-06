#include "stdafx.h"
#include "xmlfiles.h"

int FileIsXML(CTextFile* t)
{
	__int64 pos = t->GetPos();
	bool done = false;
	char c;
	int level = 0;

	t->Seek(0);

	while (!done) {
		t->Read(&c, 1);
		if (c != 0x20 && c != 0x0A && c != 0x0D) {

			if (c == '<') {
				level++;
				if (level == 1) {
					__int64 currentPos = t->GetPos();
					do
					{
						c = t->ReadChar<char>(TFRC_NOLINEBREAKS | TFRC_DONTUPDATEPOS);
					} while ((!c || c==0x0A || c==0x0D) && (!t->IsEndOfStream()));
					t->Seek(currentPos);
					if (c != '?' && c != '!') {
						t->Seek(t->GetPos()-1);
						done = 1;
						return 1;
					}
				}
			} else {
				if (c == '>')
					level--;

				if (level == 0) {
					__int64 currentPos = t->GetPos();
					do
					{
						c = t->ReadChar<char>(TFRC_NOLINEBREAKS | TFRC_DONTUPDATEPOS);
					} while ((!c || c==0x0A || c==0x0D) && (!t->IsEndOfStream()));
					t->Seek(currentPos);
					if (c != '<')
						done = 1;	
				}
			}
		}
	}

	t->Seek(pos);

	return 0;
}

int Textfile2String(CTextFile* t, char* c)
{
	c[0]=0;
	int j=0;
	int i;
	do { 
		i=t->ReadLine(c+j); 
		j+=i; } 
	while (i!=-1);

	return j;
}

char* Textfile2String(CTextFile* t)
{
	/* size of output string cannot be easily determined because of possible
	   conversions between UTF-16, UTF-8 and ANSI */

	int totalSize = 0;
	int sizeOfLine = 0;
	__int64 position = t->GetPos();
	char* pTemp;

	do 
	{
		sizeOfLine = t->ReadLine(&pTemp);
		if (sizeOfLine > 0)
			totalSize += sizeOfLine;
		free(pTemp);
	} while (sizeOfLine > -1);
	
	pTemp = (char*)malloc(totalSize+1);

	t->Seek(position);
	Textfile2String(t, pTemp);

	return pTemp;
}