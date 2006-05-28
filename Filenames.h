#ifndef I_FILENAMES
#define I_FILENAMES

#include "utf-8.h"
#include "strings.h"
#include <deque>
#include <vector>

/* expects utf-8 */

#define FILENAME_FROM_SEGMENT_TITLE 0x01
#define FILENAME_FROM_SOURCE_FILENAME 0x02
#define FILENAME_NOTHING 0x03

int Filename2LongFilename(char* in, char* out, int out_buf_len);
void FilenameKeepName(char* in_out);
void FilenameRemoveIllegalCharactersUTF8(char* in_out, char replacement);


#endif