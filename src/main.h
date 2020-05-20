/*
	Updated to 0.3.DL by KevY
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h> // need sum co�os and vaginas
#include <windows.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include "common/common.h"

// window stuff
#include <commctrl.h>

#include <vector>

// raknet stuff
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakClientInterface.h"
#include "NetworkTypes.h"
#include "BitStream.h"
#include "StringCompressor.h"

#pragma warning(disable:4996)

#include "samp_netencr.h"
#include "samp_auth.h"
#include "SAMPRPC.h"

#include "misc_funcs.h"
#include "math_stuff.h"
#include "query.h"

#include "tinyxml/tinyxml.h"
#include "console.h"
#include "xmlsets.h"
#include "ClientFactory.h"

void UserInput(char* szInput);
void Log ( char *fmt, ... );
void gen_random(char *s, const int len);