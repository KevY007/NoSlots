/*
	Updated to 0.3.DL by KevY
*/


/* TO - DO LIST: 
	
	- Go to a specific location/stuck inside a player to cause lag.
	- Update players.
	- Commands on UserInput()

*/
#include "main.h"

FILE* flLog = NULL;

void UserInput(char* szInput)
{
	if(strcmp(szInput, "status") == 1)
	{
		for each (auto & ePtr in clients)
		{
			Log("Unique ID for Client %d: %d", ePtr->uniqueID, ePtr->GetInterface()->GetUniqueID());
		}
	}
	if (strcmp(szInput, "toggleonfoot") == 1)
	{
		for each (auto & ePtr in clients)
		{
			ePtr->packetType = !ePtr->packetType;
		}
		Log("Toggle Onfoot Movement!");
	}
	if (strcmp(szInput, "tog_reconnect") == 1)
	{
		settings.spamreconnect = !settings.spamreconnect;
		Log("Spam Reconnect Status: %d", settings.spamreconnect);
	}
	return;
}

void HandleClientThread(SAMPClient* ePtr)
{
	char eTmpNick[23] = { 0 };

	while (1)
	{
		if (ePtr->iConnected == true && settings.spamreconnect && ePtr->ConnectedSince > 1)
		{
			if (settings.reconnect_delay < GetTickCount() - ePtr->ConnectedSince)
			{
				Log("[Client %d] Spam Reconnect is ON, reconnecting...", ePtr->uniqueID);
				
				
				if (settings.fixedname) {
					sprintf(eTmpNick, "%s_%d%c", settings.server.szNickname, ePtr->uniqueID, 'a' + ((ePtr->uniqueID + rand() % 100) % 26));
				}
				else gen_random(eTmpNick, 19);

				ePtr->ChangeName(eTmpNick);

				ePtr->Disconnect();
			}
		}

		ePtr->UpdateNetwork();

		Sleep(30);
	}
	return;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	srand((unsigned int)GetTickCount());

	// load up settings
	if(!LoadSettings())
	{
		Log("Failed to load settings");
		getchar();
		return 0;
	}

	SetUpConsole();

	// set up networking

	SYSTEMTIME time;
	GetLocalTime(&time);
	
	Log(" ");
	Log("* ======================== [ No Slots ] ======================== *");
	Log("  Empty slots? Say no more.");
	Log("  By KevY, completely rewritten over RakSAMP's base.");
	Log("* ======================== [ No Slots ] ======================== *");

	Log(" ");
	Log("*   Settings Start   *");
	Log(" ");

	int bVer = VERSION_037;
	if (strcmp(settings.server.version, "0.3.7") == 1) bVer = VERSION_037;
	if (strcmp(settings.server.version, "0.3.DL") == 1) bVer = VERSION_03DL;
	if (strcmp(settings.server.version, "0.3DL") == 1) bVer = VERSION_03DL;

	Log("* Version: %s (%d)", settings.server.version, bVer);
	Log("* Server: %s:%d", settings.server.szAddr, settings.server.iPort);
	Log("* Base Name: %s", settings.fixedname == 1 ? settings.server.szNickname : "Random Names");
	if(settings.spamreconnect) Log("* Spam Reconnect: [ON] | Spam Reconnect Delay: [%d]", settings.reconnect_delay);

	Log(" ");
	Log("*   Settings End   *");
	Log(" ");

	

	Log("[NoSlots] Creating %d bots...\n", settings.numbots);
	char eTmpNick[23] = { 0 };
	for (int i = 0; i < settings.numbots; i++)
	{
		if (settings.fixedname) {
			sprintf(eTmpNick, "%s_%d%c", settings.server.szNickname, i, 'a' + ((i + rand() % 100) % 26));
		}
		else gen_random(eTmpNick, 19);

		SAMPClient* eSAMPClient = new SAMPClient(settings.server.szAddr, settings.server.iPort, settings.server.szPassword, eTmpNick, bVer);
		if (eSAMPClient->GetInterface() == NULL)
		{
			Log("RakClient Interface is NULL for Client %d (%d) Halt!", eSAMPClient->uniqueID, i);
			Sleep(3000);
			continue;
		}
		
		eSAMPClient->HandleThread = std::thread(HandleClientThread, eSAMPClient);
		eSAMPClient->HandleThread.detach();

		Sleep(settings.connectDelay);
	}

	while (1)
	{
		Sleep(500);
	}
	
	return 0;
}

void Log(char *fmt, ...)
{
	remove("NoSlots.log");

	if(flLog == NULL)
	{
		flLog = fopen("NoSlots.log", "a");

		if(flLog == NULL)
			return;
	}

	SYSTEMTIME time;
	GetLocalTime(&time);

	char szPrintBuf[1024] = { 0 };

	char buffer[512];
	memset(buffer, 0, 512);

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, 512, fmt, args);
	va_end(args);

	sprintf(szPrintBuf, "[%02d:%02d:%02d.%03d] %s\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, buffer);

	printf(szPrintBuf);
	fprintf(flLog, szPrintBuf);

	fflush(flLog);
}


void gen_random(char *s, const int len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i)
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	s[len] = 0;
}
