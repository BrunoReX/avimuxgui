#include "stdafx.h"
#include "Compression.h"

TRACK_COMPRESSION_DESCRIPTOR::TRACK_COMPRESSION_DESCRIPTOR()
{
	compression = COMPRESSION_NONE;
	compressed_elements = 0;
	compression_private = NULL;
	compression_private_size = NULL;
	is_decompressed = false;
	order = 0;
}

TRACK_COMPRESSION_DESCRIPTOR::~TRACK_COMPRESSION_DESCRIPTOR()
{
	if (compression_private_size && compression_private)
		free(compression_private);
}

TRACK_COMPRESSION_DESCRIPTOR& TRACK_COMPRESSION_DESCRIPTOR::operator =(const TRACK_COMPRESSION_DESCRIPTOR &other)
{
	compression = other.compression;
	order = other.order;
	compressed_elements = other.compressed_elements;

	if (compression_private && compression_private_size)
		free(compression_private);

	compression_private_size = other.compression_private_size;
	if (compression_private_size) {
		compression_private = malloc(compression_private_size);
		memcpy(compression_private, other.compression_private,
			compression_private_size);
	}

	return *this;
}

bool TRACK_COMPRESSION_DESCRIPTOR::operator ==(const TRACK_COMPRESSION_DESCRIPTOR &other)
{
	return (
		compression == other.compression &&
		order == other.order &&
		compressed_elements == other.compressed_elements &&
		compression_private_size == other.compression_private_size &&
		((!compression_private && !other.compression_private) ||
		(compression_private && other.compression_private &&
		!memcmp(compression_private, other.compression_private, compression_private_size))));
}