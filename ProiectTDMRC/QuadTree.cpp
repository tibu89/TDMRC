#include "QuadTree.h"
#include "ArithmeticEncoder.h"
#include "QuickSort.h"

#include <queue>

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

QuadTree::QuadTree(uQuadInt size) : numNodes(1)
{
	rootNode = new Node(0, size, size, 0);

	for(unsigned int i = 1; i < 0x10; i++)
	{
		distribution[i] = 0;
	}

	distribution[0] = 1;
}

QuadTree::QuadTree() : numNodes(0), rootNode(nullptr)
{
	for(unsigned int i = 0; i < 0x10; i++)
    {
        distribution[i] = 0;
    }
}

bool QuadTree::IsLeaf(Node* node)
{
	return (node->right - node->left == 1);
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
            assert(distribution[currentNode->mask] > 0);
			distribution[currentNode->mask]--;
			currentNode->mask |= quad;
			distribution[currentNode->mask]++;
            distribution[0]++;

            numNodes++;
		}

		currentNode = *nextNode;
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
    return numNodes;
}

size_t QuadTree::Encode(unsigned char *particlePtr, unsigned int numParticles, std::ostream &outStream)
{
    QuickSort<unsigned int>::quicksort((unsigned int*)particlePtr, numParticles);

    ReadParticles(particlePtr, numParticles);
    return WriteToStream(outStream);
}

size_t QuadTree::Encode(unsigned char *particlePtr, unsigned int numParticles, void **out)
{
    QuickSort<unsigned int>::quicksort((unsigned int*)particlePtr, numParticles);

    ReadParticles(particlePtr, numParticles);
    return WriteToBuffer(out);
}

void QuadTree::ReadParticles(unsigned char *p, unsigned int numParticles)
{
    unsigned short numRepeats = 0;
    uQuadInt previousX, previousY;

    previousX = (p[0] << 8) + p[1];
    previousY = (p[2] << 8) + p[3];

    p+=4;

    for(unsigned int i = 1; i < numParticles; i++, p += 4)
    {
        uQuadInt x = (p[0] << 8) + p[1];
        uQuadInt y = (p[2] << 8) + p[3];

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
}

void QuadTree::Serialize(std::stringbuf &buffer)
{
    std::vector<Node*> breadthFirstNodes;

    InfoHeader header;

    breadthFirstNodes.push_back(rootNode);
    unsigned int numNodes = 0;

    for(unsigned int i = 0; i < breadthFirstNodes.size(); i++)
    {
        Node* currentNode = breadthFirstNodes[i];
        if(currentNode->mask == 0)
        {
            numNodes = i;
            break;
        }

        if(currentNode->lowerLeft != nullptr)
        {
            breadthFirstNodes.push_back(currentNode->lowerLeft);
        }
        if(currentNode->lowerRight != nullptr)
        {
            breadthFirstNodes.push_back(currentNode->lowerRight);
        }
        if(currentNode->upperLeft != nullptr)
        {
            breadthFirstNodes.push_back(currentNode->upperLeft);
        }
        if(currentNode->upperRight != nullptr)
        {
            breadthFirstNodes.push_back(currentNode->upperRight);
        }
    }

    header.numNodes = numNodes;
    header.numRepeats = repeatingParticles.size();
    header.size = rootNode->right;

    buffer.sputn((char*)(&header), sizeof(InfoHeader));

    for(std::list<particle>::iterator it = repeatingParticles.begin(); it != repeatingParticles.end(); it++)
    {
        buffer.sputn((char*)&(*it), sizeof(particle));
    }

    unsigned int numPairs = numNodes / 2;

    std::vector<unsigned int> byteDistribution(0x100, 0);

    for(unsigned int i = 0; i < numPairs; i++)
    {
        unsigned char byte = 0;
        assert(breadthFirstNodes[2 * i]->mask != 0);
        byte |= breadthFirstNodes[2 * i]->mask;
        byte |= (breadthFirstNodes[2 * i + 1]->mask)<<4;

        buffer.sputc(byte);
        byteDistribution[byte]++;
    }

    //add last element for uneven number of nodes
    if(numNodes % 2 != 0)
    {
        buffer.sputc(breadthFirstNodes[2 * numPairs]->mask);
        byteDistribution[breadthFirstNodes[2 * numPairs]->mask]++;
    }

    std::string outString = buffer.str();
    std::cout<<"size without arithmetic encoding: "<<outString.size()<<std::endl;
}

void QuadTree::Deserialize(std::stringbuf &inBuffer, std::stringbuf &outBuffer)
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

	nodeQueue.push(new Node(0, header.size, header.size, 0));
	unsigned int i = 0;
	bool stopAddingNodes = false;

	while(!nodeQueue.empty())
	{
		Node* currentNode = nodeQueue.front();
		nodeQueue.pop();

		if(stopAddingNodes || (stopAddingNodes = (i >= header.numNodes)))
		{
			assert(currentNode->left == currentNode->midX);

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
}

size_t QuadTree::WriteToStream(std::ostream &outStream)
{
    std::stringbuf buffer;

    Serialize(buffer);

    std::string outString = buffer.str();
    outStream.write(outString.c_str(), outString.size());

    return outString.size();
}

size_t QuadTree::WriteToBuffer(void **out)
{
    std::stringbuf buffer;

    Serialize(buffer);

    std::string outString = buffer.str();
    size_t outSize = outString.size();

    *out = new unsigned char[outSize];

    memcpy(*out, outString.c_str(), outSize);

    return outSize;
}

size_t QuadTree::ReadFromBuffer(void *in, size_t inSize, void **out)
{
    std::stringbuf inBuffer(std::string((char*)in, inSize));
	std::stringbuf outBuffer;

    Deserialize(inBuffer, outBuffer);

	size_t outSize = outBuffer.str().size();

	*out = new unsigned char[outSize];
	memcpy(*out, outBuffer.str().c_str(), outSize);

	return outSize;
}