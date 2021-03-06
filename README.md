# tigerkin

## 开发环境

* CentOS Linux release 8.4.2105
* gcc version 8.4.1
* gcc-c++ version 8.4.1
* cmake version 3.18.2
* boost version 1.66.0
* yaml-cpp version 0.6.2
* ragel version 7.0.0.9

## 配置系统

使用配置前需要先声明，即约定优于配置，如果没有提前声明配置不会解析对应字段

* 支持`std`基本数据类型
  * `std::string`,`std::vector`,`std::list`,`std::set`,`set::unordered_set`,`std::map`,`std::unordered_map`
  * 支持自定义类型(需要自己写对于的偏特化模板)

## 日志系统(同步)

* 日志器支持配置初始化
* 支持标准控制台输出
* 支持文件输出
  * 运行过程中日志文件被误删除，可自动重新生成
* 支持格式自定义

  | 标识 | 说明 | 标识 | 说明 | 标识 | 说明 |
  |:---:|:---:|:---:|:---:|:---:|:---:|
  | m | 消息 | p | 日志级别 | r | 累计毫秒数 |
  | c | 日志名称 | t | 线程id | n | 换行 |
  | d | 时间 | f | 文件名 | l | 行号 |
  | T | Tab | F | 协程id | N | 线程名称 |


## 线程系统

基于对`pthread`的封装，使用灵活简单

* 信号量
* 支持读写分离互斥锁
  * 注意读锁是线程共享资源
  * 写锁是独占资源
* 多线程
  * 使用简单并且确保在获取到子线程`id`的同时线程已经开始执行
  * 线程号和线程名称与`top`命令中的线程对应
* 线程的挂起与恢复

## 协程系统

基于对`ucontext`封装的非对称协程，使用灵活简单

* 非对称设计，每个协程`coroutine`只有唤醒`resume`和挂起`yield`两个操作，在哪里挂起，当唤醒之后就会回到哪里
* 协程一旦被唤醒就会保存执行协程的线程号，这个协成之后就只能在这个线程中执行
* 每个线程在运行第一个协程时都会创建一个`Main`协程即线程当前运行切入点
  * 在一个协程被首次唤醒
    * 如果是从`Main`协程唤醒，会创建一个对应的栈并将这个协程放入栈底
    * 如果不是从`Main`协程唤醒，那么会将这个协程放入唤醒这个协程的协程所在栈的栈顶
* `yield`只对栈顶协程生效，因为非栈顶协程已经是`YIELD`状态
* `resume`只对栈顶协程和未加入栈中的协程生效
* 可以通过`stackId`来`resume`一个栈的栈顶协程

## 调度器

对协程系统做的进一步封装，使用方便灵活
* 支持多线程操作
  * 初始化调度器时，线程数最好不要超过`CPU`核数(线程上线文切换会消耗一定资源)
* 可指定协程在哪个线程中执行
* 参数`userCaller`可设置调度线程(创建调度器的线程)是否要参与协程任务的执行
  * 当`userCaller`为`true`时，在`start`处将会创建相应子线程，调度线程和子线程都会执行协程池中的协程
  * 当`userCaller`为`false`时，在`start`处将会创建相应子线程，子线程会执行协程池中的协程，而调度线程会继续向下执行
* 协程任务执行完毕时除调度线程外，所有线程都会陷入挂起状态(让出`CPU`资源)等待被再次唤醒

## I/O管理器

基于`epoll`对调度器的扩展，使用方便，灵活，扩展性高
* 支持多线程操作
* 等待事件将会放入一个新的协程中执行
* 支持毫秒级定时器
* 支持条件定时器

## Hook

`hook`系统底层先关API功能。`hook`是控制是线程粒度的，可以自由选择是否开启。通过`hook`可以让一些不具备一步功能的`API`，展现出异步的功能

* 支持`socket io`相关`API`
* 支持`sleep`系列函数

## Socket

分装地址类提供统一的地址(`IPv4`、`IPv6`、`Unix`)操作(域名、`IP`解析等)相关接口。封装`Socket`类提供所有`Socket API`功能

## ByteArray序列化模块

`ByteArray`二进制序列化模块，提供对二进制的常用操作

* 支持读写基础数据类型`int8_t`、`int16_t`、`int32_t`、`int64_t`、`varint`、`std::string`等
* 支持字节序转换
* 支持序列化到文件和从文件反序列化

## TcpServer

基于`Socket`类，封装了一个通用的`TcpServer`的服务器类，提供简单的`API`，使用便捷，可以快速绑定一个或多个地址，启动服务，监听端口，`accept`连接，处理`socket`连接等功能。具体业务功能服务器实现，只需要继承该类就可以快速实现

## Stream

封装流式的统一接口。将文件、`socket`封装成统一的接口。使用的时候，采用统一的风格操作。基于统一的风格，可以提供更灵活的扩展。

* 支持`SocketStream`

## HTTP

采用`Ragel`(有限状态机，性能媲美汇编)，实现`HTTP/1.1`的简单协议实现和`uri`的解析。基于`SocketStream`实现`HTTP`客户端和服务端

* 支持`HTTP`客户端(`HttpConnection`)
* 支持`HTTP`服务端(`HttpSession`)
* 支持`HTTP`连接池(`HttpConnectionPool`)

## Servlet

实现了`ServletDispatch`、`FunctionServlet`、`NotFoundServlet`等，配合`HTTP`模块，提供`HTTP`服务器

* 支持`uri`的精准和模糊匹配

## 示例

### Http服务

```cpp
#include "../src/tigerkin.h"

int32_t pingpong(tigerkin::http::HttpRequest::ptr req,
                 tigerkin::http::HttpResponse::ptr rsp,
                 tigerkin::http::HttpSession::ptr sess) {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "receive http:\n"
                                                   << req->toString();
    if (req->getMethod() == tigerkin::http::HttpMethod::GET) {
        rsp->setBody(req->toString());
    } else {
        rsp->setBody(req->getBody());
    }
    rsp->setHeaders(req->getHeaders());
    return 0;
}

int32_t hello(tigerkin::http::HttpRequest::ptr req,
              tigerkin::http::HttpResponse::ptr rsp,
              tigerkin::http::HttpSession::ptr sess) {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "receive http:\n"
                                                   << req->toString();
    rsp->setBody("Hello! I'm tigerkin!");
    return 0;
}

void run() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER START";
    auto httpServer = std::make_shared<tigerkin::http::HttpServer>();
    auto addr = tigerkin::IpAddress::LookupAnyIpAddress("0.0.0.0:8080");
    while (!httpServer->bind(addr)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER BIND ERROR\n"
                                                       << addr->toString();
        sleep(2000);
    }
    auto dispatch = httpServer->getServletDispatch();
    dispatch->addServlet("/", &hello);
    dispatch->addServlet("/hello", &hello);
    dispatch->addServlet("/pingpong", &pingpong);
    dispatch->addServlet("/stop", [httpServer](tigerkin::http::HttpRequest::ptr req,
                                                    tigerkin::http::HttpResponse::ptr rsp,
                                                    tigerkin::http::HttpSession::ptr sess) {
        auto iom = tigerkin::IOManager::GetThis();
        iom->addTimer(2000, []() {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER STOP";
        });
        rsp->setBody("close http server");
        return 0;
    });
    httpServer->start();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("../conf/log.yml", "logs");
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2));
    iom->schedule(&run);
    iom->start();
    return 0;
}
```

## 其它

欢迎`star`、`watching`、`fork`、`issue`

* 联系方式
  * QQ: 517829514
  * EMail: huxiaoheigame@gmail.com