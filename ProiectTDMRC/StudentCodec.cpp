#include "StudentCodec.h"
#include "stdafx.h"
#include "QuadTree.h"
#include "ArithmeticEncoder.h"

#include <fstream>


size_t StudentCodec::compress(void *in, size_t in_size, void **out)
{
	unsigned char *p = static_cast<unsigned char*>(in);

	unsigned int numParticles = in_size / 4;

	QuadTree *quadTree = new QuadTree();

	std::stringbuf strBufQuadEnc;

    size_t outSize = quadTree->Encode(p, numParticles, strBufQuadEnc);
    std::cout<<"size after quadtree encode: "<<outSize<<" | compression ratio: "<<in_size / (float)outSize<<std::endl;

	delete quadTree;

	ArithmeticEncoder<unsigned long long> ariEncoder;

	std::stringbuf strBufAriEnc;

	ariEncoder.Encode(strBufAriEnc, strBufQuadEnc, outSize);
	outSize = strBufAriEnc.str().size();
    std::cout<<"size after arithmetic encode: "<<outSize<<" | compression ratio: "<<in_size / (float)outSize<<std::endl;

    *out = malloc(outSize);
    memcpy(*out, strBufAriEnc.str().c_str(), outSize);

	return outSize;
}

size_t StudentCodec::decompress(void *in, size_t in_size, void **out)
{
    std::stringbuf strBuf1(std::string((char*)in, in_size));
    std::stringbuf strBuf2;

    ArithmeticEncoder<unsigned long long> ariEncoder;
    ariEncoder.Decode(strBuf1, strBuf2);

    strBuf1.str("");

    std::cout<<"size after arithmetic decode: "<<strBuf2.str().size()<<std::endl;

    size_t outSize = QuadTree::Decode(strBuf2, strBuf1);

    *out = new unsigned char[outSize];
    memcpy(*out, strBuf1.str().c_str(), outSize);

	return outSize;
}
