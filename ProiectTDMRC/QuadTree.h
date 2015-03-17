#pragma once

#include <assert.h>

#define UPPER_RIGHT (1<<0)
#define UPPER_LEFT  (1<<1)
#define LOWER_LEFT  (1<<2)
#define LOWER_RIGHT (1<<3)

typedef unsigned char bitmask4;
typedef unsigned short uQuadInt;

class Node
{
	uQuadInt left, right, up, down;
	uQuadInt midX, midY;
	bitmask4 mask;

	Node *upperRight, *upperLeft, *lowerLeft, *lowerRight;

public:
	Node(uQuadInt l, uQuadInt r, uQuadInt u, uQuadInt d) : left(l), right(r), up(u), down(d), mask(0)
	{
		assert(left < right);
		assert(down < up);

		midX = (right + left) / 2;
		midY = (down + up) / 2;
	}
};

class QuadTree
{
private:
	Node *rootNode;
	unsigned int numNodes;

	void AddParticle(uQuadInt x, uQuadInt y);
	
public:
	QuadTree(uQuadInt width, uQuadInt height);
};