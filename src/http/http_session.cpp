/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_session.h"

namespace tigerkin {

namespace http {

HttpSession::HttpSession(Socket::ptr socket, bool owner)
    : SocketStream(socket, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buffSize = HttpRequestParser::GetHttpRequestBufferSize();
    std::shared_ptr<char> buffer(new char[buffSize], [](char *ptr) {
        delete[] ptr;
    });
    char *data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buffSize - offset);
        if (len < 0) {
            close();
            return nullptr;
        }
        len += offset;
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (int)buffSize) {
            close();
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "BUFFER BIGGER THAN MAX BUFFER";
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    } while (true);
    int64_t length = parser->getContentLength();
    if (length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if (length >= offset) {
            memcpy(&body[0], data, offset);
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        if (length > 0) {
            if (readFixLength(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    parser->getData()->init();
    return parser->getData();
}

ssize_t HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixLength(data.c_str(), data.size());
}

}  // namespace http
}  // namespace tigerkin
