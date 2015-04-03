#include "StudentCodec.h"
#include "stdafx.h"
#include "QuadTree.h"

#include <fstream>


size_t StudentCodec::compress(void *in, size_t in_size, void **out)
{
	//TO DO: your compression code
	unsigned char *p = static_cast<unsigned char*>(in);

	unsigned int numParticles = in_size / 4;

	QuadTree *quadTree = new QuadTree(32 * 256);

    size_t outSize = quadTree->Encode(p, numParticles, out);
    std::cout<<"size after quadtree encode: "<<outSize<<std::endl;

	delete quadTree;

	return outSize;
}

size_t StudentCodec::decompress(void *in, size_t in_size, void **out)
{
	//TO DO: your decompression code
	*out = new unsigned char[in_size];
	memcpy(*out, in, in_size);

	size_t outSize = QuadTree::ReadFromBuffer(in, in_size, out);

	return outSize;
}
