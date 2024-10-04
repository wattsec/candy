// SPDX-License-Identifier: MIT
#ifndef CANDY_WEBSOCKET_CLIENT_H
#define CANDY_WEBSOCKET_CLIENT_H

#include "websocket/common.h"
#include <Poco/Net/WebSocket.h>
#include <string>

namespace Candy {

class WebSocketClient {
public:
    // 连接或断开与服务端的连接
    // Connect or disconnect from the server
    int connect(const std::string &address);
    int disconnect();

    // 设置读超时时间
    // Set read timeout
    int setTimeout(int timeout);

    // 读操作返回 0 表示超时.由于客户端只与一个服务端通信,事实上只需要操作的 buffer,
    // 为了和服务端操作的数据结构保持一直,使用了相同的参数.
    // Read operation returns 0 indicating timeout. Since the client only communicates with one server,
    // it actually only needs to operate the buffer. To keep the data structure consistent with the server,
    // the same parameters are used.
    int read(WebSocketMessage &message);
    int write(const WebSocketMessage &message);

    // 设置 Ping 消息
    // Set Ping message
    int setPingMessage(const std::string &message);
    int sendPingMessage();

private:
    int sendPingMessage(WebSocketMessage &message);

    int timeout;
    std::shared_ptr<Poco::Net::WebSocket> ws;
    int64_t timestamp;
    std::string pingMessage;
};

} // namespace Candy

#endif
