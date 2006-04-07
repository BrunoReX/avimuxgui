#ifndef  I_VERSION
#define  I_VERSION

void ComposeVersionString(char* buffer);
char* GetAMGVersionString(char* buffer, int buf_len);
char* GetAMGVersionDate();
char* GetAMGVersionTime();

#endif