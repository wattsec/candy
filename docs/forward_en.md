# Server Relay

## The Simplest TUN Usage

Refer to [simpletun](https://github.com/gregnietsky/simpletun) for the simplest P2P connection implementation. Here we name two devices as Client A and Client B.

```plaintext
┌──────────┐  ┌──────────┐
│ Client A ├──┤ Client B │
└──────────┘  └──────────┘
```

## Server Forwarding Traffic

Based on the simplest P2P connection, add an intermediate device to forward traffic without changing the packet content. For the two clients, the traffic is no different from the previous scenario.

```plaintext
┌──────────┐  ┌────────┐  ┌──────────┐
│ Client A ├──┤ Server ├──┤ Client B │
└──────────┘  └────────┘  └──────────┘
```

## Adding Routing Functionality

The server adds routing functionality, recording the mapping relationship between each client's virtual address and connection. It analyzes the destination address of the original IPv4 packet and finds the corresponding connection to send data through the mapping.

```plaintext
              ┌──────────┐
              │ Client D │
              └────┬─────┘
                   │
┌──────────┐   ┌───┴────┐   ┌──────────┐
│ Client A ├───┤ Server ├───┤ Client B │
└──────────┘   └───┬────┘   └──────────┘
                   │
              ┌────┴─────┐
              │ Client C │
              └──────────┘
```

