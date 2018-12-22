#pragma once

#include "Module.h"
#include "net/Net.h"
#include "Node.h"
#include "MCC.h"
#include "MCP.h"

class ModuleNodeCluster : public Module, public TCPNetworkManagerDelegate
{
public:

	// Virtual methods from parent class Module

	bool init() override;

	bool start() override;

	bool update() override;

	bool updateGUI() override;

	bool cleanUp() override;

	bool stop() override;


	// TCPNetworkManagerDelegate virtual methods

	void OnAccepted(TCPSocketPtr socket) override;

	void OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream) override;

	void OnDisconnected(TCPSocketPtr socket) override;

private:

	bool startSystem();

	void runSystem();

	void stopSystem();


	void spawnMCP(int nodeId, int requestedItemId, int contributedItemId);

	void spawnMCC(int nodeId, int contributedItemId, int constraintItemId);

	void CreatePayOption(MCP* mcp);

	void SetPaymentData(MCP* mcp_payer, bool payment_option);

	std::vector<NodePtr> _nodes; /**< Array of nodes spawn in this host. */

	int state = 0; /**< State machine. */

	bool payment_option = false;
	MCP* mcp_payer = nullptr;
};
