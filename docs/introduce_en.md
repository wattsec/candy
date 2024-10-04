# 介绍

## What is a Virtual Private Network

There are many proxy tools on the internet claiming to be VPNs (Virtual Private Networks). Strictly speaking, these tools should be called Proxies. The difference is: VPNs allow devices within the virtual network to access each other; Proxies use one device as a stepping stone to access another device.

Typical VPNs include OpenVPN, IPSec, and WireGuard, which is integrated into the Linux kernel. Typical Proxies include Socks5, Shadowsocks, and V2Ray.

This project can and only can help you network multiple devices together.

## Why create another Virtual Private Network tool

Many classic VPNs mentioned above already meet the vast majority of user scenarios. However, their protocol characteristics are obvious, which becomes a drawback in China's special network environment, as firewalls can easily identify and block the traffic. I was once a WireGuard user, and under the interference of the firewall, I lost connection with other devices in the network. Therefore, it is necessary to design a VPN with the idea of designing a proxy, giving the VPN some firewall resistance capabilities.

## Overall Design Concept

The design principle is simplicity. The goal is to complete the design with the least amount of code and the simplest concepts without sacrificing performance and core functionality.

### Reducing Configuration Complexity

WireGuard is already relatively simple to configure among VPNs, but it is still too complex for me. Recall how long it took you to complete your first WireGuard network setup. WireGuard requires mandatory specification of virtual addresses, which is not suitable for scenarios where multiple clients need to be flexibly connected and dynamically assigned addresses.

Using WSS (Web Socket Secure) for communication ensures the security of link data while eliminating the process of configuring public and private keys. Using password authentication for clients makes it easy to add new clients to the network, allowing the server to implement dynamic address allocation.

### Efficient Reconnection

In some cases, WireGuard disconnects, and only restarting the client can solve the problem. For an unattended device, this means complete disconnection. I once configured the device to restart once a day to solve this problem, which is obviously an ugly solution.

Using WSS communication allows Ping/Pong to complete TCP keep-alive. Even if the TCP connection is abnormally disconnected, the application can detect and handle it promptly.

### Peer-to-Peer Connection with NAT Traversal

Although WireGuard supports peer-to-peer connections, it requires devices to be directly accessible to each other, which is powerless for situations where both parties are behind NAT. Adding NAT traversal functionality can save server forwarding traffic and reduce communication latency.

NAT traversal obtains the public address and port mapped by the local UDP Socket through a STUN server, exchanges address and port information with other clients through the server, and attempts to establish a connection.

### High-Speed Client Relay

Sometimes two clients cannot connect directly, but they can both connect directly to another client. In this case, the other client can act as a relay for mutual access.

This is essentially a [routing](https://en.wikipedia.org/wiki/Routing) problem. This project implements the distance vector algorithm.

### Software-Defined Wide Area Network

Deploy the Candy client on the gateway and configure the system routing to form a wide area network from multiple local area networks.

