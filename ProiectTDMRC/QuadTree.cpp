#include "QuadTree.h"

bitmask4 GetQuadrant(Node* node, uQuadInt x, uQuadInt y)
{
	if(x < node->midX)
	{
		if(y < node->midY)
		{
			return LOWER_LEFT;
		}
		else
		{
			return UPPER_LEFT;
		}
	}
	else
	{
		if(y < node->midY)
		{
			return LOWER_RIGHT;
		}
		else
		{
			return UPPER_RIGHT;
		}
	}
}

QuadTree::QuadTree(uQuadInt width, uQuadInt height)
{
	rootNode = new Node(0, width, height, 0);
	numNodes = 1;
}

void QuadTree::AddParticle(uQuadInt x, uQuadInt y)
{
	Node* currentNode = rootNode;

	while(	currentNode->left + 1 < currentNode->right &&
			currentNode->down + 1 < currentNode->up)
	{
		bitmask4 quad = GetQuadrant(currentNode, x, y);
		Node* nextNode;

		switch(quad)
		{
		case UPPER_RIGHT:
			nextNode = currentNode->upperRight;
			break;
		case LOWER_RIGHT:
			nextNode = currentNode->lowerRight;
			break;
		case UPPER_LEFT:
			nextNode = currentNode->upperLeft;
			break;
		case LOWER_LEFT:
			nextNode = currentNode->lowerLeft;
			break;
		}

		if(nextNode == nullptr)
		{
			nextNode = new Node(currentNode);
			currentNode->mask &= quad;
		}

		currentNode = nextNode;
	}
}
