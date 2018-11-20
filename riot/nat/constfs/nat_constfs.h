#pragma once

#define NAT_CONSTFS_MOUNTPOINT "/const"
#define NAT_CONSTFS_DEFAULT_FILE "default.ujcpak"
#define NAT_CONSTFS_DEFAULT_PATH NAT_CONSTFS_MOUNTPOINT "/" NAT_CONSTFS_DEFAULT_FILE

int init_nat_constfs(void);
