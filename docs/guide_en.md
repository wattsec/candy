# 如何使用

For detailed usage, please refer to the [configuration file](https://github.com/lanthora/candy/blob/master/candy.cfg).

## Connecting to the Official Network

Start with the default configuration to connect to the official network. The client will cache the address assigned by the server and will prioritize using this address the next time it starts. The address is saved in the `/var/lib/candy` directory. Before starting the container service, you need to create a directory on the Host for mapping; otherwise, data loss due to container restart will result in reassigning the address.

Create a directory identical to the one inside the container for better understanding.

```bash
mkdir -p /var/lib/candy
```

Connect to the official network using a container

```bash
docker run --detach --restart=always --privileged=true --net=host --volume /var/lib/candy:/var/lib/candy docker.io/lanthora/candy:latest
```

Connect to the official network using a Linux distribution [Service](candy.service)

```bash
systemctl enable --now candy
```

Once two or more clients are successfully deployed, they can communicate with each other. Thanks to the routing feature, the more clients in the network, the lower the access latency.

## Deploying a Private Network

Parameters can be specified via command line

```bash
# View available command line parameters
candy --help
```

They can also be specified through a configuration file. When using a configuration file, you need to specify the path. It is recommended to use a configuration file as it allows you to modify the configuration without rebuilding the container.

```bash
# Specify the configuration file path
candy -c /path/to/candy.cfg
```

### Basic Server Configuration

Listen on port 80 of all network interfaces, automatically assign addresses in the 10.0.0.0/24 subnet after the client connects, and set the login password to 123456

```ini
# Work in server mode
mode = "server"
# The server does not support wss, external services like nginx/caddy are needed for encryption, wss is recommended in production environments
websocket = "ws://0.0.0.0:80"
# If not configured, the client needs to specify a static address
dhcp = "10.0.0.0/24"
# If not configured, the password is empty
password = "123456"
```

### Basic Client Configuration

Client configuration matching the above server

```ini
# Work in client mode
mode = "client"
# The example uses ws for plaintext transmission, the client supports the wss protocol
websocket = "ws://127.0.0.1:80"
# Needs to be consistent with the server configuration
password = "123456"

# Static address, can be omitted if the server is configured with dhcp, the server will randomly assign an address
tun = "10.0.0.1/24"
# Network card name, used to distinguish multiple clients on a single machine, the network card name on the same host must not conflict, if not configured, the default network card name candy is used
name = "test"
# STUN server, used to obtain public network information required to establish a peer-to-peer connection, if not configured, peer-to-peer connection is not enabled
stun = "stun://stun.canets.org"
```

