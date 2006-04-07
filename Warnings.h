#ifndef I_WARNINGS
#define I_WARNINGS

void Note (char* reason, __int64 pos = 0);
void Warning (char* reason, __int64 pos = 0);
void Obfuscated (char* reason, __int64 pos = 0);
void B0rked (char* reason,  __int64 pos = 0);
void FatalError (char* reason, __int64 pos = 0);

void EnableLongMessages(int bEnabled = 1);

#endif