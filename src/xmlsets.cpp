

/*
	Updated to 0.3.DL by KevY
*/

#include "main.h"

struct stSettings settings;
TiXmlDocument xmlSettings;

int LoadSettings()
{
	// load xml
	if(!xmlSettings.LoadFile("NoSlots.xml"))
	{
		MessageBox(NULL, "Failed to load the config file", "Error", MB_ICONERROR);
		ExitProcess(0);
	}

	TiXmlElement* mainElement = xmlSettings.FirstChildElement("NoSlots");
	if(mainElement)
	{
		TiXmlElement* serverElement = mainElement->FirstChildElement("server");
		if(serverElement)
		{
			char *pszAddr = (char *)serverElement->GetText();
			if(pszAddr)
			{
				int iPort = 7777;
				char *pszAddrBak = pszAddr;

				while(*pszAddrBak)
				{
					if(*pszAddrBak == ':')
					{
						*pszAddrBak = 0;
						pszAddrBak++;
						iPort = atoi(pszAddrBak);
					}
					pszAddrBak++;
				}

				strcpy(settings.server.szAddr, pszAddr);
				settings.server.iPort = iPort;
				serverElement->QueryIntAttribute("fixedname", (int*)&settings.fixedname);
				strcpy(settings.server.szNickname, (char *)serverElement->Attribute("basenick"));
				strcpy(settings.server.szPassword, (char *)serverElement->Attribute("password"));
			}
		}

		TiXmlElement* valuesElement = mainElement->FirstChildElement("values");
		if (valuesElement)
		{
			valuesElement->QueryIntAttribute("bots", (int*)&settings.numbots);
			valuesElement->QueryIntAttribute("bot_delay", (int*)&settings.connectDelay);
			strcpy(settings.server.version, (char*)valuesElement->Attribute("version"));
		}

		TiXmlElement* togglesElement = mainElement->FirstChildElement("toggles");
		if (togglesElement)
		{
			togglesElement->QueryIntAttribute("spam_reconnect", (int*)&settings.spamreconnect);
			togglesElement->QueryIntAttribute("reconnect_delay", (int*)&settings.reconnect_delay);
		}
	}

	xmlSettings.Clear();

	return 1;
}

int UnLoadSettings()
{
	memset(&settings, 0, sizeof(settings));

	return 1;
}

int ReloadSettings()
{
	if(UnLoadSettings() && LoadSettings())
	{
		Log("Settings reloaded");
		return 1;
	}

	Log("Failed to reload settings");

	return 0;
}
