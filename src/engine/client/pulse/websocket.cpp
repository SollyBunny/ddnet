#include "websocket.h"
#include <base/system.h>

namespace DDNetPulse {

class WebSocketImpl : public WebSocket {
private:
    sio::client m_Client;
    sio::socket::ptr m_Socket;
    ConnectCallback m_OnConnect;
    ConnectCallback m_OnDisconnect;
    ErrorCallback m_OnError;
    bool m_IsConnecting;

public:
    WebSocketImpl() : m_IsConnecting(false) {
        // Set up default Socket.IO event handlers
        m_Client.set_open_listener([this]() {
            m_Socket = m_Client.socket();
            m_IsConnecting = false;
            if(m_OnConnect) m_OnConnect();
        });

        m_Client.set_close_listener([this](sio::client::close_reason const& reason) {
            m_IsConnecting = false;
            if(m_OnDisconnect) m_OnDisconnect();
        });

        m_Client.set_fail_listener([this]() {
            m_IsConnecting = false;
            if(m_OnError) m_OnError("Connection failed");
        });
    }

    ~WebSocketImpl() override {
        Disconnect();
    }

    bool Connect(const std::string& uri) override {
        if(m_IsConnecting || IsConnected()) {
            if(m_OnError) m_OnError("Already connecting or connected");
            return false;
        }

        m_IsConnecting = true;
        m_Client.connect(uri);
        return true;
    }

    void Disconnect() override {
        m_IsConnecting = false;
        if(m_Socket) {
            m_Socket->close();
            m_Socket.reset();
        }
        m_Client.close();
    }

    bool IsConnected() const override {
        return m_Client.opened();
    }

    void On(const std::string& event, MessageCallback callback) override {
        if(!m_Socket) {
            if(m_OnError) m_OnError("Cannot register event handler - not connected");
            return;
        }
        
        m_Socket->on(event, [callback](sio::event& ev) {
            if(ev.get_message()) {
                std::string data;
                if(ev.get_message()->get_flag() == sio::message::flag_string) {
                    data = ev.get_message()->get_string();
                }
                callback(ev.get_name(), data);
            }
        });
    }

    void Off(const std::string& event) override {
        if(!m_Socket) {
            if(m_OnError) m_OnError("Cannot unregister event handler - not connected");
            return;
        }
        m_Socket->off(event);
    }

    bool Emit(const std::string& event, const std::string& data) override {
        if(!m_Socket || !IsConnected()) {
            if(m_OnError) m_OnError("Cannot emit event - not connected");
            return false;
        }
        
        m_Socket->emit(event, data);
        return true;
    }

    void SetOnConnect(ConnectCallback callback) override {
        m_OnConnect = std::move(callback);
    }

    void SetOnDisconnect(ConnectCallback callback) override {
        m_OnDisconnect = std::move(callback);
    }

    void SetOnError(ErrorCallback callback) override {
        m_OnError = std::move(callback);
    }
};

std::unique_ptr<WebSocket> WebSocket::Create() {
    return std::make_unique<WebSocketImpl>();
}

} // namespace DDNetPulse