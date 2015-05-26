#include "QuadTree.h"
#include "ArithmeticEncoder.h"
#include "QuickSort.h"

#include <queue>

bitmask4 QuadTree::GetQuadrant(Node &node, uQuadInt x, uQuadInt y)
{
    uQuadInt halfSize = node.size / 2;
    x -= node.left;
    y -= node.down;

    assert(x < node.size);
    assert(y < node.size);

	if(x < halfSize)
	{
		if(y < halfSize)
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
		if(y < halfSize)
		{
			return LOWER_RIGHT;
		}
		else
		{
			return UPPER_RIGHT;
		}
	}
}

int QuadTree::CreateChild(Node &parent, bitmask4 quadrant, std::vector<Node> &nodeVector)
{
    uQuadInt halfSize = parent.size / 2;

	switch(quadrant)
	{
	case LOWER_LEFT:
		nodeVector.push_back(Node(parent.left, parent.down, halfSize));
		break;
	case LOWER_RIGHT:
		nodeVector.push_back(Node(parent.left + halfSize, parent.down, halfSize));
		break;
	case UPPER_RIGHT:
		nodeVector.push_back(Node(parent.left + halfSize, parent.down + halfSize, halfSize));
		break;
	case UPPER_LEFT:
        nodeVector.push_back(Node(parent.left, parent.down + halfSize, halfSize));
		break;
	default:
		assert(false); //CreateChild called with invalid quadrant
		return -1;
		break;
	}

	return nodeVector.size() - 1;
}

Node* QuadTree::CreateChild(Node* parent, bitmask4 quadrant)
{
    uQuadInt halfSize = parent->size / 2;

	switch(quadrant)
	{
	case LOWER_LEFT:
		return new Node(parent->left, parent->down, halfSize);
		break;
	case LOWER_RIGHT:
        return new Node(parent->left + halfSize, parent->down, halfSize);
        break;
	case UPPER_RIGHT:
        return new Node(parent->left + halfSize, parent->down + halfSize, halfSize);
        break;
	case UPPER_LEFT:
        return new Node(parent->left, parent->down + halfSize, halfSize);
        break;
	default:
		assert(false); //CreateChild called with invalid quadrant
		break;
	}

	return nullptr;
}


QuadTree::QuadTree() : rootNodeID(-1), offX(0), offY(0)
{
    for(unsigned int i = 0; i < 0x10; i++)
    {
        distribution[i] = 0;
    }
}

void QuadTree::Reset()
{
    rootNodeID = -1;

    for(unsigned int i = 0; i < 0x10; i++)
    {
        distribution[i] = 0;
    }

    repeatingParticles.clear();
    nodePool.clear();

    offX = offY = 0;
}

bool QuadTree::IsLeaf(Node &node)
{
	return (node.size == 1);
}

void QuadTree::SetRootNode(uQuadInt x, uQuadInt y)
{
    assert(nodePool.size() == 0);

    uQuadInt size = 1;

    while(size <= x || size <= y)
    {
        size *= 2;
    }

    nodePool.push_back(Node(0, 0, size));
    distribution[0] = 1;
    rootNodeID = 0;
}

void QuadTree::CheckDimensions(uQuadInt x, uQuadInt y)
{
    Node *rootNodePtr = &nodePool[rootNodeID];

    while(rootNodePtr->size <= x || rootNodePtr->size <= y)
    {
        assert(rootNodePtr->left == 0);
		assert(rootNodePtr->down == 0);

        Node newRoot(0, 0, rootNodePtr->size * 2);
        newRoot.lowerLeftID = rootNodeID;
        newRoot.mask = LOWER_LEFT;
        distribution[LOWER_LEFT]++;

        nodePool.push_back(newRoot);
        rootNodeID = nodePool.size() - 1;

        rootNodePtr = &nodePool.back();
    }
}

