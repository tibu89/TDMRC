#include "StudentCodec.h"
#include "stdafx.h"
#include "QuadTree.h"
#include "ArithmeticEncoder.h"

#include <fstream>


size_t StudentCodec::compress(void *in, size_t in_size, void **out)
{
	//TO DO: your compression code
	unsigned char *p = static_cast<unsigned char*>(in);

	unsigned int numParticles = in_size / 4;

	QuadTree *quadTree = new QuadTree(32 * 256);

	std::stringbuf strBufQuadEnc;

    size_t outSize = quadTree->Encode(p, numParticles, strBufQuadEnc);
    std::cout<<"size after quadtree encode: "<<outSize<<" | compression ratio: "<<in_size / (float)outSize<<std::endl;

	delete quadTree;

	*out = malloc(outSize);
	memcpy(*out, strBufQuadEnc.str().c_str(), outSize);

	ArithmeticEncoder<unsigned long long> ariEncoder;

	std::stringbuf strBufAriEnc;

	ariEncoder.SetFrequencyFromBuffer(strBufQuadEnc);

	ariEncoder.Encode(strBufAriEnc, strBufQuadEnc, outSize);
	outSize = strBufAriEnc.str().size();
	std::cout<<"size after arithmetic encode: "<<outSize<<" | compression ratio: "<<in_size / (float)outSize<<std::endl;

	return outSize;
}

size_t StudentCodec::decompress(void *in, size_t in_size, void **out)
{
	//TO DO: your decompression code
	size_t outSize = QuadTree::ReadFromBuffer(in, in_size, out);

	return outSize;
}
