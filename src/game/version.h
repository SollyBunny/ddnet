/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VERSION_H
#define GAME_VERSION_H

// ddnet
#define GAME_NAME "DDNet"
#define DDNET_VERSION_NUMBER 19030
extern const char *GIT_SHORTREV_HASH;
#ifndef GAME_RELEASE_VERSION_INTERNAL
#define GAME_RELEASE_VERSION_INTERNAL 19.3
#endif
#define GAME_RELEASE_VERSION STRINGIFY(GAME_RELEASE_VERSION_INTERNAL)

// teeworlds
#define CLIENT_VERSION7 0x0705
#define GAME_VERSION "0.6.4, " GAME_RELEASE_VERSION
#define GAME_NETVERSION "0.6 626fce9a778df4d4"
#define GAME_NETVERSION7 "0.7 802f1be60a05665f"

// custom
#define CLIENT_NAME "Solly"
#ifndef TCLIENT_VERSION
#define TCLIENT_VERSION "10.3.0"
#endif
#define INF_VERSION "0.2"
#define SOLLY_VERSION "0.0.0"
#define BUILD_DATE __DATE__ " " __TIME__
#define CLIENT_NAME_FULL "Solly: " SOLLY_VERSION " TClient: " TCLIENT_VERSION " Inf: " INF_VERSION " DDNet: " GAME_RELEASE_VERSION " Built: " BUILD_DATE " Commit: %s"

#endif
