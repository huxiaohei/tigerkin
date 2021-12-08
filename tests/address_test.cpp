/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/address.h"

#include "../src/macro.h"

void test_lookup() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup start";
    std::vector<tigerkin::Address::ptr> rsts;
    if (!tigerkin::IpAddress::Lookup(rsts, "www.baidu.com")) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup fail";
        return;
    }
    for (size_t i = 0; i < rsts.size(); ++i) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << rsts[i]->toString();
    }
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup end";
}

void test_lookup_any() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup any start";
    tigerkin::Address::ptr addr = tigerkin::Address::LookupAny("0.0.0.0");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "addr:" << *addr;
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup any end";
}

void test_iface() {
    std::multimap<std::string, std::pair<tigerkin::IpAddress::ptr, uint32_t>> rts;
    if (!tigerkin::IpAddress::GetInterfaceAddresses(rts, AF_INET)) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test ipv4 interface addresses fail";
    }
    if (!tigerkin::IpAddress::GetInterfaceAddresses(rts, AF_INET6)) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test ipv6 interface addresses fail";
    }
    for (auto i : rts) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n"
                                                    << i.first << ":\n\t"
                                                    << "ip:" << i.second.first->toString() << "\n\t"
                                                    << "prefixlen:" << i.second.second << "\n\t"
                                                    << "subnetmask:" << i.second.first->subnetMask(i.second.second)->toString() << "\n\t"
                                                    << "networkAddress:" << i.second.first->networkAddress(i.second.second)->toString() << "\n\t"
                                                    << "broadcastAddress:" << i.second.first->broadcastAddress(i.second.second)->toString();
    }
    std::vector<std::pair<tigerkin::IpAddress::ptr, uint32_t>> loRst;
    tigerkin::IpAddress::GetInterfaceAddresses(loRst, "lo", AF_INET);
    for (auto i : loRst) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "lo ipv4 interface addresses:\n\t"
                                                    << "ip:" << i.first->toString() << "\n\t"
                                                    << "prefixlen:" << i.second << "\n\t"
                                                    << "subnetmask:" << i.first->subnetMask(i.second)->toString() << "\n\t"
                                                    << "networkAddress:" << i.first->networkAddress(i.second)->toString() << "\n\t"
                                                    << "broadcastAddress:" << i.first->broadcastAddress(i.second)->toString();
    }
}

int main() {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "address test start";
    test_lookup();
    test_lookup_any();
    test_iface();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "address test end";
}
