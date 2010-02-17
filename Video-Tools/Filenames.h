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

/* converts a filename that might contain ..\..\.. and stuff to
   a filename in UNC form, so that CreateFileW allows opening
   it even if the filename is long 
*/
//int Filename2LongFilename(char* in, char* out, int out_buf_len);

/* input: a filename with extension and folder name
   output: the filename without extension and without the folder name
*/
//void FilenameKeepName(char* in_out);

/* replaces characters that cannot occur in a filename with
   the replacement character
*/
//void FilenameRemoveIllegalCharactersUTF8(char* in_out, char replacement);


#endif