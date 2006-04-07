#include "stdafx.h"
#include "xmlfiles.h"

int FileIsXML(CTEXTFILE* t)
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
					c = t->ReadChar(TFRC_NOLINEBREAKS | TFRC_DONTUPDATEPOS);
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
					c = t->ReadChar(TFRC_NOLINEBREAKS | TFRC_DONTUPDATEPOS);
					if (c != '<')
						done = 1;	
				}
			}
		}
	}

	t->Seek(pos);

	return 0;
}

int Textfile2String(CTEXTFILE* t, char* c)
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

