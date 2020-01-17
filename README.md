# wlt
a tool based on C and lua to switch route

## About the C program

The main part written by C requires cJSON.

The usage is based on nftables named sets. You should first create nft sets in mangle table like

```
table ip mangle {
	set default1 {
		type ipv4_addr
		flags timeout
	}

	set default2 {
		type ipv4_addr
		flags timeout
	}

	set global1 {
		type ipv4_addr
		flags timeout
	}

	chain prerouting {
		type filter hook prerouting priority -150; policy accept;
		ip saddr @default1 counter mark set mark & 0xfffffff1 | 0x00000001	# The marks are used in PBRs 
		ip saddr @default2 counter mark set mark & 0xfffffff2 | 0x00000002
		ip saddr @global1 counter mark set mark & 0xffffff1f | 0x00000010
	}
	<other chains or rules>
}
```

The ip rules should be like
```
0:	from all lookup local 
5:	from all lookup main 
10:	from <wan ip> lookup wan # default via <gateway> dev <wan iface> This ensures where in where out
170:	from all fwmark 0x10/0xf0 lookup global1  # <ip> via <endpoint internal ip in tunnel> dev <iface> (*n)
180:	from all fwmark 0x1/0xf lookup default1   # default via <endpoint internal ip in tunnel> dev <iface>
180:	from all fwmark 0x2/0xf lookup default2
32767:	from all lookup default
```

The routes and set names can be defined by yourself. You should also modify the conf file to make it same with nftables.

The conf file of the above example can be (json)
```
{
	"sets": {
		"default": ["default1", "default2"],
		"other": [],
		"global": ["global1"]
	}
}
```

The usage is
```
/path/to/wlt <ip> <reset> <conffile> <route1> <route2> ... <route_n> <timeout>
```
n is the "group" amount in the conf file. In the above example, it is 3.

Usage example:
```
./wlt 192.168.80.100 noreset /etc/wlt.conf 2 0 9 300
```

This means that you are modifying the route of 192.168.80.100

"noreset" means you are not resetting all routes. Any strings other than "reset" will not reset the routes.

2 means change the route in first group to the second one, namely "default1".

0 means not changing the route in second group.

9 means resetting the route in third group. So the ip is not in any set of the third group. The packets will not be marked the specified bits, either.

## Call the program by http
