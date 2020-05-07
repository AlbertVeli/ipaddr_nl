#!/bin/sh

# Helper script to find if port present in iface (given by index).
# Return 0 if present, else ret != 0
# Called by lldpd. See Mantis issue #17740

if [ $# != 2 ]; then
	echo "Usage: $0 <port> <interface index>"
	exit 1
fi

# Port name
port=$1

# Interface index, integer (as shown by ip addr)
ifindex=$2

portifindex=`ip address show $port | head -1 | cut -d: -f1`

# Get ifname from ifindex
ifname=`ip addr | grep "^${ifindex}:" | awk '{ print $2 }' | tr -d ':$' | cut -d@ -f1`
if [ "$ifname" == "" ]; then
	echo "couldn't get interface name"
	exit 2
fi

# Port attached to bridge?
vid=`bridge vlan | grep $port | awk '{ print $2 }'`
if [ "$vid" == "" ]; then
	# Not attached to bridge
	# Check if port matches ifindex
	if [ "$portifindex" == "$ifindex" ]; then
		echo -n "Found"
		exit 0
	else
		echo "port $port doesnt match ifindex $ifindex"
		exit 3
	fi
fi

# Check that port ifindex is attached to bridge
ifindexinbridge=`bridge link | grep "^${portifindex}:"`
if [ "$ifindexinbridge" == "" ]; then
	# ifindex not attached to bridge
	echo "port ifindex $portifindex not attached to bridge"
	exit 4
fi

# Check that vids matches
ifvid=`ip -d link show $ifname | grep '802.1Q id' | awk '{ print $5 }'`
if [ "$ifvid" == "$vid" ]; then
	echo -n "Found"
	exit 0
fi

# Not found
echo "Nope"
exit 5
