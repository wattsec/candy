// SPDX-License-Identifier: MIT
#include "websocket/client.h"
#include "utility/time.h"
#include "utility/uri.h"
#include <Poco/Exception.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Timespan.h>
#include <spdlog/spdlog.h>

namespace Candy {

int WebSocketClient::connect(const std::string &address) {
    Uri uri(address);
    if (!uri.isValid()) {
        spdlog::critical("invalid websocket server: {}", address);
        return -1;
    }
    try {
        const std::string path = uri.path().empty() ? "/" : uri.path();
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;
        if (uri.scheme() == "wss") {
            // FIXME: MSYS2 下编译的 Windows 版本会因为找不到证书而导致无法连接,暂时关闭证书校验.
            using Poco::Net::Context;
            Context::Ptr context = new Context(Context::TLS_CLIENT_USE, "", "", "", Context::VERIFY_NONE);
            Poco::Net::HTTPSClientSession cs(uri.host(), uri.port().empty() ? 443 : std::stoi(uri.port()), context);
            this->ws = std::make_shared<Poco::Net::WebSocket>(cs, request, response);
        } else if (uri.scheme() == "ws") {
            Poco::Net::HTTPClientSession cs(uri.host(), uri.port().empty() ? 80 : std::stoi(uri.port()));
            this->ws = std::make_shared<Poco::Net::WebSocket>(cs, request, response);
        } else {
            spdlog::critical("invalid websocket scheme: {}", address);
            return -1;
        }
        this->ws->setReceiveTimeout(Poco::Timespan(this->timeout, 0));
        this->timestamp = Time::bootTime();
        return 0;
    } catch (std::exception &e) {
        spdlog::critical("websocket connect failed: {}", e.what());
        return -1;
    }
}

int WebSocketClient::disconnect() {
    if (this->ws) {
        this->ws->close();
    }
    return 0;
}

int WebSocketClient::setTimeout(int timeout) {
    this->timeout = timeout;
    return 0;
}

int WebSocketClient::read(WebSocketMessage &message) {
    if (!this->ws) {
        spdlog::critical("websocket read before connected");
        return -1;
    }

    try {
        char buffer[1500] = {0};
        int flags = 0;
        int length = this->ws->receiveFrame(buffer, sizeof(buffer), flags);
        if (length == 0 && flags == 0) {
            message.type = WebSocketMessageType::Error;
            message.buffer = "abnormal disconnect";
            return 1;
        }
        if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PING) {
            flags = (int)Poco::Net::WebSocket::FRAME_FLAG_FIN | (int)Poco::Net::WebSocket::FRAME_OP_PONG;
            this->ws->sendFrame(buffer, length, flags);
            this->timestamp = Time::bootTime();
            return 0;
        }
        if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PONG) {
            this->timestamp = Time::bootTime();
            return 0;
        }
        if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
            message.type = WebSocketMessageType::Close;
            message.buffer.assign(buffer, length);
            return 1;
        }
        if (length > 0) {
            message.type = WebSocketMessageType::Message;
            message.buffer.assign(buffer, length);
            this->timestamp = Time::bootTime();
            return 1;
        }
        return 0;
    } catch (Poco::TimeoutException const &e) {
        if (Time::bootTime() - this->timestamp > 30000) {
            message.type = WebSocketMessageType::Error;
            message.buffer = "websocket pong timeout";
            return 1;
        }
        if (Time::bootTime() - this->timestamp > 15000) {
            return sendPingMessage(message);
        }
        return 0;
    } catch (std::exception &e) {
        message.type = WebSocketMessageType::Error;
        message.buffer = e.what();
        return 1;
    }
}

int WebSocketClient::write(const WebSocketMessage &message) {
    if (!this->ws) {
        spdlog::critical("websocket write before connected");
        return -1;
    }

    try {
        this->ws->sendFrame(message.buffer.c_str(), message.buffer.length(), Poco::Net::WebSocket::FRAME_BINARY);
        return 0;
    } catch (std::exception &e) {
        spdlog::critical("websocket write failed: {}", e.what());
        return -1;
    }
}

int WebSocketClient::setPingMessage(const std::string &message) {
    this->pingMessage = message;
    spdlog::debug("set ping message: {}", this->pingMessage);
    return 0;
}

int WebSocketClient::sendPingMessage() {
    WebSocketMessage wsMessage;
    return sendPingMessage(wsMessage);
}

int WebSocketClient::sendPingMessage(WebSocketMessage &message) {
    try {
        int flags = (int)Poco::Net::WebSocket::FRAME_FLAG_FIN | (int)Poco::Net::WebSocket::FRAME_OP_PING;
        this->ws->sendFrame(this->pingMessage.c_str(), this->pingMessage.size(), flags);
        return 0;
    } catch (std::exception &e) {
        message.type = WebSocketMessageType::Error;
        message.buffer = e.what();
        return 1;
    }
}

} // namespace Candy
