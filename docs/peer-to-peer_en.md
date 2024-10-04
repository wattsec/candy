# Peer-to-Peer Connection

## Protocol Specification

During the establishment and use of peer-to-peer connections using UDP, there are specific formatted messages that are encrypted throughout the process.

### Heartbeat and Handshake

Heartbeat and handshake use the exact same message format.

- Type: Used to distinguish packet types, always 0
- TUN: Local virtual address
- IP: Public address of this client used for peer-to-peer connection
- Port: Public port of this client used for peer-to-peer connection listening
- ACK: 0 or 1, indicating whether the heartbeat or handshake message from the other party has been received, used to update the peer-to-peer connection state machine

```plaintext
+------------------------------------------------------------------------------+
| Type (1 Byte) | TUN (4 Bytes) | IP (4 Bytes) | Port (2 Bytes) | ACK (1 Byte) |
+------------------------------------------------------------------------------+
```

### Raw IPv4 Packet Forwarding

The format is consistent with the packets forwarded through WebSocket, but this uses UDP transmission.

- Type: Used to distinguish packet types, always 1
- Raw: Raw data at the IP layer

```plaintext
+-------------------------------+
| Type (1 Byte) | Raw (n Bytes) |
+-------------------------------+
```

### Latency Detection

Send a latency detection packet to the other party, and the other party will reply without processing it. The sender can then calculate the latency between the sender and the receiver.

- Type: Used to distinguish packet types, always 2
- Src: Virtual source address
- Dst: Virtual destination address
- Timestamp: Milliseconds elapsed since the machine was started

```plaintext
+---------------------------------------------------------------------+
| Type (1 Byte) | Src (4 Bytes) | Dst (4 Bytes) | Timestamp (8 Bytes) |
+---------------------------------------------------------------------+
```

### Route Synchronization

Broadcast route changes to all directly connected devices.

- Type: Used to distinguish packet types, always 4
- Dst: Virtual destination address
- Next: Virtual next-hop address
- Delay: Measured latency

```plaintext
+------------------------------------------------------------------+
| Type (1 Byte) | Dst (4 Bytes) | Next (4 Bytes) | Delay (8 Bytes) |
+------------------------------------------------------------------+
```

## State Machine

### INIT

When receiving an IPv4 packet forwarded by the server through WebSocket, check if the source address is in the INIT state. If it is, enter the PREPARING state. If the peer-to-peer connection feature is not enabled, switch to the FAILED state. Ignore other states.

### PREPARING

Periodically check if there are peers in the PREPARING state. If there are, send STUN requests after traversing all peers. When a STUN response is received, send its public network information to all peers in the PREPARING state. If there is no public network information of the peer, switch to SYNCHRONIZING state, otherwise switch to CONNECTING state. STUN uses UDP and may lose packets, so as long as there are peers in the PREPARING state, it will continuously send STUN requests.

### SYNCHRONIZING

This state indicates that the public network information of the local machine has been successfully obtained, but the public network information of the peer has not yet been received. At this time, send a heartbeat without ACK. The other party may be sending, or the version does not support, or the peer-to-peer connection is not enabled. If it times out, enter the FAILED state.

### CONNECTING

This state indicates that the public network information of the peer is available. At this time, send a heartbeat with ACK. Receiving a heartbeat with ACK from the other party indicates a successful connection and enters the CONNECTED state. If it times out without success, enter the WAITING state.

### CONNECTED

When TUN receives data, if the peer is in the CONNECTED state, it is directly sent via UDP. Periodically send heartbeats to the peer and check if the peer's heartbeat has been received recently. If the peer's heartbeat has not been received for a long time, enter the INIT state. When UDP receives the peer's heartbeat, it will reset the heartbeat timeout counter.

### WAITING

Use an exponential backoff algorithm to re-enter the INIT state after a specific time. The retry interval gradually increases from 30 seconds to 1 hour.

### FAILED

The FAILED state indicates that the peer does not support peer-to-peer connections. No proactive measures are taken for peers in this state. However, if a connection message from the peer is received, it will passively enter the PREPARING state. This corresponds to the situation where the peer switches from a client that does not support peer-to-peer connections to a client that supports peer-to-peer connections.
