#include "ClientFactory.h"

int identifier = 0;

std::vector<SAMPClient*> clients;

float szBotPos[3];

void ConnectionRejected(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);

	SAMPClient* clientPtr = NULL;
	for each (auto & i in clients)
	{
		if (i->GetInterface() != NULL && i->GetInterface()->GetUniqueID() == rpcParams->recipient->uniqueID)
		{
			clientPtr = i;
			break;
		}
	}

	if (clientPtr == NULL) return Log("[Connection Reject] Failed to find Client Pointer for SA:MP Client! (MTU: %d)", rpcParams->recipient->GetMTUSize());

	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteRejectReason;

	bsData.Read(byteRejectReason);

	if (byteRejectReason == REJECT_REASON_BAD_VERSION)
	{
		clientPtr->iVersion = !clientPtr->iVersion;
		Log("[Client %d] Bad SA-MP version, switching to %s instead.", clientPtr->uniqueID, clientPtr->iVersion == VERSION_037 ? "0.3.7" : "0.3.DL");

		clientPtr->iConnected = false;
		clientPtr->iConnectionRequested = false;
		clientPtr->ConnectedSince = 0;
		clientPtr->ReqConnectTick = GetTickCount() + 1;
	}
	else if (byteRejectReason == REJECT_REASON_BAD_NICKNAME)
	{
		sprintf(clientPtr->szNickname, "%s_%d%c", clientPtr->szInitialName, clientPtr->uniqueID, 'a' + (clientPtr->uniqueID % 26));

		Log("[Client %d] Bad nickname. Changing name to %s.", clientPtr->uniqueID, clientPtr->szNickname);

		clientPtr->iConnected = false;
		clientPtr->iConnectionRequested = false;
		clientPtr->ConnectedSince = 0;
		clientPtr->ReqConnectTick = GetTickCount() + 1;
	}
	else if (byteRejectReason == REJECT_REASON_BAD_MOD)
	{
		Log("[Client %d] Connection Reject: Bad mod version. No further connections.", clientPtr->uniqueID);
	}
	else if (byteRejectReason == REJECT_REASON_BAD_PLAYERID)
	{
		Log("[Client %d] Connection Reject: Bad player ID. No further connections.", clientPtr->uniqueID);
	}
	else
		Log("[Client %d] ConnectionRejected: Unknown. No further connections", clientPtr->uniqueID);
}

void Packet_AUTH_KEY(Packet* p, RakClientInterface* pRakClient)
{
	char* auth_key;
	bool found_key = false;

	for (int x = 0; x < 512; x++)
	{
		if (!strcmp(((char*)p->data + 2), AuthKeyTable[x][0]))
		{
			auth_key = AuthKeyTable[x][1];
			found_key = true;
		}
	}

	if (found_key)
	{
		RakNet::BitStream bsKey;
		BYTE byteAuthKeyLen;

		byteAuthKeyLen = (BYTE)strlen(auth_key);

		bsKey.Write((BYTE)ID_AUTH_KEY);
		bsKey.Write((BYTE)byteAuthKeyLen);
		bsKey.Write(auth_key, byteAuthKeyLen);

		pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, NULL);

		//Log("[AUTH] %s -> %s", ((char*)p->data + 2), auth_key);
	}
	else
	{
		Log("[NoSlots] Unknown AUTH_IN! (%s)", ((char*)p->data + 2));
	}
}

SAMPClient::SAMPClient(char* hostname, int port, char* password, char* nick, int version)
{
    // On creation
    this->pRakClient = RakNetworkFactory::GetRakClientInterface();
    if (this->pRakClient == NULL)
        return;

	identifier++;
	this->uniqueID = identifier;
	this->pRakClient->SetUniqueID(this->uniqueID);
	this->iPort = port;
    strcpy(this->szAddr, hostname);
    strcpy(this->szPassword, password);
	strcpy(this->szInitialName, nick);
	strcpy(this->szNickname, nick);
	this->iVersion = version;

	this->pRakClient->RegisterClassMemberRPC(&RPC_ConnectionRejected, ConnectionRejected);
	this->pRakClient->SetMTUSize(576);
	this->iConnected = false;
	this->iConnectionRequested = false;
	this->ReqConnectTick = GetTickCount();
	this->ConnectedSince = 0;
	clients.push_back(this);
}

RakClientInterface* SAMPClient::GetInterface()
{
	return this->pRakClient;
}

