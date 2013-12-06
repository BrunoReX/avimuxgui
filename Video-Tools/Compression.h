#ifndef I_COMPRESSION
#define I_COMPRESSION

#include <vector>

const int COMPRESSION_NONE         = 0x00000000;
const int COMPRESSION_ZLIB         = 0x00000001;
const int COMPRESSION_HDRSTRIPPING = 0x00000004;
const int COMPRESSION_ERROR        =-0x00000037;
const int COMPRESSION_UNKNOWN		= -0x01;

class TRACK_COMPRESSION_DESCRIPTOR
{
public:
	/* simple construktor */
	TRACK_COMPRESSION_DESCRIPTOR();

	/* copy constructor */
	TRACK_COMPRESSION_DESCRIPTOR(const TRACK_COMPRESSION_DESCRIPTOR& other) {
		compression = other.compression;
		order = other.order;
		compressed_elements = other.compressed_elements;

		compression_private_size = other.compression_private_size;
		if (compression_private_size) {
			compression_private = malloc(compression_private_size);
			memcpy(compression_private, other.compression_private,
				compression_private_size);
		} else
			compression_private = NULL;
	}

	/* destructor */
	virtual ~TRACK_COMPRESSION_DESCRIPTOR();

	/* assignment */
	TRACK_COMPRESSION_DESCRIPTOR& operator=(const TRACK_COMPRESSION_DESCRIPTOR &other);

	/* check wether two compression descriptors are equal */
	bool operator==(const TRACK_COMPRESSION_DESCRIPTOR &other);

	/* compression algorithm */
	int			compression;

	/* order in which several compressions on the same track
	   is applied */
	int			order;

	/* settings for a certain compression algorithm; this is
	   specific for each algorithm */
	void*		compression_private;
	int			compression_private_size;

    /* defines whether or not this compression can be and is reversed
	   by the reader */
	bool		is_decompressed;

	/* defines which part of a track is compressed, i.e. headers,
	   frames */
	int			compressed_elements;
};
typedef std::vector<TRACK_COMPRESSION_DESCRIPTOR> TRACK_COMPRESSION;

#define CCompressionInfo TRACK_COMPRESSION_DESCRIPTOR

#endif