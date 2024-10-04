# Client Relay

Currently, there is an issue: Suppose there are three clients A, B, and C. A can establish peer-to-peer connections with both B and C, but B and C cannot establish a peer-to-peer connection. In this case, the connection can only degrade to being forwarded through the server. In fact, letting A relay the traffic between B and C is a better solution.

In reality, relay A might be multiple devices, and the link is not just A. The problem can be expanded to: How to find the least costly path among several relay links in the network to establish a connection.

The solution to this problem is [routing algorithms](https://www.google.com/search?q=routing+algorithms), which this project implements based on.

## How to Determine the Cost of Communication Between Two Nodes

Currently, latency is the only criterion. If two nodes cannot establish a peer-to-peer connection, the cost is considered infinite. When a node is congested, the latency will increase. As the cost increases, the node can automatically choose other lower-cost routes.

A new DELAY packet is added to the peer-to-peer connection, carrying the virtual addresses of the sender and receiver, as well as a 64-bit millisecond-level timestamp from the sender's system startup to the current moment. The receiver does not process the packet but only returns it to the sender. After receiving the packet it sent, the sender calculates the latency based on the timestamp in the packet.

## How to Update the Routing Table

The routing table records the cost Z from X to Y. The initial state of the routing table is empty.

After successfully establishing a peer-to-peer connection with a client, obtain the latency through the DELAY packet and record it in the table. At this time, X equals Y, and Z is the latency in milliseconds. When a new client establishes a peer-to-peer connection, repeat the above operation and broadcast this record to devices that have already successfully established peer-to-peer connections. When broadcasting, adjust X to the local machine.

When detecting that the peer-to-peer connection with a device is disconnected, delete all records with the next hop as that address from the routing table. Each record is broadcast once, similar to when establishing a connection, except that Z is a special value indicating disconnection.

In addition to establishing and disconnecting connections, clients should periodically obtain latency, update, and broadcast.

After receiving the record, other devices compare the latency of using this route with the currently recorded latency: If the latency increases (or disconnects), and the destination address and next hop address are the same as the local record, then update (or delete) the local record and broadcast; if the latency is lower, update the record and broadcast.

## How to Forward Packets

When sending a packet, look up the routing table based on the destination address. If the lookup fails, use the server to forward it. If the lookup succeeds, encrypt the packet in a way that the next hop can decrypt and send it. After receiving the packet, the next hop checks the destination address. If it is the local machine, it processes it directly; otherwise, it continues to find the next hop.

## Other Notes

Some devices may have high bandwidth costs and are not suitable as relay devices, so a "routing cost" parameter is added to describe this situation. The "routing cost" will be added to the actual round-trip latency between two devices as the routing latency broadcast.
`route = 0` means this function is not enabled. The maximum value is 1000, corresponding to a real round-trip latency of 1 second. If you want to use this device as a relay as much as possible, you should configure `route = 1`.

Since this function can only be used among devices that have already established peer-to-peer connections, an active discovery function is added. For devices willing to actively act as relays, this function can be enabled to let as many clients as possible discover them.
`discovery = 0` means this function is not enabled. `discovery = 60` means sending a broadcast packet every 60 seconds.
