/*
	Updated to 0.3.DL by KevY
*/

// MAIN STUFF /////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#define NOSLOTS_VERSION "v0.1-R1"

#define NETCODE_CONNCOOKIELULZ 0x6969

#define AUTHOR "KevY / 0x32789"

#define REJECT_REASON_BAD_VERSION	1
#define REJECT_REASON_BAD_NICKNAME	2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4

#define BP _asm int 3

typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int		int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#define isfinite	_finite
