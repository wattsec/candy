// SPDX-License-Identifier: MIT
#if defined(_WIN32) || defined(_WIN64)

#include "peer/peer.h"
#include "utility/address.h"
#include <spdlog/spdlog.h>
#include <string.h>
// clang-format off
#include <winsock2.h>
#include <mstcpip.h>
// clang-format on

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

namespace Candy {

UdpHolder::UdpHolder() {
    this->socket = INVALID_SOCKET;
    return;
}

UdpHolder::~UdpHolder() {
    SOCKET winsock = std::any_cast<SOCKET>(this->socket);
    if (winsock != INVALID_SOCKET) {
        closesocket(winsock);
        this->socket = INVALID_SOCKET;
    }
    return;
}

int UdpHolder::init() {
    this->socket = INVALID_SOCKET;
    SOCKET winsock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (winsock == INVALID_SOCKET) {
        spdlog::error("create udp socket failed: {}", WSAGetLastError());
        return -1;
    }
    // https://stackoverflow.com/a/42409198
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(winsock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
    // set non-blocking
    u_long mode = 1;
    if (ioctlsocket(winsock, FIONBIO, &mode) != NO_ERROR) {
        closesocket(winsock);
        spdlog::error("set non-blocking failed: {}", WSAGetLastError());
        return -1;
    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(this->port);
    if (bind(winsock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        closesocket(winsock);
        spdlog::error("socket binding failed: {}", WSAGetLastError());
        return -1;
    }
    this->socket = winsock;
    return 0;
}

uint16_t UdpHolder::getBindPort() {
    if (this->port) {
        return this->port;
    }

    SOCKET winsock = std::any_cast<SOCKET>(this->socket);
    if (winsock != INVALID_SOCKET) {
        struct sockaddr_in local;
        int len = sizeof(local);
        memset(&local, 0, sizeof(local));
        if (getsockname(winsock, (struct sockaddr *)&local, &len) != SOCKET_ERROR) {
            this->port = ntohs(local.sin_port);
            return this->port;
        }
    }
    return 0;
}

uint32_t UdpHolder::getDefaultIP() {
    if (this->ip) {
        return this->ip;
    }
    SOCKET winsock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (winsock == INVALID_SOCKET) {
        return 0;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = 0x12345678;
    addr.sin_family = AF_INET;
    if (connect(winsock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(winsock);
        return 0;
    }
    int len = sizeof(addr);
    if (getsockname(winsock, (struct sockaddr *)&addr, &len) == SOCKET_ERROR) {
        closesocket(winsock);
        return 0;
    }
    this->ip = ntohl(addr.sin_addr.s_addr);
    closesocket(winsock);
    return this->ip;
}

size_t UdpHolder::read(UdpMessage &message) {
    SOCKET winsock = std::any_cast<SOCKET>(this->socket);
    if (winsock == INVALID_SOCKET) {
        spdlog::error("udp socket not initialized successfully");
        return -1;
    }

    message.buffer.resize(1500);
    struct sockaddr_in from;
    int addr_len = sizeof(from);
    memset(&from, 0, sizeof(from));

    int len = recvfrom(winsock, message.buffer.data(), message.buffer.size(), 0, (struct sockaddr *)&from, &addr_len);
    if (len != SOCKET_ERROR) {
        message.buffer.resize(len);
        message.ip = Address::netToHost((uint32_t)from.sin_addr.s_addr);
        message.port = Address::netToHost((uint16_t)from.sin_port);
        return len;
    }
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK) {
        struct timeval timeout = {.tv_sec = 1};
        fd_set set;
        FD_ZERO(&set);
        FD_SET(winsock, &set);
        select(0, &set, NULL, NULL, &timeout);
        return 0;
    }
    spdlog::error("udp socket read failed: {}", error);
    return -1;
}

size_t UdpHolder::write(const UdpMessage &message) {
    SOCKET winsock = std::any_cast<SOCKET>(this->socket);
    if (winsock == INVALID_SOCKET) {
        spdlog::error("udp socket not initialized successfully");
        return -1;
    }

    struct sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = Address::hostToNet(message.ip);
    to.sin_port = Address::hostToNet(message.port);
    int len = sendto(winsock, message.buffer.c_str(), message.buffer.length(), 0, (struct sockaddr *)&to, sizeof(to));
    if (len != SOCKET_ERROR) {
        return len;
    }
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK) {
        return 0;
    }
    spdlog::debug("udp socket write failed: {}", error);
    return -1;
}

} // namespace Candy

#endif
