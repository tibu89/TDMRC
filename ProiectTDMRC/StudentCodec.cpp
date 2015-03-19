#include "StudentCodec.h"
#include "stdafx.h"
#include "QuadTree.h"


size_t StudentCodec::compress(void *in, size_t in_size, void **out)
{
	//TO DO: your compression code
	*out = new unsigned char[in_size];
	memcpy(*out, in, in_size);

	unsigned char *p = static_cast<unsigned char*>(in);

	unsigned int numParticles = in_size / 4;

	QuadTree *quadTree = new QuadTree(32 * 256);

	for(unsigned int i = 0; i < numParticles; i++, p += 4)
	{
		uQuadInt x = p[0];
		x <<= 8;
		x |= p[1];

		uQuadInt y = p[2];
		y <<= 8;
		y |= p[3];

		quadTree->AddParticle(x, y);
	}

	return in_size;
}

size_t StudentCodec::decompress(void *in, size_t in_size, void **out)
{
	//TO DO: your decompression code
	*out = new unsigned char[in_size];
	memcpy(*out, in, in_size);

	return in_size;
}
