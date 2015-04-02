#pragma once

#include <assert.h>
#include <iostream>
#include <sstream>

#define UPPER_RIGHT 1
#define UPPER_LEFT  2
#define LOWER_LEFT  4
#define LOWER_RIGHT 8

typedef unsigned char bitmask4;
typedef unsigned short uQuadInt;

struct Node
{
	uQuadInt left, right, up, down;
	uQuadInt midX, midY;
    union coercion { bitmask4 mask; bitmask4 numOccurences; };

    coercion data;

	Node *upperRight, *upperLeft, *lowerLeft, *lowerRight;

	Node(uQuadInt l = 0, uQuadInt r = 1, uQuadInt u = 1, uQuadInt d = 0) : left(l), right(r), up(u), down(d)
	{
		assert(left < right);
		assert(down < up);

		midX = (right + left) / 2;
		midY = (down  + up)   / 2;

		upperRight = upperLeft = lowerLeft = lowerRight = nullptr;

        data.mask = 0;
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
    uQuadInt size;
};

class QuadTree
{
private:
	Node *rootNode;
	unsigned int numNodes;

	unsigned int distribution[0x10];

	bitmask4 GetQuadrant(Node* node, uQuadInt x, uQuadInt y);

    void Serialize(std::stringbuf &buffer);

    static void Deserialize(std::stringbuf &inBuffer, std::stringbuf &outBuffer);
    static Node* CreateChild(Node* parentNode, bitmask4 quadrant);
    static bool IsLeaf(Node* node);
	
public:
    QuadTree();
	QuadTree(uQuadInt size);
	~QuadTree() {if(rootNode != nullptr) delete rootNode;}

	void AddParticle(uQuadInt x, uQuadInt y);
    unsigned int GetNumNodes();

    void WriteToStream(std::ostream &outStream);
    size_t WriteToBuffer(void **out);

    static size_t ReadFromBuffer(void *in, size_t inSize, void **out);
};