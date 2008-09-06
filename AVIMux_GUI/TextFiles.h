/*

CTextFile provides easy access to text files, which can be using UTF-8,
UTF-16-LE, UTF-16-BE, or the current windows code page. This class will
perform the necessary conversions to output text in the desired format.

Files not using the current windows code page must have a BOM, otherwise
the file encoding is not detected automatically.

*/

#ifndef I_TEXTFILE
#define I_TEXTFILE

#include "../basestreams.h"
#include "../utf-8.h"

const int TFRC_ALL			= 0x01;
const int TFRC_NOLINEBREAKS = 0x02;
const int TFRC_DONTUPDATEPOS= 0x04;

/** \brief Allows reading a text file 
 *
 * This class reads a file from a CStream and includes conversion from
 * and to UTF-8 and UTF-16 as well as readling entire lines.
 */
class CTextFile: public STREAM
{
	private:
		int		iCharCoding;
		int		iOutputCoding;
		int		iHdrSize;
		STREAM*	source;
	protected:



		/** \brief Read one input character
		 * 
		 * This method reads one character from the input stream. 
		 * \remarks In the case
		 * of UTF-8, the number of bytes that needs to be read is 
		 * typically 1..4. 
		 */
		int     virtual ReadInputChar(unsigned char* pDest, size_t max_len);
	public:
		/* constructors */
		CTextFile();
		CTextFile(DWORD _dwMode, STREAM* s, int iOutputFormat = CHARACTER_ENCODING_ANSI);
		virtual ~CTextFile();
		/* encoding -> bool */
		bool	virtual IsUTF8In();
		bool	virtual IsUTF8Out();
		bool	virtual IsUTF16In();
		bool	virtual IsUTF16Out();
		bool	virtual IsAnsiIn();
		bool	virtual IsAnsiOut();

		/* read bytes */
		int		virtual Read(void* d, int iBytes);

		/* Those two use the one above */
		int		virtual ReadLine(char* d);
		int		virtual ReadLine(char* d, int max_len);

		/** \brief Returns the input file encoding
		 *
		 * The difference between UTF-16 LE and UTF-16 BE is also returned.
		 */
		int		GetFileInputEncoding();

		/** \brief Returns the input encoding
		 *
		 * This method does, unlike GetFileInputEncoding(), return the encoding
		 * the characters have just before converting them to the appropriate output
		 * format.
		 * \remarks When UTF-16 BE is detected, it is first converted to LE
		 * in the reading function. Thus, the actual conversion function gets
		 * the UTF-16 LE encoded characters. Consequently, this method will report
		 * UTF-16 LE encoding for both UTF-16 BE and UTF-16 LE.
		 */
		int		GetInputEncoding();

		/** \brief Returns the output encoding
		 *
         * \remarks UTF-16 BE not being supported for output, the return
		 * value cannot be CHARACTER_ENCODING_UTF16_BE 
		 */
		int		GetOutputEncoding();

		/** \brief Opens a stream 
		 *
		 * This method initializes the CTextFile object for reading from a
		 * stream
		 * \param dwMode STREAM_READ for reading
		 * \param s The CStream to read from
		 */
		int		virtual Open(DWORD  dwMode, STREAM* s);

		/** \brief Reopen a file
		 */
		int		virtual ReOpen();

		/** \brief Read one character from the stream
		 *
		 * This method reads one character from the stream and returns it
		 * as character (char) or wide character (wchar_t).
		 * \param T Can be \a char oder \a wchar_t .
		 * \param flags So far, this parameter has no meaning.
         */
		template<class T>
		T ReadChar(int flags);

		/** \brief Read one line from the source stream 
		 *
		 * This method reads one line from the source stream. It allocates
		 * the required memory.
		 * \param d A pointer to pointer that will point to the allocated buffer
		 * \returns The number of bytes that were stored
		 * \remarks It is assumed that no line is longer than 4095 bytes.
		 */
		int		virtual ReadLine(char** d);

		/** \brief Override an automatically detected file input encoding 
		 *
		 * \param iEncoding The encoding to assume.
		 * \remarks Possible encoding values are
		 * - CHARACTER_ENCODING_ANSI
		 * - CHARACTER_ENCODING_UTF8
		 * - CHARACTER_ENCODING_UTF16_LE
		 * - CHARACTER_ENCODING_UTF16_BE
		 */
		void	virtual SetFileInputEncoding(int iEncoding);

		/** \brief Choose the encoding to return text from the file 
		 *
		 * \param iEncoding The encoding to return the output in.
		 * \remarks Possible encoding values are
		 * - CHARACTER_ENCODING_ANSI
		 * - CHARACTER_ENCODING_UTF8
		 * - CHARACTER_ENCODING_UTF16_LE
		 */
		void	virtual SetOutputEncoding(int iEncoding);
		
		/** \brief Seek inside the file
		 *
		 * This method seeks to a position relative to the first byte
		 * after a possible BOM.
		 * \param qwPos Position to seek to.
		 * \returns 
		 * - success: STREAM_SEEK_OK
		 * - failure: STREAM_SEEK_ERROR
		 */
		int		virtual Seek(__int64 qwPos);

		/** \brief Check if the file's end was already reached.
		 *
		 * This method returns true if the last character in the file
		 * has already been read.
		 */
		bool	virtual IsEndOfStream();

		/** \brief Retrieve the current position in the file
		 *
		 * This method retrieves the current position in the file where
		 * the first byte that is not part of the BOM is byte 0.
		 * \returns The current position in the file.
		 */
		__int64 virtual GetPos();

		/** \brief Determines the file size.
		 *
		 * Returns the size of the text file used as source, not including
		 * a BOM that might be present at the beginning of the file.
		 * \returns The file size
		 */
		__int64 virtual GetSize();

};


#endif