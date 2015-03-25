#include "QuadTree.h"

#include <vector>
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
	numNodes++;
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
	numNodes--; //false alarm;
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
            assert(distribution[currentNode->data.mask] > 0);
			distribution[currentNode->data.mask]--;
			currentNode->data.mask |= quad;
			distribution[currentNode->data.mask]++;
            distribution[0]++;
		}

		currentNode = *nextNode;
	}

    //currently the code doesn't handle more than 15 identical particles, will extend in future (maybe)
    assert(currentNode->data.numOccurences < 0x0F);
    distribution[currentNode->data.numOccurences]--;
    currentNode->data.numOccurences++;
    distribution[currentNode->data.numOccurences]++;
}

unsigned int QuadTree::GetNumNodes()
{
    return numNodes;
}

void QuadTree::Serialize(std::stringbuf &buffer)
{
    std::vector<Node*> breadthFirstNodes;

    InfoHeader header;

    header.numNodes = GetNumNodes();
    header.size = rootNode->right;

    buffer.sputn((char*)(&header), sizeof(InfoHeader));

    breadthFirstNodes.push_back(rootNode);

    for(unsigned int i = 0; i < breadthFirstNodes.size(); i++)
    {
        Node* currentNode = breadthFirstNodes[i];
        if(currentNode->data.mask == 0)
        {
            continue;
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

    std::cout<<breadthFirstNodes.size()<<std::endl;

    unsigned int numPairs = breadthFirstNodes.size() / 2;

    for(unsigned int i = 0; i < numPairs; i++)
    {
        unsigned char byte = 0;
        byte |= breadthFirstNodes[2 * i]->data.mask;
        byte |= (breadthFirstNodes[2 * i + 1]->data.mask)<<4;

        buffer.sputc(byte);
    }

    //add last element for uneven number of nodes
    if(breadthFirstNodes.size() % 2 != 0)
    {
        buffer.sputc(breadthFirstNodes[2 * numPairs]->data.mask);
    }

    std::string outString = buffer.str();
    std::cout<<outString.size()<<std::endl;
}

void QuadTree::Deserialize(std::stringbuf &inBuffer, std::stringbuf &outBuffer)
{
    InfoHeader header;

    inBuffer.sgetn((char*)(&header), sizeof(InfoHeader));

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

	//delete current data, if any
    if(rootNode != nullptr) delete rootNode;

	rootNode = new Node(0, header.size, header.size, 0);
	numNodes = 1;

	std::queue<Node*> nodeQueue;

	nodeQueue.push(rootNode);
	unsigned int i = 0;
	bool stopAddingNodes = false;

	char buffer[4];

	while(!nodeQueue.empty())
	{
		assert(i < header.numNodes);

		Node* currentNode = nodeQueue.front();
		nodeQueue.pop();

		currentNode->data.mask = bitmaskVector[i++];

		if(stopAddingNodes || (stopAddingNodes = IsLeaf(currentNode)))
		{
			assert(currentNode->left == currentNode->midX);

			buffer[0] = (char)(currentNode->left >> 8);
			buffer[1] = (char)currentNode->left;
			buffer[2] = (char)(currentNode->down >> 8);
			buffer[3] = (char)currentNode->down;

			for(unsigned int j = 0; j < currentNode->data.numOccurences; j++)
			{
				outBuffer.sputn(buffer, sizeof(buffer));
			}

			continue;
		}

		if(currentNode->data.mask & LOWER_LEFT)
		{
			currentNode->lowerLeft = CreateChild(currentNode, LOWER_LEFT);
			nodeQueue.push(currentNode->lowerLeft);
		}
		if(currentNode->data.mask & LOWER_RIGHT)
		{
			currentNode->lowerRight = CreateChild(currentNode, LOWER_RIGHT);
			nodeQueue.push(currentNode->lowerRight);
		}
		if(currentNode->data.mask & UPPER_LEFT)
		{
			currentNode->upperLeft = CreateChild(currentNode, UPPER_LEFT);
			nodeQueue.push(currentNode->upperLeft);
		}
		if(currentNode->data.mask & UPPER_RIGHT)
		{
			currentNode->upperRight = CreateChild(currentNode, UPPER_RIGHT);
			nodeQueue.push(currentNode->upperRight);
		}
	}
}

void QuadTree::WriteToStream(std::ostream &outStream)
{
    std::stringbuf buffer;

    Serialize(buffer);

    std::string outString = buffer.str();
    outStream.write(outString.c_str(), outString.size());
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

	std::cout<<"deserialized buffer size: "<<outSize<<std::endl;

	*out = new unsigned char[outSize];
	memcpy(*out, outBuffer.str().c_str(), outSize);

	return outSize;
}