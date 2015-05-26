#include "StudentCodec.h"
#include "stdafx.h"
#include "QuadTree.h"
#include "ArithmeticEncoder.h"

#include <fstream>

static const unsigned int maxQuadSize = 64 * 256;

bool particleCompare(particle &a, particle &b)
{
    unsigned short aQuadY = a.y / maxQuadSize;
    unsigned short bQuadY = b.y / maxQuadSize;

    if(aQuadY < bQuadY)
    {
        return true;
    }
    if(aQuadY > bQuadY)
    {
        return false;
    }

    unsigned short aQuadX = a.x / maxQuadSize;
    unsigned short bQuadX = b.x / maxQuadSize;

    if(aQuadX < bQuadX)
    {
        return true;
    }
    if(aQuadX > bQuadX)
    {
        return false;
    }

    if(a.y == b.y)
    {
        return a.x < b.x;
    }

    return a.y < b.y;
}

bool particleWithinLimits(particle &p, int &lowX, int &highX, int &lowY, int &highY)
{
    return p.x < highX && p.y < highY && p.x >= lowX && p.y >= lowY;
}

int compressWithQuadTrees(std::vector<particle> &particlesVector, std::stringbuf &outBuf)
{
    int currentQuadX = 0, currentQuadY = 0;
    int lowX = 0, lowY = 0, highX = maxQuadSize, highY = maxQuadSize;
    QuadTree quadTree;

    for(std::vector<particle>::iterator it = particlesVector.begin(); it != particlesVector.end(); it++)
    {
        particle currentParticle = *it;

        if(!particleWithinLimits(currentParticle, lowX, highX, lowY, highY))
        {
            quadTree.Serialize(outBuf);
            quadTree.Reset();

            currentQuadX = currentParticle.x / maxQuadSize;
            currentQuadY = currentParticle.y / maxQuadSize;

            lowX = currentQuadX * maxQuadSize;
            lowY = currentQuadY * maxQuadSize;

            highX = lowX + maxQuadSize;
            highY = lowY + maxQuadSize;
        }

        quadTree.AddParticle(currentParticle.x, currentParticle.y);
    }

    quadTree.Serialize(outBuf);

    return outBuf.str().size();
}

size_t StudentCodec::compress(void *in, size_t in_size, void **out)
{
	unsigned char *p = static_cast<unsigned char*>(in);

	unsigned int numParticles = in_size / 4;
    std::vector<particle> particlesVector((particle*)in, (particle*)in + in_size / sizeof(particle));
    delete[] in;

	QuadTree *quadTree = new QuadTree();

	std::stringbuf strBufQuadEnc;

    size_t outSize = compressWithQuadTrees(particlesVector, strBufQuadEnc);
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
