#pragma once

#include <assert.h>
#include <iostream>
#include <sstream>

#define UPPER_RIGHT (1<<0)
#define UPPER_LEFT  (1<<1)
#define LOWER_LEFT  (1<<2)
#define LOWER_RIGHT (1<<3)

typedef unsigned char bitmask4;
typedef unsigned short uQuadInt;

struct Node
{
	uQuadInt left, right, up, down;
	uQuadInt midX, midY;
    union coercion { bitmask4 mask; bitmask4 numOccurences; };

    coercion data;

	Node *upperRight, *upperLeft, *lowerLeft, *lowerRight;

	Node(uQuadInt l, uQuadInt r, uQuadInt u, uQuadInt d) : left(l), right(r), up(u), down(d)
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
		if(upperRight != nullptr) delete upperRight;
		if(upperLeft  != nullptr) delete  upperLeft;
		if(lowerRight != nullptr) delete lowerRight;
		if(lowerLeft  != nullptr) delete  lowerLeft;
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
	Node* CreateChild(Node* parentNode, bitmask4 quadrant);

	bool IsLeaf(Node* node);

    void Serialize(std::stringbuf &buffer);
    void Deserialize(std::stringbuf &buffer);
	
public:
    QuadTree();
	QuadTree(uQuadInt size);
	~QuadTree() {if(rootNode != nullptr) delete rootNode;}

	void AddParticle(uQuadInt x, uQuadInt y);
    unsigned int GetNumNodes();

    void WriteToStream(std::ostream &outStream);
    size_t WriteToBuffer(void **out);

    void ReadFromBuffer(void *in, size_t inSize);
};