#include "socket_request.h"
#include "game/client/gameclient.h"

#include <sio_client.h>


void CWebSocket::OnInit()
{
	SocketConnect();
}

void CWebSocket::SocketConnect()
{
	CGameClient* pClient = (CGameClient*)GameClient();
	
	// websocket init here
	pClient->m_SocketIOConnected = false;
	pClient->m_SocketIO.set_open_listener([this, pClient]() {
		pClient->m_SocketIOConnected = true;
		dbg_msg("socket.io", "Connected to server");
	});

	pClient->m_SocketIO.set_close_listener([this, pClient](sio::client::close_reason const& reason) {
		pClient->m_SocketIOConnected = false;
		dbg_msg("socket.io", "Disconnected from server");
	});

	pClient->m_SocketIO.set_fail_listener([this, pClient]() {
		pClient->m_SocketIOConnected = false;
		dbg_msg("socket.io", "Connection failed");
	});

	// Connect
	pClient->m_SocketIO.connect("http://localhost:3001"); //TODO: change to actual API server
}

void CWebSocket::SocketDisconnect()
{
	CGameClient* pClient = (CGameClient*)GameClient();
	if(pClient->m_SocketIOConnected) {
		pClient->m_SocketIO.close();
		pClient->m_SocketIOConnected = false;
	}
}

void CWebSocket::SocketMessage(const char *pEvent, const sio::message::list pData)
{
	CGameClient* pClient = (CGameClient*)GameClient();
	pClient->SendSocketMessage(pEvent, pData);
}
