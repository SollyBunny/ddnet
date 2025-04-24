 #pragma once

#include <string>
#include <functional>
#include <memory>
#include <sio_client.h>

namespace DDNetPulse {

class WebSocket {
public:
	// Callback types
	using MessageCallback = std::function<void(const std::string& event, const std::string& data)>;
	using ConnectCallback = std::function<void()>;
	using ErrorCallback = std::function<void(const std::string& error)>;

	// Connection states
	enum class State {
		Disconnected,
		Connecting,
		Connected,
		Error
	    };

	static std::unique_ptr<WebSocket> Create();
	virtual ~WebSocket() = default;

	// Connection
	virtual bool Connect(const std::string& uri) = 0;
	virtual void Disconnect() = 0;
	virtual bool IsConnected() const = 0;

	// Message
	virtual void On(const std::string& event, MessageCallback callback) = 0;
	virtual void Off(const std::string& event) = 0;
	virtual bool Emit(const std::string& event, const std::string& data) = 0;

	// Event
	virtual void SetOnConnect(ConnectCallback callback) = 0;
	virtual void SetOnDisconnect(ConnectCallback callback) = 0;
	virtual void SetOnError(ErrorCallback callback) = 0;
};

} // namespace DDNetPulse