#include "stdafx.h"
#include "Warnings.h"
#include "stdio.h"
#include "formatint64.h"

FILE* warn_output = stderr;
int bWarningsEnabled = 1;
int bLongMessages = 1;

void EnableWarnings(int bEnabled = 1) 
{
	bWarningsEnabled = bEnabled;
}

void EnableLongMessages(int bEnabled)
{
	bLongMessages = bEnabled;
}

void MKVParser_DebugMessage(char* message, char* cat, __int64 pos = 0)
{
	if (bWarningsEnabled) {
		char	buffer[1000];
		buffer[0]=0;
		if (!pos) {
			if (bLongMessages) {
				sprintf(buffer,"(mkv parser) %s: \n  %s\n\n",cat,message);
			} else {
				sprintf(buffer,"%s: %s\n",cat,message);
			}
		} else {
			char cPos[30]; cPos[0]=0; QW2Str(pos, cPos, 1);
			if (bLongMessages) {
				sprintf(buffer,"(mkv parser) %s: \n  pos. %s: %s\n\n", cat, cPos, message);
			} else {
				sprintf(buffer," %s: %sB: %s\n\n", cat, cPos, message);
			}
		}
		fprintf(warn_output,buffer);
	}
}

// output note to stderr
void Note (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"note",pos);
}

// output warning about b0rked file to stderr
void B0rked (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"file is b0rked", pos);
}

// output warning about obfuscated file to stderr
void Obfuscated (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"file is obfuscated", pos);
}

// output warning
void Warning (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"warning",pos);
}

// Fatal
void FatalError (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"fatal",pos);
}
