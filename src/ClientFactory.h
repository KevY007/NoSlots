#pragma once
#include "main.h"

#define VERSION_037     0
#define VERSION_03DL    1

struct stOnFootData
{
    uint16_t	sLeftRightKeys;
    uint16_t	sUpDownKeys;
    union
    {
        uint16_t			sKeys;
        struct stSAMPKeys*	stSampKeys;
    };
    float		fPosition[3];
    float		fQuaternion[4];
    uint8_t		byteHealth;
    uint8_t		byteArmor;
    uint8_t		byteCurrentWeapon;
    uint8_t		byteSpecialAction;
    float		fMoveSpeed[3];
    float		fSurfingOffsets[3];
    uint16_t	sSurfingVehicleID;
    short		sCurrentAnimationID;
    short		sAnimFlags;
};

extern float szBotPos[3];

class SAMPClient {
public:
    SAMPClient(char *hostname, int port, char *password, char *nick, int version);   // This is the constructor declaration
    ~SAMPClient();  // This is the destructor: declaration
    int UpdateNetwork();
    void Disconnect();
    void ChangeName(char* name);
  
    RakClientInterface* SAMPClient::GetInterface();

    int uniqueID = -1;
    char szAddr[256] = { 0 };
    int iPort = 7777;
    int iVersion = VERSION_037;
    char szInitialName[23] = { 0 };
    char szNickname[23] = { 0 };
    char szPassword[32] = { 0 };

    bool iConnected = false;
    bool iConnectionRequested = false;
    int packetType = 0;
    DWORD ConnectedSince = 0;
    DWORD ReqConnectTick = -1;
    DWORD lastPacketSent = -1;

    std::thread HandleThread;

private:
    RakClientInterface* pRakClient;

    void Packet_ConnectionSucceeded(Packet* p);
    void UpdatePlayerScoresAndPings(int iWait, int iMS);
};

extern std::vector<SAMPClient*> clients;
extern int identifier;

void ConnectionRejected(RPCParameters* rpcParams);