#include <net/gnrc/ipv6/nib.h>
#include <net/gnrc/ipv6.h>
#include <net/gnrc/netapi.h>
#include <net/gnrc/netif.h>
#include <net/ipv6/addr.h>

#include "nat_ip.h"


int init_ip(void)
{
#if defined(BR_IPV6_ADDR) && defined(BR_IPV6_DEFRTR) && defined(BR_IPV6_USE_IID)
    // Assuming client with single network interface, so blindly using first (and hopefully only).
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);

    if (!netif)
        return 0;

    ipv6_addr_t addr;
    ipv6_addr_from_str(&addr, BR_IPV6_ADDR);

#if BR_IPV6_USE_IID
    eui64_t iid;
    if (gnrc_netapi_get(netif->pid, NETOPT_IPV6_IID, 0, &iid, sizeof(iid)) < 0) {
        printf("Failed getting interface iid.\n");
        return -1;
    }
    ipv6_addr_set_aiid(&addr, iid.uint8);
#endif

    if (gnrc_netif_ipv6_addr_add(netif, &addr, 64, 0) < 0) {
        printf("Failed setting address.\n");
        return -1;
    }

    gnrc_ipv6_nib_change_rtr_adv_iface(netif, false);

    // Add default route
    ipv6_addr_t defroute = IPV6_ADDR_UNSPECIFIED;
    ipv6_addr_from_str(&addr, BR_IPV6_DEFRTR);
    if (gnrc_ipv6_nib_ft_add(&defroute, 0, &addr, netif->pid, 0) < 0) {
        printf("Failed setting default route.\n");
        return -1;
    }
#endif

    return 0;
}
