#ifndef I_XMLFILES
#define I_XMLFILES

#include "..\xml.h"
#include "textfiles.h"

int FileIsXML(CTextFile* t);
int Textfile2String(CTextFile* t, char* c);
char* Textfile2String(CTextFile* t);

#endif