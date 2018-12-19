#include "UCC.h"


// TODO: Make an enum with the states
enum UCC_states
{
	ST_INIT,
	WAITING_REQUEST, 
	WAITING_CONSTRAINT,
	NEGOTIATION_FINISHED
};

UCC::UCC(Node *node, uint16_t _contributedItemId, uint16_t _constraintItemId) :
	Agent(node), contributedItemId(_constraintItemId), constraintItemId(_constraintItemId)
{
	setState(ST_INIT);
}

UCC::~UCC()
{
}

void UCC::update()
{
	if(state() == ST_INIT)
		setState(WAITING_REQUEST);
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ItemRequest:
		if (state() == WAITING_REQUEST)
		{
			PacketHeader packetHead;
			packetHead.packetType = PacketType::ConstraintRequest;
			packetHead.srcAgentId = id();
			packetHead.dstAgentId = packetHeader.srcAgentId;

			PacketConstraintRequest packetData;
			packetData.itemId = getConstraintId();

			OutputMemoryStream outstream;
			packetHead.Write(outstream);
			packetData.Write(outstream);

			setState(WAITING_CONSTRAINT);

			socket->SendPacket(outstream.GetBufferPtr(), outstream.GetSize());
		}
		else
		{
			wLog << "UCC 1 - OnPacketReceived() - PacketType::RequestItem was unexpected.";
		}
		break;

	case PacketType::ConstraintAcceptance:
		if (state() == WAITING_CONSTRAINT)
		{
			UCPacketAcceptNegotiation packetData;
			packetData.Read(stream);

			negotiationSuccesful = packetData.negotiationAccepted;
			setState(NEGOTIATION_FINISHED);
		}
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool UCC::SendToUCP(uint16_t id)
{
	return true;
}