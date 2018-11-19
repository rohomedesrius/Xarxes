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

UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &ucpsLoc, unsigned int searchDepth) : Agent(node)
{
	_requestedItemId = requestedItemId;
	_contributedItemId = contributedItemId;
	_ucpLocation = ucpsLoc;
	_searchDepth = searchDepth;

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
		itemRequest();
		break;
	
	case ST_RESOLVING_CONSTRAINT: 
		resolveConstraint();
		break;
	
	case ST_SENDING_CONSTRAINT: 
		sendConstraint();
		break;
	
	case ST_NEGOTIATION_FINISHED: 
		finishNegotiation();
		break;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::itemRequest()
{

}

void UCP::resolveConstraint()
{

}

void UCP::sendConstraint()
{

}

void UCP::finishNegotiation()
{

}