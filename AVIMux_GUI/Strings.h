#ifndef I_STRINGS
#define I_STRINGS

char* ucase(char* s,char* d) ;
char* getword(char** s);
bool isint(char* s);
bool isposint(char* s);

void splitpathname(char* p, char** f, char** e, char** cPath = NULL);


#endif