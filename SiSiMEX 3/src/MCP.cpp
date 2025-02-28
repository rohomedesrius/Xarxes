#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,
	ST_WAIT_ACCEPTANCE,
	ST_NEGOCIATING_UCP_RES,
	ST_NEGOTIATION_FINISHED
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth, unsigned int money) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID),
	_searchDepth(searchDepth),
	_money(money)
{
	setState(ST_INIT);
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
		queryMCCsForItem(_requestedItemId);
		setState(ST_REQUESTING_MCCs);
		break;

	case ST_ITERATING_OVER_MCCs:
		StartNegotioationRequest();
		break;

	case ST_REQUESTING_MCCs:
		break;

	case ST_WAIT_ACCEPTANCE:
		break;

	case ST_NEGOCIATING_UCP_RES:

		if (_ucp.get() != nullptr)
		{
			if (_ucp.get()->state() == 4)
			{
				if (_ucp.get()->NegotiationAccepted())
				{
					// Negotiation Accepted -------
					negotiationSucces = true;
					setState(ST_NEGOTIATION_FINISHED);
				}

				else
				{
					if (_mccRegisterIndex < _mccRegisters.size())
					{
						_ucp->stop();
						_mccRegisterIndex++;
						setState(ST_ITERATING_OVER_MCCs);
					}
					else
					{
						// Negotiation Canceled -------
						negotiationSucces = false;
						setState(ST_NEGOTIATION_FINISHED);
					}
				}
				
			}
		}
		break;

	case ST_NEGOTIATION_FINISHED:
		break;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)
	StopUCP();
	destroy();
}

void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Read(stream);

			// Log the returned MCCs
			for (auto &mccdata : packetData.mccAddresses)
			{
				uint16_t agentId = mccdata.agentId;
				const std::string &hostIp = mccdata.hostIP;
				uint16_t hostPort = mccdata.hostPort;
				//iLog << " - MCC: " << agentId << " - host: " << hostIp << ":" << hostPort;
			}

			// Store the returned MCCs from YP
			_mccRegisters.swap(packetData.mccAddresses);

			// Select the first MCC to negociate
			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::ReturnMCCsForItem was unexpected.";
		}
		break;

	case PacketType::NegotiationRequestResult:
	
		if (state() == ST_WAIT_ACCEPTANCE)
		{
			MCPacketNegociationRequest packetData;
			packetData.Read(stream);

			if (packetData.availableNegotiation == true)
			{
				CreateUCP(packetData.location);
				setState(ST_NEGOCIATING_UCP_RES);

				iLog << " - MPC: (" << id() << ") negotiating with MCC: (" << packetHeader.srcAgentId << ")";
			}

			else
			{
				_mccRegisterIndex++;
				setState(ST_ITERATING_OVER_MCCs);
			}
		}
		break;
	

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::StartNegotioationRequest()
{
	if (_mccRegisters.size() == 0 || _mccRegisterIndex > _mccRegisters.size() - 1)
	{
		setState(ST_NEGOTIATION_FINISHED);
		return false;
	}

	OutputMemoryStream ostream;

	PacketHeader p;
	p.packetType = PacketType::NegotiationRequest;
	p.srcAgentId = id();
	p.dstAgentId = _mccRegisters[_mccRegisterIndex].agentId;

	p.Write(ostream);

	bool ret = sendPacketToAgent(_mccRegisters[_mccRegisterIndex].hostIP, _mccRegisters[_mccRegisterIndex].hostPort, ostream);
	setState(ST_WAIT_ACCEPTANCE);
	return ret;
}


bool MCP::negotiationFinished() const
{
	return state() == ST_NEGOTIATION_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	return negotiationSucces; // TODO: Did the child UCP find a solution?
}

void MCP::CreateUCP(const AgentLocation loc)
{
	Node* n = new Node(App->agentContainer->allAgents().size());
	_ucp = App->agentContainer->createUCP(n, requestedItemId(), contributedItemId(), 
		loc, searchDepth(), _money);
}

void MCP::StopUCP()
{
	if (_ucp != nullptr) _ucp->stop();
}

bool MCP::queryMCCsForItem(int itemId)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}
