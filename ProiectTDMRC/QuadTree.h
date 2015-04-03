#pragma once

#include <assert.h>
#include <iostream>
#include <sstream>

#include <list>

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
	uQuadInt left, right, up, down;
	uQuadInt midX, midY;
    bitmask4 mask;

	Node *upperRight, *upperLeft, *lowerLeft, *lowerRight;

	Node(uQuadInt l = 0, uQuadInt r = 1, uQuadInt u = 1, uQuadInt d = 0) : left(l), right(r), up(u), down(d)
	{
		assert(left < right);
		assert(down < up);

		midX = (right + left) / 2;
		midY = (down  + up)   / 2;

		upperRight = upperLeft = lowerLeft = lowerRight = nullptr;

        mask = 0;
	}

	~Node()
	{
		if(upperRight != nullptr) { delete upperRight; upperRight = nullptr; }
		if(upperLeft  != nullptr) { delete  upperLeft;  upperLeft = nullptr; }
		if(lowerRight != nullptr) { delete lowerRight; lowerRight = nullptr; }
        if(lowerLeft  != nullptr) { delete  lowerLeft;  lowerLeft = nullptr; }
	}

public:
    void SetParams(uQuadInt l, uQuadInt r, uQuadInt u, uQuadInt d)
    {
        left = l; right = r; up = u; down = d;
    }
};

struct InfoHeader
{
    unsigned int numNodes;
    unsigned int numRepeats;
    uQuadInt size;
};

class QuadTree
{
private:
	Node *rootNode;
	unsigned int numNodes;

	unsigned int distribution[0x10];

    std::list<particle> repeatingParticles;

	bitmask4 GetQuadrant(Node* node, uQuadInt x, uQuadInt y);

    void Serialize(std::stringbuf &buffer);

    static void Deserialize(std::stringbuf &inBuffer, std::stringbuf &outBuffer);
    static Node* CreateChild(Node* parentNode, bitmask4 quadrant);
    static bool IsLeaf(Node* node);
    static void WriteParticle(uQuadInt x, uQuadInt y, std::stringbuf &buffer);

    void AddParticle(uQuadInt x, uQuadInt y);
    void ReadParticles(unsigned char *p, unsigned int numParticles);
    size_t WriteToStream(std::ostream &outStream);
    size_t WriteToBuffer(void **out);
	
public:
    QuadTree();
	QuadTree(uQuadInt size);
	~QuadTree() {if(rootNode != nullptr) delete rootNode;}

    unsigned int GetNumNodes();

    size_t Encode(unsigned char *particlePtr, unsigned int numParticles, std::ostream &outStream);
    size_t Encode(unsigned char *particlePtr, unsigned int numParticles, void **out);

    static size_t ReadFromBuffer(void *in, size_t inSize, void **out);    
};