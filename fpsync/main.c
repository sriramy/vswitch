
#include <stdio.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include "libnetlink/libnetlink.h"


int eth_link(link_action_t action, int ifid,
              struct ether_addr *, int mtu,
              const char *name, uint8_t state,
              uint16_t vlanid, void *args) {
        printf("eth_link\n");
        return 0;
}

int addr4(addr_action_t action, int32_t port_id,
                struct in_addr * addr, uint8_t prefixlen,
                void *args) {
        printf("addr4\n");
        return 0;
}

int addr6(addr_action_t action, int32_t port_id,
                struct in6_addr * addr, uint8_t prefixlen,
                void *args) {
        printf("addr6\n");
        return 0;
}

int route4(struct rtmsg * route, route_action_t action,
                struct in_addr * addr, uint8_t len,
                struct in_addr * nexthop, uint8_t type, void *args) {
        printf("route4\n");
        return 0;
}

int route6(struct rtmsg * route, route_action_t action,
                struct in6_addr * addr, uint8_t len,
                struct in6_addr * nexthop, uint8_t type, void *args) {
        printf("route6\n");
        return 0;
}
                                
void netl_log(const char* msg, uint32_t lvl) {
        printf("log: %s\n", msg);
}

int main()
{
	struct netl_handle* netl_h;

	netl_h = netl_create(NETLINK4_EVENTS);
	if (netl_h == NULL) {
		printf("Error: Couldn't initialize netlink socket\n");
		goto err;
	}

	netl_h->cb.addr4 = addr4;
	netl_h->cb.addr6 = addr6;
	netl_h->cb.route4 = route4;
	netl_h->cb.route6 = route6;
	netl_h->cb.link = eth_link;
	netl_h->cb.log = netl_log;

        netl_listen(netl_h, NULL);

err:
	return 0;
}
