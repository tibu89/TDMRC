#include "QuadTree.h"

bitmask4 QuadTree::GetQuadrant(Node* node, uQuadInt x, uQuadInt y)
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

Node* QuadTree::CreateChild(Node* parent, bitmask4 quadrant)
{
	switch(quadrant)
	{
	case LOWER_LEFT:
		return new Node(parent->left, parent->midX, parent->midY, parent->down);
		break;
	case LOWER_RIGHT:
		return new Node(parent->midX, parent->right, parent->midY, parent->down);
		break;
	case UPPER_RIGHT:
		return new Node(parent->midX, parent->right, parent->up, parent->midY);
		break;
	case UPPER_LEFT:
		return new Node(parent->left, parent->midX, parent->up, parent->midY);
		break;
	default:
		assert(false);
		break;
	}

	return nullptr;
}

QuadTree::QuadTree(uQuadInt size)
{
	rootNode = new Node(0, size, size, 0);
	numNodes = 1;

	for(unsigned int i = 1; i < 0x10; i++)
	{
		distribution[i] = 0;
	}

	distribution[0] = 1;
}

void QuadTree::AddParticle(uQuadInt x, uQuadInt y)
{
	Node* currentNode = rootNode;

	while(	currentNode->left + 1 < currentNode->right &&
			currentNode->down + 1 < currentNode->up)
	{
		bitmask4 quad = GetQuadrant(currentNode, x, y);
		Node** nextNode;

		switch(quad)
		{
		case UPPER_RIGHT:
			nextNode = &currentNode->upperRight;
			break;
		case LOWER_RIGHT:
			nextNode = &currentNode->lowerRight;
			break;
		case UPPER_LEFT:
			nextNode = &currentNode->upperLeft;
			break;
		case LOWER_LEFT:
			nextNode = &currentNode->lowerLeft;
			break;
		}

		if(*nextNode == nullptr)
		{
			*nextNode = CreateChild(currentNode, quad);

			assert(distribution[currentNode->mask] >= 0);
			distribution[currentNode->mask]--;
			currentNode->mask |= quad;
			distribution[currentNode->mask]++;
		}

		currentNode = *nextNode;
	}
}
