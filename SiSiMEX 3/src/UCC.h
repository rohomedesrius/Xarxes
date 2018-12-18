#pragma once
#include "Agent.h"

class UCC :
	public Agent
{
public:

	// Constructor and destructor
	UCC(Node *node, uint16_t _contributedItemId, uint16_t _constraintItemId);
	~UCC();

	// Agent methods
	void update() override;
	void stop() override;
	UCC* asUCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	// TODO
	bool SendToUCP(uint16_t id);

	uint16_t getContributedId() const { return contributedItemId; }
	uint16_t getConstraintId() const { return constraintItemId; }

	bool GetNegotiationResult() { return negotiationSuccesful; }

private:
	uint16_t contributedItemId;
	uint16_t constraintItemId;

	bool negotiationSuccesful = false;
};