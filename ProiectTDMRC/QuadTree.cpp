#include "QuadTree.h"

QuadTree::QuadTree(uQuadInt width, uQuadInt height)
{
	rootNode = new Node(0, width, height, 0);
	numNodes = 1;
}