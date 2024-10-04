# SD-WAN Networking

## Requirements

When there are multiple local area networks (LANs) with __non-conflicting addresses__ in multiple locations, it is desired that devices within one LAN can directly access devices in other LANs using their addresses, __as if they are in the same wide area network (WAN)__, without deploying the Candy client on all devices.

## Example

Assuming you have already:

- Successfully deployed the server
- Deployed Candy on the gateway and assigned a virtual address

Taking LAN A as an example to explain the table's meaning.

- The network address of the LAN is `192.168.1.0/24`, which should not conflict with B and C
- The gateway can be a router or any Linux system within the LAN, but it needs to be able to deploy the Candy client. Assume its address in the LAN is `192.168.1.1`. By configuring routes on devices within the LAN, ensure that traffic can enter the gateway
- The Candy client is deployed on the gateway, and its address in the virtual network is `10.0.0.1`

| LAN     | A              | B              | C              |
| :------ | :------------- | :------------- | :------------- |
| Network | 192.168.1.0/24 | 192.168.2.0/24 | 192.168.3.0/24 |
| Gateway | 192.168.1.1    | 192.168.2.1    | 192.168.3.1    |
| Candy   | 10.0.0.1       | 10.0.0.2       | 10.0.0.3       |

When `192.168.1.x` accesses `192.168.2.x`, the traffic is expected to be delivered as follows:

```txt
192.168.1.x => 10.0.0.1 => 10.0.0.2 => 192.168.2.x
```

Next, the configuration process for this route is explained. The configuration method for other routes is the same.

### Configure Routes on the Candy Server

Add the following configuration to the server, which will modify the system routes on the gateway.

```ini
sdwan = "10.0.0.1/32,192.168.2.0/24,10.0.0.2"
```

| Device   | Device Mask     | Dest Net    | Dest Mask     | Gateway  |
| :------- | :-------------- | :---------- | :------------ | :------- |
| 10.0.0.1 | 255.255.255.255 | 192.168.2.0 | 255.255.255.0 | 10.0.0.2 |

After adding the system route, the client of `10.0.0.1` will be able to receive IP packets destined for `192.168.2.x`. Candy is responsible for forwarding these packets to `10.0.0.2`.

### Forward Traffic to the Gateway

If the gateway is a router, no action is needed, and the traffic should be able to enter the gateway.

Otherwise, you need to configure routes on non-gateway devices to forward traffic to the gateway.

- Destination network: 192.168.2.0/24
- Gateway: 192.168.1.1

### Allow Gateway to Forward Traffic

#### Linux

If your gateway is a router, it should be easy to configure it to allow forwarding. Otherwise, you need to manually add forwarding-related configurations.

Enable kernel traffic forwarding

```bash
sysctl -w net.ipv4.ip_forward=1
```

Determine the network interface through which traffic enters the gateway. Assume it is `ethX`, and the interface name used by Candy is `candy-gw`.

```bash
iptables -t nat -A POSTROUTING -o candy-gw -j MASQUERADE
iptables -A FORWARD -i ethX -o candy-gw -j ACCEPT
iptables -A FORWARD -i candy-gw -o ethX -m state --state RELATED,ESTABLISHED -j ACCEPT
```

#### Windows

Check the network adapter name, which should match the name written in the configuration file. For the default configuration of the GUI version client, the network adapter name should be `candy`.

```ps
Get-NetAdapter
```

Enable forwarding, and replace the network adapter name with the one found in the previous step.

```ps
Set-NetIPInterface -ifAlias 'candy' -Forwarding Enabled
```

### Testing

Use the ping command to send packets from `192.168.1.x`, and capture packets on `192.168.2.x` to see the corresponding packets. To see the ping command's reply, complete the return route configuration as described above.

## Common Issues

### Unable to Access NAT Virtual Machine

You can directly access the virtual machine IP from the host, but other machines cannot access it through the host's virtual IP forwarding. This is likely a firewall issue. For example, if the virtual machine uses a NAT network with libvirt, libvirt will add firewall rules that only allow access from the host IP.

You can view the host rules with the following command.

```bash
nft list ruleset
```

You will see a reject hit.

```txt
table ip libvirt_network {
    chain guest_output {
        ip saddr 192.168.100.0/24 iif "virbr0" counter packets 3568 bytes 541261 accept
        iif "virbr0" counter packets 0 bytes 0 reject
    }

    chain guest_input {
        oif "virbr0" ip daddr 192.168.100.0/24 ct state established,related counter packets 3237 bytes 290974 accept
        oif "virbr0" counter packets 8 bytes 844 reject
    }
}
```

Now, you just need to solve the firewall issue. I chose to add two higher-priority rules to prevent the reject from hitting.

```bash
nft insert rule ip libvirt_network guest_output iif "virbr0" accept
nft insert rule ip libvirt_network guest_input oif "virbr0" accept
```

Viewing the rules again will show two additional accept rules at the beginning, and the network should be accessible without issues.

```txt
table ip libvirt_network {
    chain guest_output {
        iif "virbr0" accept
        ip saddr 192.168.100.0/24 iif "virbr0" counter packets 3568 bytes 541261 accept
        iif "virbr0" counter packets 0 bytes 0 reject
    }

    chain guest_input {
        oif "virbr0" accept
        oif "virbr0" ip daddr 192.168.100.0/24 ct state established,related counter packets 3237 bytes 290974 accept
        oif "virbr0" counter packets 8 bytes 844 reject
    }
}
```

Throughout the process, [pwru](https://github.com/cilium/pwru) helped identify the existence of netfilter firewall rules at the kernel level.

