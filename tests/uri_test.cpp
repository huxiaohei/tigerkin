/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/27
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Uri::ptr uri = tigerkin::Uri::Create("http://www.huxiaohei.com/test/uri?id=10086&name=tigerkin#frg");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "URI:\n\t"
                                                << uri->toString();
    auto addr = uri->createAddress();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "ADDRESS:\n\t"
                                                << addr->toString();
}