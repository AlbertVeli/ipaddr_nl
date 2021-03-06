/* Example of how to use netlink to show ifconfig / ip address
 * information directly from C code.
 *
 * This will only work on platforms where netlink is available.
 *
 * Pungenday, the 45th day of Discord in the YOLD 3186
 *
 * Albert Veli
 *
 * License: LGPLv2 (same as libnl)
 */
#include <stdio.h>
#include <string.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/link/vlan.h>
#include <netlink/route/link/bridge.h>

/* Enough? */
#define MAX_IFINDEX 256
/* vids[ifindex] = vid */
static int vids[MAX_IFINDEX];

/* Return vlan ifindex for this vid, or 0 if not found. */
static int vlan_ifindex(int vid)
{
	int i;

	for (i = 1; i < MAX_IFINDEX; i++) {
		if (vids[i] == vid)
			return i;
	}

	/* Not found */
	return 0;
}

/* If !free, alloc nl_sock else free previously allocated one */
static struct nl_sock *cached_nl_sock(int free)
{
	static struct nl_sock *sk = NULL;

	if (free) {
		if (sk) {
			nl_socket_free(sk);
			sk = NULL;
		}
		return NULL;
	}

	/* Return cached nl_sock */
	if (sk)
		return sk;

	/* Alloc and return new nl_sock */
	sk = nl_socket_alloc();
	if (!sk)
		return NULL;

	if (nl_connect(sk, NETLINK_ROUTE)) {
		nl_socket_free(sk);
		return NULL;
	}

	return sk;
}

static void __link_cb(struct nl_object *obj, void *arg __attribute__((unused)))
{
	char buf[64];
	struct rtnl_link *link = (struct rtnl_link *)obj;
	//const char *something = arg;
	char *name;
	int ifindex;
	int mtu;
	int master;
	struct nl_addr *addr;

	name = rtnl_link_get_name(link);
	if (!name)
		return;
	addr = rtnl_link_get_addr(link);
	if (!addr)
		return;
	master = rtnl_link_get_master(link);
	ifindex = rtnl_link_get_ifindex(link);
	mtu = rtnl_link_get_mtu(link);

	rtnl_link_operstate2str(rtnl_link_get_operstate(link), buf, sizeof(buf));
	printf("%d: %s, mtu %d, master %d, %s", ifindex, name, mtu, master, buf);
	const char *addr_s = nl_addr2str(addr, buf, sizeof(buf));
	if (addr_s)
		printf(", %s", addr_s);

	if (rtnl_link_is_vlan(link)) {
		int vid = rtnl_link_vlan_get_id(link);
		if (ifindex >= 0 && ifindex < MAX_IFINDEX) {
			vids[ifindex] = vid;
			printf(" vid %d", vid);
		}
	}

	if (rtnl_link_is_bridge(link)) {
		printf(" bridge");
	}

	printf("\n");
}

int print_links(void)
{
	struct nl_sock *sk = cached_nl_sock(0);
	struct nl_cache *links;
	int err;

	memset(vids, 0, sizeof(vids));

	err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &links);
	if (err) {
		perror("rtnl_link_alloc_cache");
		return 1;
	}
	if (nl_cache_is_empty(links)) {
		perror("nl cache empty");
		nl_cache_free(links);
		return 2;
	}

	nl_cache_foreach(links, __link_cb, NULL);

	/* Free allocated structs */
	nl_cache_free(links);

	return 0;
}

int main (void)
{
	int vid, ifindex;

	print_links();

	/* Free allocated nl_sock */
	cached_nl_sock(1);

	/* Test if vlan_ifindex works.
	 * This will only print something
	 * if vlan interface with vid 1 exists.
	 */
	vid = 1;
	ifindex = vlan_ifindex(vid);
	if (ifindex > 0)
		printf("\nifindex for vlan%d: %d\n", vid, ifindex);

	return 0;
}

