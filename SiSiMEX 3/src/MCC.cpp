#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	ST_NEGOCIATING,
	ST_UNREGISTERING,
	ST_FINISHED
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
	case ST_INIT:
		if (registerIntoYellowPages()) 
			setState(ST_REGISTERING);
		else 
			setState(ST_FINISHED);
		break;

	case ST_REGISTERING:
		// See OnPacketReceived()
		break;
		
	case ST_IDLE:
		break;

	case ST_NEGOCIATING:
		if (_ucc.get() != nullptr)
		{
			if (_ucc.get()->state() == State::ST_FINISHED)
			{
				if (_ucc.get()->GetNegotiationResult())
				{
					//negotiation succesful
					setState(ST_FINISHED);
				}

				else
				{
					//negotiation unsuccesful
					//_ucc->stop();
					setState(ST_IDLE);
				}

				node()->itemList().SetItemUsed(_contributedItemId, false);
			}
		}
		break;

	case ST_UNREGISTERING:
		break;
			
	case ST_FINISHED:
		destroy();
		break;
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	DestroyUCC();

	unregisterFromYellowPages();
	setState(ST_FINISHED);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	PacketHeader outPacketHead;
	MCPacketNegociationRequest packetData;
	OutputMemoryStream outStream;


	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::NegotiationRequest:

		outPacketHead.packetType = PacketType::NegotiationAcceptance;
		outPacketHead.srcAgentId = id();
		outPacketHead.dstAgentId = packetHeader.srcAgentId;

		if (state() != ST_IDLE)
		{
			//negociation is available
			packetData.availableNegotiation = true;
		}

		else
		{
			//TODO
			if (node()->itemList().isItemUsed(_contributedItemId))
			{
				packetData.availableNegotiation = false;

				outPacketHead.Write(outStream);
				packetData.Write(outStream);
				socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());

				break;
			}
			else
			{
				node()->itemList().SetItemUsed(_contributedItemId, true);
			}

			packetData.availableNegotiation = true;

			CreateUCC();

			//Set AgentLocation
			packetData.location.hostIP = socket->RemoteAddress().GetIPString();
			packetData.location.hostPort = LISTEN_PORT_AGENTS;
			packetData.location.agentId = _ucc->id();

			setState(ST_NEGOCIATING);
		}

		outPacketHead.Write(outStream);
		packetData.Write(outStream);
		socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());

		break;


	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return negotiationFinished();
}

bool MCC::registerIntoYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::unregisterFromYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	sendPacketToYellowPages(stream);
}

void MCC::CreateUCC()
{
	// TODO: Create a unicast contributor
	Node* newNode = new Node(App->agentContainer->allAgents().size());
	_ucc = App->agentContainer->createUCC(node(), contributedItemId(), constraintItemId());
}

void MCC::DestroyUCC()
{
	// TODO: Destroy the unicast contributor child
	if (_ucc != nullptr)
		_ucc->stop();
}
