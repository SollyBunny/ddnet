/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VERSION_H
#define GAME_VERSION_H

extern const char *GIT_SHORTREV_HASH;
#define BUILD_DATE __DATE__ ", " __TIME__

#ifndef GAME_RELEASE_VERSION
#define GAME_RELEASE_VERSION "19.3"
#endif

// teeworlds
#define CLIENT_VERSION7 0x0705
#define GAME_VERSION "0.6.4, " GAME_RELEASE_VERSION
#define GAME_NETVERSION "0.6 626fce9a778df4d4"
#define GAME_NETVERSION7 "0.7 802f1be60a05665f"

// ddnet
#define DDNET_VERSION_NUMBER 19030
#define GAME_NAME "DDNet"

// mods
#define SOLLY_VERSION "0.0.0"
#define TCLIENT_VERSION "10.1.2"
#define INF_VERSION "0.2"

// client
#define CLIENT_VERSION "0.0.0"
#define CLIENT_NAME "Solly"
#define CLIENT_NAME_FULL "Solly: " SOLLY_VERSION " TClient: " TCLIENT_VERSION " Inf: " INF_VERSION " DDNet: " GAME_RELEASE_VERSION " Built: " BUILD_DATE " Commit: %s"

#endif
