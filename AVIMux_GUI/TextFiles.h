/*

CTEXTFILE provides easy access to text files, which can be
using either the current windows code page, or utf-8.
This class will perform the necessary conversions to output
text in the desired format.

*/

#ifndef I_TEXTFILE
#define I_TEXTFILE

#include "../basestreams.h"

const int CM_ANSI  = 1;
const int CM_UTF8  = 2;
const int CM_UTF16 = 3; // not yet implemented!

const int TFRC_ALL			= 0x01;
const int TFRC_NOLINEBREAKS = 0x02;
const int TFRC_DONTUPDATEPOS= 0x04;

class CTEXTFILE: public STREAM
{
	private:
		int		iCharCoding;
		int		iOutputCoding;
		int		iHdrSize;
		STREAM*	source;
	protected:
		bool	virtual IsUTF8In();
	public:
		CTEXTFILE::CTEXTFILE();
		CTEXTFILE::CTEXTFILE(DWORD _dwMode, STREAM* s, int iOutputFormat = CM_ANSI);
		bool	virtual IsUTF8Out();
		int		virtual Open(DWORD _dwMode, STREAM* s);
		int		virtual ReOpen();
		int		virtual Read(void* d, int iBytes);
		char	virtual ReadChar(int flags);
		int		virtual ReadLine(char* s);
		void	virtual SelectInputFormat(int iFormat);
		void	virtual SelectOutputFormat(int iFormat);
		int		virtual Seek(__int64 qwPos);
		bool	virtual IsEndOfStream();
		__int64 virtual GetPos();
		__int64 virtual GetSize();

};


#endif