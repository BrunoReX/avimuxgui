#include "stdafx.h"
#include "Warnings.h"
#include "stdio.h"
#include "formatint64.h"
#include <sstream>
#include <string>

FILE* warn_output = stderr;
int bWarningsEnabled = 1;
int bLongMessages = 1;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	std::ostringstream sstrResult;

	if (bWarningsEnabled) {
		char	buffer[1000];
		buffer[0]=0;
		printf("\r                                                                               \r");
		if (!pos) {
			if (bLongMessages) {
				sstrResult << "(mkv parser) " << cat << ": \x0D\x0A  " << message << "\x0D\x0A\x0D\x0A";
				//sprintf(buffer,"(mkv parser) %s: \n  %s\n\n",cat,message);
			} else {
				sstrResult << cat << ": " << message << "\x0D\x0A";
				//sprintf(buffer,"%s: %s\n",cat,message);
			}
		} else {
			char cPos[30]; cPos[0]=0; QW2Str(pos, cPos, 1);
			if (bLongMessages) {
				sstrResult << "(mkv parser) " << cat << ": \x0D\x0A  pos. " << cPos << ": " << message << "\x0D\x0A\x0D\x0A"; 
				//sprintf(buffer,"(mkv parser) %s: \n  pos. %s: %s\n\n", cat, cPos, message);
			} else {
				sstrResult << " " << cat << ": " << cPos << "B: " << message << "\x0D\x0A\x0D\x0A";
				//sprintf(buffer," %s: %sB: %s\n\n", cat, cPos, message);
			}
		}

		fprintf(warn_output,sstrResult.str().c_str());
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
void Weird (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"file is weird", pos);
}

// output warning
void Warning (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"warning",pos);
}

// Fatal
void FatalError (char* reason, __int64 pos) {
	MKVParser_DebugMessage(reason,"fatal",pos);
}
