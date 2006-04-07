#ifndef I_STRINGS
#define I_STRINGS

#include <vector>

char* ucase(char* s,char* d) ;
char* getword(char** s);
bool isint(char* s);
bool isposint(char* s);

void splitpathname(char* p, char** f, char** e, char** cPath = NULL);
int split_string(char* in, char* separator, std::vector<char*>& dest);
void DeleteStringVector(std::vector<char*>& v);


#endif