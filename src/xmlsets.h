#pragma once
/*
	Updated to 0.3.DL by KevY
*/

struct stServer
{
	char szAddr[256];
	int iPort;
	char szNickname[14];
	char szPassword[32];
	char version[8];
};

struct stSettings
{
	struct stServer server;
	
	int fixedname;

	int numbots;
	int connectDelay;
	int spamreconnect;
	int reconnect_delay;
};
extern struct stSettings settings;

int LoadSettings();
int UnLoadSettings();
int ReloadSettings();
