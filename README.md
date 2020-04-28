Short example of how to use netlink from C code to show
(some of the) information from ifconfig / ip address.

Can be extended to show more information about each interface using
more calls to the [netlink api](https://www.infradead.org/~tgr/libnl/doc/api/group__rtaddr.html).

This will only work on platforms where netlink is available (i.e. GNU/Linux)

Pungenday, the 45th day of Discord in the YOLD 3186

Albert Veli

License: LGPLv2 (same as libnl, which it uses)
