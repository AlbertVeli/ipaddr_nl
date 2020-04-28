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
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>

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

/* callback for nl_cache_foreach, below */
static void __addr_cb(struct nl_object *o, void *data)
{
	char buf[64];
	int ifindex = (int)(intptr_t)data;
	struct rtnl_addr *addr = (struct rtnl_addr *)o;
	if (!addr) {
		return;
	}

	int cur_ifindex = rtnl_addr_get_ifindex(addr);
	if(cur_ifindex != ifindex)
		return;

	const struct nl_addr *local = rtnl_addr_get_local(addr);
	if (!local) {
		return;
	}

	const char *addr_s = nl_addr2str(local, buf, sizeof(buf));
	if (!addr_s) {
		return;
	}
	printf("\n   %s (%d)", addr_s, nl_addr_get_family(local));
}

/* Loop through all interfaces and print if_index, if_name and IP address(es) */
int print_ifaces(void)
{
	struct if_nameindex *if_ni, *i;
	struct nl_sock *sk = cached_nl_sock(0);
	struct nl_cache *addr_cache;
	int err;

	if_ni = if_nameindex();
	if (if_ni == NULL) {
		perror("if_nameindex");
		return 1;
	}

	err = rtnl_addr_alloc_cache(sk, &addr_cache);
	if (err) {
		perror("rtnl_addr_alloc_cache");
		return 2;
	}
	if (nl_cache_is_empty(addr_cache)) {
		perror("nl cache empty");
		return 3;
	}

	for (i = if_ni; !(i->if_index == 0 && i->if_name == NULL); i++) {
		printf("%u: %s", i->if_index, i->if_name);
		nl_cache_foreach(addr_cache, __addr_cb, (void*)(intptr_t)i->if_index);
		printf("\n");
	}

	/* Free allocated structs */
	nl_cache_free(addr_cache);
	if_freenameindex(if_ni);
	cached_nl_sock(1);

	return 0;
}

int main (void)
{
	print_ifaces();

	return 0;
}