void QuadTree::AddParticle(uQuadInt x, uQuadInt y)
{
    x -= offX;
    y -= offY;

    if(nodePool.size() == 0)
    {
        SetRootNode(x, y);
    }

    CheckDimensions(x, y);

	int currentNodeID = rootNodeID;

	while(	nodePool[currentNodeID].size > 1)
	{
		bitmask4 quad = GetQuadrant(nodePool[currentNodeID], x, y);
		int nextNodeID = nodePool[currentNodeID].GetChildIDFromQuad(quad);

		if(nextNodeID == -1)
		{
			nextNodeID = CreateChild(nodePool[currentNodeID], quad, nodePool);
			nodePool[currentNodeID].SetChildIDFromQuad(quad, nextNodeID);
            assert(distribution[nodePool[currentNodeID].mask] > 0);
			distribution[nodePool[currentNodeID].mask]--;
			nodePool[currentNodeID].mask |= quad;
			distribution[nodePool[currentNodeID].mask]++;
            distribution[0]++;
		}

		currentNodeID = nextNodeID;
    }
}

void QuadTree::WriteParticle(uQuadInt x, uQuadInt y, std::stringbuf &outBuffer)
{
    char buffer[4];

    buffer[0] = (char)(x >> 8);
    buffer[1] = (char)x;
    buffer[2] = (char)(y >> 8);
    buffer[3] = (char)y;

    outBuffer.sputn(buffer, sizeof(buffer));
}

unsigned int QuadTree::GetNumNodes()
{
    return nodePool.size();
}

bool particleWithinLimits(uQuadInt x, uQuadInt y, int lowX, int highX, int lowY, int highY)
{
    return x < highX && y < highY && x >= lowX && y >= lowY;
}

int QuadTree::ReadParticlesWithinLimits(std::vector<particle> &particlesVector, unsigned int startIndex, int lowX, int highX, int lowY, int highY)
{
    unsigned short numRepeats = 0;
    unsigned int i;
    uQuadInt previousX, previousY;

    particle currentParticle = particlesVector[startIndex];
    unsigned char* p = (unsigned char*)&currentParticle;

    previousX = (p[0] << 8) + p[1];
    previousY = (p[2] << 8) + p[3];

    for(i = startIndex + 1; i < particlesVector.size(); i++)
    {
        currentParticle = particlesVector[i];
        p = (unsigned char*)&currentParticle;

		uQuadInt x = (p[0] << 8) + p[1];
        uQuadInt y = (p[2] << 8) + p[3];

        if(!particleWithinLimits(x, y, lowX, highX, lowY, highY))
        {
            break;
        }

        if(previousX == x && previousY == y)
        {
            numRepeats++;
            repeatingParticles.push_back(particle(x,y));
        }
        else
        {
            if(numRepeats == 0)
            {
                AddParticle(previousX, previousY);
            }

            numRepeats = 0;
        }

        previousX = x;
        previousY = y;
    }

    if(numRepeats == 0)
    {
        AddParticle(previousX, previousY);
    }

    return i;
}

void QuadTree::Serialize(std::stringbuf &buffer)
{
    if(rootNodeID == -1) return;

    std::vector<Node*> breadthFirstNodes;

    InfoHeader header;

	breadthFirstNodes.push_back(&nodePool[rootNodeID]);
    unsigned int numNodes = 0;

    for(unsigned int i = 0; i < breadthFirstNodes.size(); i++)
    {
        Node* currentNode = breadthFirstNodes[i];
        if(currentNode->mask == 0)
        {
            numNodes = i;
            break;
        }

        if(currentNode->lowerLeftID != -1)
        {
            breadthFirstNodes.push_back(&nodePool[currentNode->lowerLeftID]);
        }
        if(currentNode->lowerRightID != -1)
        {
            breadthFirstNodes.push_back(&nodePool[currentNode->lowerRightID]);
        }
        if(currentNode->upperLeftID != -1)
        {
            breadthFirstNodes.push_back(&nodePool[currentNode->upperLeftID]);
        }
        if(currentNode->upperRightID != -1)
        {
            breadthFirstNodes.push_back(&nodePool[currentNode->upperRightID]);
        }
    }

    header.numNodes = numNodes;
    header.numRepeats = repeatingParticles.size();
    header.size = nodePool[rootNodeID].size;

//    header.offsetX = offX;
//    header.offsetY = offY;

    buffer.sputn((char*)(&header), sizeof(InfoHeader));

    for(std::vector<particle>::iterator it = repeatingParticles.begin(); it != repeatingParticles.end(); it++)
    {
        buffer.sputn((char*)&(*it), sizeof(particle));
    }

    unsigned int numPairs = numNodes / 2;

    for(unsigned int i = 0; i < numPairs; i++)
    {
        unsigned char byte = 0;
        assert(breadthFirstNodes[2 * i]->mask != 0);
        byte |= breadthFirstNodes[2 * i]->mask;
        byte |= (breadthFirstNodes[2 * i + 1]->mask)<<4;

        buffer.sputc(byte);
    }

    //add last element for uneven number of nodes
    if(numNodes % 2 != 0)
    {
        buffer.sputc(breadthFirstNodes[2 * numPairs]->mask);
    }
}

