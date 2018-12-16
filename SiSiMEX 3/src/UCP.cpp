#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

// TODO: Make an enum with the states
enum UCP_states
{
	ST_INIT, 
	ST_REQUEST_ITEM,
	ST_RESOLVING_CONSTRAINT,
	ST_SENDING_CONSTRAINT, 
	ST_NEGOTIATION_FINISHED
};

UCP::UCP(Node *node, uint16_t _requestedItemId, uint16_t _contributedItemId, const AgentLocation &ucpsLoc, unsigned int _searchDepth) : Agent(node)
{
	requestedItemId = _requestedItemId;
	contributedItemId = _contributedItemId;
	ucpLocation = ucpsLoc;
	searchDepth = _searchDepth;

	setState(ST_INIT);
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
	case ST_INIT: 
		break;

	case ST_REQUEST_ITEM: 
		break;
	
	case ST_RESOLVING_CONSTRAINT: 
		break;
	
	case ST_SENDING_CONSTRAINT: 
		break;
	
	case ST_NEGOTIATION_FINISHED: 
		break;
	}
}

void UCP::stop()
{
	if (mcp != nullptr)
		mcp->stop();

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ConstraintRequest:
		if (state() == ST_REQUEST_ITEM)
		{
			PacketHeader packet;
			packet.packetType = packetType;
			PacketItemRequest item_req;
			PacketConstraintRequest constrain_req;
			
			item_req.Read(stream);
			UCPacketAcceptNegotiation negPacket;
			OutputMemoryStream outStream;

			unsigned int _depth = getDepth();

			if (item_req.itemId == getContItemId())
			{
				iLog << " - Accept Negotation: ";
				negotiationSuccess = true;
				setState(ST_NEGOTIATION_FINISHED);
						
				packet.packetType = PacketType::ConstraintResult;
				packet.srcAgentId = id();
				packet.dstAgentId = packetHeader.srcAgentId;

				negPacket.negotiationAccepted = negotiationSuccess;

				packet.Write(outStream);
				negPacket.Write(outStream);

				socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
			}
			else
			{
				iLog << " - Search another Negotation: " << "Search Depth" << _depth;
				if (_depth == 2)
				{
					iLog << " - MAX SEARCH DEPTH: ";
					setState(ST_NEGOTIATION_FINISHED);
					PacketHeader outPacketHead;
					UCPacketAcceptNegotiation negPacket;
					OutputMemoryStream outStream;

					outPacketHead.packetType = PacketType::ConstraintResult;
					outPacketHead.srcAgentId = id();
					outPacketHead.dstAgentId = packetHeader.srcAgentId;

					negPacket.negotiationAccepted = negotiationSuccess;

					outPacketHead.Write(outStream);
					negPacket.Write(outStream);

					socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
				}
				else
				{
					setState(ST_RESOLVING_CONSTRAINT);
					_depth += 1;
					Node* newNode = new Node(App->agentContainer->allAgents().size());
					mcp = App->agentContainer->createMCP(newNode, item_req.itemId, getContItemId(), _depth);
				}
			}

			break;
		}
		else
		{
			wLog << "UCP 1 - OnPacketReceived() - PacketType::RequestConstraint was unexpected";
		}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

uint16_t UCP::getReqItemId()
{
	return requestedItemId;
}

uint16_t UCP::getContItemId()
{
	return contributedItemId;
}

AgentLocation UCP::getUcpLoc()
{
	return ucpLocation;
}

unsigned int UCP::getDepth()
{
	return searchDepth;
}