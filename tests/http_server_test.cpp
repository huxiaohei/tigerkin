/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

void run() {
    tigerkin::http::HttpServer::ptr server(new tigerkin::http::HttpServer());
    tigerkin::Address::ptr addr = tigerkin::IpAddress::LookupAnyIpAddress("0.0.0.0:8080");
    while (!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2));
    iom->schedule(&run);
    iom->start();
    return 0;
}