/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/address.h"

#include "../src/macro.h"

void test_lookup() {
    std::vector<tigerkin::IpAddress::ptr> rsts;
    if (!tigerkin::IpAddress::Lookup(rsts, "www.baidu.com", "http")) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test lookup fail" << std::endl;
        return;
    }
    for (size_t i = 0; i < rsts.size(); ++i) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << rsts[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<tigerkin::IpAddress::ptr, uint32_t>> rts;
    if (!tigerkin::IpAddress::GetInterfaceAddress(rts, AF_INET6)) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "test iface fail" << std::endl;
        return;
    }
    for (auto i : rts) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n" << i.first << ":\n\t"
                                                    << "ip:" << i.second.first->toString() << "\n\t"
                                                    << "prefixlen:" << i.second.second << "\n\t"
                                                    << "broadcast:" << i.second.first->broadcastAddress(i.second.second)->toString() << "\n\t"
                                                    << "network:" << i.second.first->networkAddress(i.second.second)->toString() << "\n\t"
                                                    << "subnetMask:" << i.second.first->subnetMask(i.second.second)->toString();
    }
}

int main() {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "address test start";
    test_lookup();
    test_iface();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "address test end";
}
