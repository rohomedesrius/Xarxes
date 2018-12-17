#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

class AgentLocation;

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t _requestedItemId, uint16_t _contributedItemId, const AgentLocation &ucpLoc, unsigned int _searchDepth);
	~UCP();

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	uint16_t getReqItemId();
	uint16_t getContItemId();
	AgentLocation getUcpLoc();
	unsigned int getDepth();

	bool SendPacketFinishNegotiation();
	bool NegotiationAccepted() const { return negotiationSuccess; }

	
private:
	uint16_t requestedItemId;
	uint16_t contributedItemId;

	AgentLocation ucpLocation;

	unsigned int searchDepth;

	bool negotiationSuccess = false;

	MCPPtr mcp; //MCP pointer
};

