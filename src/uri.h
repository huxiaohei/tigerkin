/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/26
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

/**
 * 参考标准
 * https://www.ietf.org/rfc/rfc3986.txt
 *
 *   foo://user@example.com:8042/over/there?name=ferret#nose
 *   \_/   \___________________/\_________/ \_________/ \__/
 *    |           |                  |            |       |
 * scheme     authority             path        query   fragment
 *    |   ___________________________|_____
 *   / \ /                                 \
 *   urn:example:over:there:name:ferret:nose
 *
 * authority: [user@host:port]
 */

#ifndef __TIGERKIN_URI_H__
#define __TIGERKIN_URI_H__

#include <stdint.h>

#include <memory>
#include <string>

#include "address.h"

namespace tigerkin {

class Uri {
   public:
    typedef std::shared_ptr<Uri> ptr;

    Uri() : m_port(0){};

    Address::ptr createAddress() const;

    const std::string& getScheme() const { return m_scheme; }
    const std::string& getUserInfo() const { return m_userInfo; };
    const std::string getHost() const { return m_host; }
    const std::string& getPath() const;
    int32_t getPort() const;
    const std::string& getQuery() const { return m_query; }
    const std::string& getFragment() const { return m_fragment; }

    void setScheme(std::string v) { m_scheme = v; }
    void setUserInfo(std::string v) { m_userInfo = v; };
    void setHost(std::string v) { m_host = v; }
    void setPort(int32_t v) { m_port = v; }
    void setPath(std::string v) { m_path = v; }
    void setQuery(std::string v) { m_query = v; }
    void setFragment(std::string v) { m_fragment = v; }

    bool isDefaultPort() const;
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;

   public:
    static Uri::ptr Create(const std::string& uristr);

   private:
    std::string m_scheme;
    std::string m_userInfo;
    std::string m_host;
    int32_t m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
};

}  // namespace tigerkin

#endif  // !__TIGERKIN_URI_H__