void SAMPClient::Packet_ConnectionSucceeded(Packet* p)
{
	RakNet::BitStream bsSuccAuth((unsigned char*)p->data, p->length, false);
	unsigned short myPlayerID;
	unsigned int uiChallenge;

	bsSuccAuth.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
	bsSuccAuth.IgnoreBits(32); // binaryAddress
	bsSuccAuth.IgnoreBits(16); // port

	bsSuccAuth.Read(myPlayerID);

	bsSuccAuth.Read(uiChallenge);

	Log("[Client %d] Connected, joining the game.", this->uniqueID);

	int iVersion = (this->iVersion == VERSION_037 ? 4057 : 4062);
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;
	BYTE byteMod = 1;

	char auth_bs[4 * 16] = { 0 };
	gen_gpci(auth_bs, 0x3e9);

	BYTE byteAuthBSLen;
	byteAuthBSLen = (BYTE)strlen(auth_bs);
	BYTE byteNameLen = (BYTE)strlen(this->szNickname);

	char iClientVer[24];
	sprintf(iClientVer, "0.3.7-R2");
	if (this->iVersion == VERSION_03DL) sprintf(iClientVer, "0.3.DL");

	BYTE iClientVerLen = (BYTE)strlen(iClientVer);

	RakNet::BitStream bsSend;

	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(this->szNickname, byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write(auth_bs, byteAuthBSLen);
	bsSend.Write(iClientVerLen);
	bsSend.Write(iClientVer, iClientVerLen);

	pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	
	this->ConnectedSince = GetTickCount();
	this->iConnected = true;
	return;
}

void SAMPClient::UpdatePlayerScoresAndPings(int iWait, int iMS)
{
	static DWORD dwLastUpdateTick = 0;

	if (iWait)
	{
		if ((GetTickCount() - dwLastUpdateTick) > (DWORD)iMS)
		{
			dwLastUpdateTick = GetTickCount();
			RakNet::BitStream bsParams;
			pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
		}
	}
	else
	{
		RakNet::BitStream bsParams;
		pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void SAMPClient::ChangeName(char* name)
{
	strcpy(this->szInitialName, name);
	return;
}

void SAMPClient::Disconnect()
{
	this->pRakClient->Disconnect(500);
	this->iConnected = false;
	this->iConnectionRequested = false;
	this->ConnectedSince = 0;
	this->ReqConnectTick = GetTickCount() + 100;
	return;
}

int SAMPClient::UpdateNetwork()
{
	if (this->iConnectionRequested == false && this->iConnected == false && GetTickCount() >= this->ReqConnectTick)
	{
		Log("[Client %d] Connecting to the server...", this->uniqueID);

		pRakClient->SetPassword(szPassword);
		this->ConnectedSince = 0;
		this->iConnectionRequested = true;
		this->ReqConnectTick = GetTickCount() + 1;

		int conret = (int)pRakClient->Connect(szAddr, iPort, 0, 0, 5);
		
		return conret;
	}
	
	unsigned char packetIdentifier;
	Packet* pkt;

	while (pkt = pRakClient->Receive())
	{
		if ((unsigned char)pkt->data[0] == ID_TIMESTAMP)
		{
			if (pkt->length > sizeof(unsigned char) + sizeof(unsigned int))
				packetIdentifier = (unsigned char)pkt->data[sizeof(unsigned char) + sizeof(unsigned int)];
			else
				return 0;
		}
		else
			packetIdentifier = (unsigned char)pkt->data[0];

		//Log("[RAKSAMP] Packet received. PacketID: %d.", pkt->data[0]);

		switch (packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			Log("[Client %d] Connection was closed by the server. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 100;
			this->ConnectedSince = 0;
			
			break;
		case ID_CONNECTION_BANNED:
			Log("[Client %d] You are banned. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 2000;
			this->ConnectedSince = 0;
			
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			Log("[Client %d] Connection attempt failed. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 100;
			this->ConnectedSince = 0;
		
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Log("[Client %d] The server is full. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 100;
			this->ConnectedSince = 0;

			break;
		case ID_INVALID_PASSWORD:
			Log("[Client %d] Invalid password. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 2000;
			this->ConnectedSince = 0;

			break;
		case ID_CONNECTION_LOST:
			Log("[Client %d] The connection was lost. Reconnecting", this->uniqueID);

			this->iConnected = false;
			this->iConnectionRequested = false;
			this->ReqConnectTick = GetTickCount() + 100;
			this->ConnectedSince = 0;

			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);
			break;
		case ID_AUTH_KEY:
			Packet_AUTH_KEY(pkt, pRakClient);
			break;
		}

		pRakClient->DeallocatePacket(pkt);
	}

	UpdatePlayerScoresAndPings(1, 4000);
	if (this->iConnected && this->ConnectedSince != -1 && GetTickCount() - this->lastPacketSent >= 2000)
	{
		RakNet::BitStream bs;

		if (packetType == 0)
		{
			struct tempSpecDat {
				UINT16 lrkey;
				UINT16 udkey;
				UINT16 keys;
				float fPos[3];
			};

			tempSpecDat eTempDat;
			memset(&eTempDat, 0, sizeof(tempSpecDat));

			eTempDat.fPos[0] = szBotPos[0];
			eTempDat.fPos[1] = szBotPos[2];
			eTempDat.fPos[2] = szBotPos[2];

			bs.Write(212);
			bs.Write(eTempDat);
		}
		else
		{
			stOnFootData OutgoingOnFootData;
			memset(&OutgoingOnFootData, 0, sizeof(stOnFootData));

			OutgoingOnFootData.fPosition[0] = szBotPos[0];
			OutgoingOnFootData.fPosition[1] = szBotPos[1];
			OutgoingOnFootData.fPosition[2] = szBotPos[2];

			bs.Write(207);
			bs.Write(OutgoingOnFootData);
		}

		pRakClient->Send(&bs, PacketPriority::LOW_PRIORITY, PacketReliability::UNRELIABLE_SEQUENCED, '\0');
		this->lastPacketSent = GetTickCount();
	}
	return 1;
}


SAMPClient::~SAMPClient(void) 
{
	Log("[Client %d] Killing client, bye.", this->uniqueID);
	this->HandleThread.~thread();
	this->pRakClient->Disconnect(500);
	this->pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ConnectionRejected);
    // On delete
}