size_t QuadTree::Decode(std::stringbuf &inBuffer, std::stringbuf &outBuffer)
{
    InfoHeader header;

    inBuffer.sgetn((char*)(&header), sizeof(InfoHeader));

    if(header.numRepeats > 0)
    {
        particle prevParticle(0,0);
        particle currentParticle(0,0);

        inBuffer.sgetn((char*)(&prevParticle), sizeof(particle));

        WriteParticle(prevParticle.x, prevParticle.y, outBuffer);

        for(unsigned int i = 1; i < header.numRepeats; i++)
        {
            inBuffer.sgetn((char*)(&currentParticle), sizeof(particle));

            WriteParticle(prevParticle.x, prevParticle.y, outBuffer);
            if(!(currentParticle == prevParticle))
            {
                WriteParticle(currentParticle.x, currentParticle.y, outBuffer);
            }

            prevParticle = currentParticle;
        }

        WriteParticle(prevParticle.x, prevParticle.y, outBuffer);
    }

	std::vector<bitmask4> bitmaskVector;

	unsigned int numPairs = header.numNodes / 2;

	for(unsigned int i = 0; i < numPairs; i++)
	{
		unsigned char currentByte = inBuffer.sbumpc();
		bitmaskVector.push_back(currentByte & 0xF);
		bitmaskVector.push_back(currentByte >> 4);
    }

    if(header.numNodes % 2 != 0)
    {
        unsigned char lastByte = inBuffer.sbumpc();
        bitmaskVector.push_back(lastByte);

        assert(lastByte < 0x10);
    }

	std::queue<Node*> nodeQueue;

	nodeQueue.push(new Node(0, 0, header.size));
	unsigned int i = 0;
	bool stopAddingNodes = false;

	while(!nodeQueue.empty())
	{
		Node* currentNode = nodeQueue.front();
		nodeQueue.pop();

		if(stopAddingNodes || (stopAddingNodes = (i >= header.numNodes)))
		{
			assert(currentNode->size == 1);

            WriteParticle(currentNode->left, currentNode->down, outBuffer);
		}
        else
        {
            currentNode->mask = bitmaskVector[i++];

		    if(currentNode->mask & LOWER_LEFT)
		    {
			    nodeQueue.push(CreateChild(currentNode, LOWER_LEFT));
		    }
		    if(currentNode->mask & LOWER_RIGHT)
		    {
			    nodeQueue.push(CreateChild(currentNode, LOWER_RIGHT));
		    }
		    if(currentNode->mask & UPPER_LEFT)
		    {
			    nodeQueue.push(CreateChild(currentNode, UPPER_LEFT));
		    }
		    if(currentNode->mask & UPPER_RIGHT)
		    {
			    nodeQueue.push(CreateChild(currentNode, UPPER_RIGHT));
		    }
        }

        delete currentNode;
	}

    return outBuffer.str().size();
}

size_t QuadTree::WriteToBuffer(std::stringbuf &buffer)
{
    Serialize(buffer);

    std::string outString = buffer.str();
    size_t outSize = outString.size();

    return outSize;
}