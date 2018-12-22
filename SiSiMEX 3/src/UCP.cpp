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
	ST_IDLE, 
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
		sendPacketStartNegotiation();
		setState(ST_REQUEST_ITEM);
		break;

	case ST_REQUEST_ITEM: 
		break;
	
	case ST_RESOLVING_CONSTRAINT: 
		break;
	
	case ST_IDLE: 
		if (mcp.get() != nullptr)
		{
			if (mcp.get()->state() == ST_NEGOTIATION_FINISHED)
				negotiationSuccess = true;
			else
				negotiationSuccess = false;

			setState(ST_NEGOTIATION_FINISHED);
			SendPacketFinishNegotiation();

		}
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
		if (state() == ST_REQUEST_ITEM || state() == ST_IDLE)
		{

			PacketItemRequest item_req;	
			item_req.Read(stream);


			if (item_req.itemId == getContItemId())
			{
				iLog << " - Accept Negotation: ";
				negotiationSuccess = true;
				setState(ST_NEGOTIATION_FINISHED);

				PacketHeader outPacketHead;
				UCPacketAcceptNegotiation negPacket;
				OutputMemoryStream outStream;
						
				outPacketHead.packetType = PacketType::ConstraintAcceptance;
				outPacketHead.srcAgentId = id();
				outPacketHead.dstAgentId = packetHeader.srcAgentId;

				negPacket.negotiationAccepted = negotiationSuccess;

				outPacketHead.Write(outStream);
				negPacket.Write(outStream);

				socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
			}
			else
			{
				iLog << " - Search another Negotation: " << "Search Depth " << searchDepth;
				if (searchDepth == MAX_SEARCH_DEPTH)
				{
					iLog << " - MAX SEARCH DEPTH: ";
					setState(ST_NEGOTIATION_FINISHED);
					PacketHeader outPacketHead;
					UCPacketAcceptNegotiation negPacket;
					OutputMemoryStream outStream;

					outPacketHead.packetType = PacketType::ConstraintAcceptance;
					outPacketHead.srcAgentId = id();
					outPacketHead.dstAgentId = packetHeader.srcAgentId;

					negPacket.negotiationAccepted = negotiationSuccess;

					outPacketHead.Write(outStream);
					negPacket.Write(outStream);

					socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
				}
				else
				{
					setState(ST_IDLE);
					searchDepth ++;
					Node* newNode = new Node(App->agentContainer->allAgents().size());
					mcp = App->agentContainer->createMCP(newNode, item_req.itemId, getContItemId(), searchDepth);
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

bool UCP::sendPacketStartNegotiation()
{
	PacketHeader packet;
	packet.packetType = PacketType::ItemRequest;
	packet.srcAgentId = id();
	packet.dstAgentId = ucpLocation.agentId;

	OutputMemoryStream stream;
	packet.Write(stream);

	return sendPacketToAgent(ucpLocation.hostIP, ucpLocation.hostPort, stream);
}


bool UCP::SendPacketFinishNegotiation()
{
	// Create message header and data
	PacketHeader packet;
	packet.packetType = PacketType::ConstraintAcceptance;
	packet.srcAgentId = id();
	packet.dstAgentId = ucpLocation.agentId;

	UCPacketAcceptNegotiation packetAcceptance;
	packetAcceptance.negotiationAccepted = negotiationSuccess;

	// Serialize message
	OutputMemoryStream stream;
	packet.Write(stream);
	packet.Write(stream);

	return sendPacketToAgent(ucpLocation.hostIP, ucpLocation.hostPort, stream);
}
