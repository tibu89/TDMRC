#pragma once

#include <assert.h>
#include <iostream>
#include <sstream>

#include <list>
#include <vector>

#define UPPER_RIGHT 1
#define UPPER_LEFT  2
#define LOWER_LEFT  4
#define LOWER_RIGHT 8

typedef unsigned char bitmask4;
typedef unsigned short uQuadInt;

struct particle
{
    uQuadInt x;
    uQuadInt y;

    particle(uQuadInt _x, uQuadInt _y) : x(_x), y(_y){}

    bool operator== (particle &p) {return x == p.x && y == p.y;}
};

struct Node
{
	uQuadInt left, down;
	uQuadInt size;
    bitmask4 mask;

	int upperRightID, upperLeftID, lowerLeftID, lowerRightID;

	Node(uQuadInt l = 0, uQuadInt d = 0, uQuadInt s = 1) : left(l), down(d), size(s)
	{
		assert(size >= 1);

		upperRightID = upperLeftID = lowerLeftID = lowerRightID = -1;

        mask = 0;
	}

public:
    void SetParams(uQuadInt l, uQuadInt d, uQuadInt s)
    {
        left = l; 
        down = d;
        size = s;
    }

	int GetChildIDFromQuad(bitmask4 quad)
	{
		switch(quad)
		{
		case UPPER_RIGHT:
			return upperRightID;
		case LOWER_RIGHT:
			return lowerRightID;
		case UPPER_LEFT:
			return upperLeftID;
		case LOWER_LEFT:
			return lowerLeftID;
		default:
			assert(false); //GetChildIDFromQuad called with invalid quad
		}

		return -1;
	}

	void SetChildIDFromQuad(bitmask4 quad, int id)
	{
		switch(quad)
		{
		case UPPER_RIGHT:
			upperRightID = id;
			break;
		case LOWER_RIGHT:
			lowerRightID = id;
			break;
		case UPPER_LEFT:
			upperLeftID = id;
			break;
		case LOWER_LEFT:
			lowerLeftID = id;
			break;
		default:
			assert(false); //SetChildIDFromQuad called with invalid quad
		}
	}
};

struct InfoHeader
{
    unsigned int numNodes;
    unsigned int numRepeats;
    int offsetX, offsetY;

    uQuadInt size;
};

class QuadTree
{
private:
    int rootNodeID;
    int offX, offY;

	unsigned int distribution[0x10];

    std::vector<particle> repeatingParticles;
	std::vector<Node> nodePool;

	bitmask4 GetQuadrant(Node &node, uQuadInt x, uQuadInt y);

    static Node* CreateChild(Node *parentNode, bitmask4 quadrant);
	static int   CreateChild(Node &parentNode, bitmask4 quadrant, std::vector<Node> &nodeVector);
    static bool IsLeaf(Node &node);
    static void WriteParticle(uQuadInt x, uQuadInt y, std::stringbuf &buffer);

    void SetRootNode(uQuadInt x, uQuadInt y);
    void CheckDimensions(uQuadInt x, uQuadInt y);    
    size_t WriteToBuffer(std::stringbuf &buf);

    void AddParticle(uQuadInt x, uQuadInt y);
	
public:
	QuadTree();

    unsigned int GetNumNodes();

    int ReadParticlesWithinLimits(std::vector<particle> &particlesVector, unsigned int startIndex, int lowX, int highX, int lowY, int highY);
    void SetOffsets(int _offX, int _offY){ offX = _offX; offY = _offY; }
    void Reset();

    void Serialize(std::stringbuf &buffer);

    static size_t Decode(std::stringbuf &inBuffer, std::stringbuf &outBuffer);
};