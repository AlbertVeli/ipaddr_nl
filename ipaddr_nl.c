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
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>

/* From libiproute2 (rt_names.c) */
static char *rtnl_rtscope_tab[256] = {
	[RT_SCOPE_UNIVERSE]  = "global",
	[RT_SCOPE_NOWHERE]   = "nowhere",
	[RT_SCOPE_HOST]      = "host",
	[RT_SCOPE_LINK]      = "link",
	[RT_SCOPE_SITE]      = "site",
};

/* From libiproute2 (rt_names.c) */
const char *rtnl_rtscope_n2a(int id, char *buf, int len)
{
	if (id < 0 || id >= 256) {
		snprintf(buf, len, "%d", id);
		return buf;
	}

	if (rtnl_rtscope_tab[id])
		return rtnl_rtscope_tab[id];

	snprintf(buf, len, "%d", id);
	return buf;
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

/* callback for nl_cache_foreach, below */
static void __addr_cb(struct nl_object *obj, void *data)
{
	char addrbuf[64];
	char brdbuf[32];
	char brdbuf2[64];
	char inetbuf[16];
	char scopebuf[64];
	int ifindex = (int)(intptr_t)data;
	struct rtnl_addr *addr = (struct rtnl_addr *)obj;
	int family;
	int scope;
	const char *p;

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

	p = nl_addr2str(local, addrbuf, sizeof(addrbuf));
	if (!p) {
		return;
	}

	const struct nl_addr *brd = rtnl_addr_get_broadcast(addr);
	if (!brd) {
		strcpy(brdbuf2, "");
	} else {
		p = nl_addr2str(brd, brdbuf, sizeof(brdbuf));
		if (!p) {
			strcpy(brdbuf2, "");
		} else {
			snprintf(brdbuf2, sizeof(brdbuf2), " brd %s", brdbuf);
		}
	}

	scope = rtnl_addr_get_scope(addr);
	p = rtnl_rtscope_n2a(scope, scopebuf, sizeof(scopebuf));

	family = nl_addr_get_family(local);
	nl_af2str(family, inetbuf, sizeof(inetbuf));
	printf("\n    %s %s%s scope %s", inetbuf, addrbuf, brdbuf2, p);
}

/* Loop through all interfaces and print if_index, if_name and IP address(es)
 *
 * Based on example code for man page if_nameindex
 * https://linux.die.net/man/3/if_nameindex
 */
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
		/* One interface can have more than one address attached */
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

