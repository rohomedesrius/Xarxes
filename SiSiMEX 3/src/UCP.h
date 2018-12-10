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

	void itemRequest();
	void resolveConstraint();
	void sendConstraint();
	void finishNegotiation();
	
	// TODO
private:
	uint16_t requestedItemId;
	uint16_t contributedItemId;

	AgentLocation ucpLocation;

	unsigned int searchDepth;

	MCPPtr mcp; //MCP pointer
};

