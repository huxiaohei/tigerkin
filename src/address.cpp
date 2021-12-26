/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/06
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "address.h"

#include <ifaddrs.h>
#include <stddef.h>

#include <iomanip>

#include "endian.h"
#include "macro.h"

namespace tigerkin {

template <class T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template <class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    while (value) {
        value &= value - 1;
        ++result;
    }
    return result;
}

Address::ptr Address::Create(const sockaddr *addr, socklen_t addrlen) {
    if (addr == nullptr) {
        return nullptr;
    }
    Address::ptr result;
    switch (addr->sa_family) {
        case AF_INET:
            result.reset(new IpV4Address(*(const sockaddr_in *)addr));
            break;
        case AF_INET6:
            result.reset(new IpV6Address(*(const sockaddr_in6 *)addr));
            break;
        default:
            result.reset(new UnknowAddress(*addr));
            break;
    }
    return result;
}

bool Address::Lookup(std::vector<Address::ptr> &result, const ::std::string &host, int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char *service = NULL;

    if (!host.empty() && host[0] == '[') {
        const char *endipv6 = (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
        if (endipv6) {
            if (*(endipv6 + 1) == ':') {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }
    if (node.empty()) {
        service = (const char *)memchr(host.c_str(), ':', host.size());
        if (service) {
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }
    if (node.empty()) {
        node = host;
    }
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if (error) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "GET ADDR INFO FAIL:"
                                                     << "\n\thost:" << host
                                                     << "\n\terror:" << error
                                                     << "\n\terrstr:" << gai_strerror(error);
        return false;
    }

    next = results;
    while (next) {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !result.empty();
}

Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if (Lookup(result, host, family, type, protocol)) {
        return result[0];
    }
    return nullptr;
}

bool IpAddress::GetInterfaceAddresses(std::multimap<std::string, std::pair<IpAddress::ptr, uint32_t>> &rsts, int family) {
    struct ifaddrs *nxt, *rst;
    if (getifaddrs(&rst) != 0) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET IFADDRS ERROR:\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    try {
        for (nxt = rst; nxt != NULL; nxt = nxt->ifa_next) {
            if (!nxt->ifa_addr || nxt->ifa_addr->sa_family != family) {
                continue;
            }
            IpAddress::ptr addr;
            uint32_t prefixLen = ~0u;
            switch (nxt->ifa_addr->sa_family) {
                case AF_INET: {
                    addr.reset(new IpV4Address(*(const sockaddr_in *)nxt->ifa_addr));
                    uint32_t netmask = ((sockaddr_in *)nxt->ifa_netmask)->sin_addr.s_addr;
                    prefixLen = CountBytes(netmask);
                    break;
                }
                case AF_INET6: {
                    addr.reset(new IpV6Address(*(const sockaddr_in6 *)nxt->ifa_addr));
                    in6_addr &netmask = ((sockaddr_in6 *)nxt->ifa_netmask)->sin6_addr;
                    prefixLen = 0;
                    for (int i = 0; i < 16; ++i) {
                        prefixLen += CountBytes(netmask.s6_addr[i]);
                    }
                    break;
                }
                default:
                    break;
            }
            if (addr) {
                rsts.insert(std::make_pair(nxt->ifa_name, std::make_pair(addr, prefixLen)));
            }
        }
        return true;
    } catch (const std::exception &e) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET INTERFACE ADDRESS ERROR:\n\t"
                                                      << "error:" << e.what();
        return false;
    }
}

bool IpAddress::GetInterfaceAddresses(std::vector<std::pair<IpAddress::ptr, uint32_t>> &result, const std::string &iface, int family) {
    if (iface.empty() || iface == "*") {
        if (family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(IpV4Address::ptr(new IpV4Address()), 0u));
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(IpV6Address::ptr(new IpV6Address()), 0u));
        }
        return true;
    }
    std::multimap<std::string, std::pair<IpAddress::ptr, uint32_t>> rsts;
    if (!GetInterfaceAddresses(rsts, family)) {
        return false;
    }
    auto its = rsts.equal_range(iface);
    while (its.first != its.second) {
        result.push_back(its.first->second);
        ++its.first;
    }
    return !result.empty();
}

int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &rhs) const {
    size_t minLen = std::min(getAddrLen(), rhs.getAddrLen());
    int rt = memcmp(getAddr(), rhs.getAddr(), minLen);
    if (rt > 0) {
        return true;
    } else if (rt < 0) {
        return false;
    }
    return getAddrLen() > rhs.getAddrLen();
}

bool Address::operator==(const Address &rhs) const {
    return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address &ths) const {
    return !(*this == ths);
}

IpAddress::ptr IpAddress::LookupAnyIpAddress(const std::string &host, int family, int type, int protocol) {
    std::vector<Address::ptr> rst;
    if (Lookup(rst, host, family, type, protocol)) {
        for (auto &i : rst) {
            IpAddress::ptr v = std::dynamic_pointer_cast<IpAddress>(i);
            if (v) {
                return v;
            }
        }
    }
    return nullptr;
}

IpV4Address::IpV4Address(const sockaddr_in &addr) {
    m_addr = addr;
}

IpV4Address::IpV4Address(uint32_t address, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
    m_addr.sin_port = byteswapOnLittleEndian(port);
}

IpV4Address::IpV4Address(const char *address, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    int rt = inet_pton(AF_INET6, address, &m_addr.sin_addr);
    if (rt <= 0) {
        std::stringstream ss;
        ss << "IPV4 ADDRESS INIT ERROR:\n\t"
           << "address:" << address << "\n\t"
           << "port:" << port << "\n\t"
           << "rt:" << rt << "\n\t"
           << "errno:" << errno << "\n\t"
           << "strerror:" << strerror(errno);
        throw std::logic_error(ss.str());
    }
}

const sockaddr *IpV4Address::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IpV4Address::getAddr() {
    return (sockaddr *)&m_addr;
}

const socklen_t IpV4Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &IpV4Address::insert(std::ostream &os) const {
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff) << ":"
       << byteswapOnLittleEndian(m_addr.sin_port);
    return os;
}

IpAddress::ptr IpAddress::Create(const char *address, uint16_t port) {
    addrinfo hints, *rsts;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    int err = getaddrinfo(address, NULL, &hints, &rsts);
    if (err) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "IP ADDRESS CREATE FAIL:\n\t"
                                                     << "address:" << address << "\n\t"
                                                     << "error:" << err << "\n\t"
                                                     << "errno:" << errno << "\n\t"
                                                     << "errstr:" << strerror(errno);
        return nullptr;
    }
    try {
        IpAddress::ptr result = std::dynamic_pointer_cast<IpAddress>(Address::Create(rsts->ai_addr, (socklen_t)rsts->ai_addrlen));
        if (result) {
            result->setPort(port);
        }
        freeaddrinfo(rsts);
        return result;
    } catch (...) {
        freeaddrinfo(rsts);
        return nullptr;
    }
}

IpAddress::ptr IpV4Address::broadcastAddress(uint32_t prefixLen) {
    if (prefixLen > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(baddr));
}

IpAddress::ptr IpV4Address::networkAddress(uint32_t prefixLen) {
    if (prefixLen > 32) {
        return nullptr;
    }
    sockaddr_in naddr(m_addr);
    naddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(naddr));
}

IpAddress::ptr IpV4Address::subnetMask(uint32_t prefixLen) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(subnet));
}

uint16_t IpV4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IpV4Address::setPort(uint16_t v) {
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

IpV6Address::ptr IpV6Address::Create(const char *address, uint16_t port) {
    IpV6Address::ptr rt(new IpV6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    int rst = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
    if (rst <= 0) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "TPV6 ADDRESS CREATE FAIL:\n\t"
                                                     << "address:" << address << "\n\t"
                                                     << "port:" << port << "\n\t"
                                                     << "errno:" << errno << "\n\t"
                                                     << "strerror:" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IpV6Address::IpV6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IpV6Address::IpV6Address(const sockaddr_in6 &addr) {
    m_addr = addr;
}

IpV6Address::IpV6Address(const uint8_t address[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

const sockaddr *IpV6Address::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IpV6Address::getAddr() {
    return (sockaddr *)&m_addr;
}

const socklen_t IpV6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &IpV6Address::insert(std::ostream &os) const {
    // [AD80:0000:0000:0000:ABAA:0000:00C2:0002]
    os << "[";
    const uint16_t *addr = (uint16_t *)m_addr.sin6_addr.s6_addr;
    for (int i = 0; i < 8; ++i) {
        os << std::hex << std::setw(4) << std::setfill('0') << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
        if (i == 7) {
            os << "]";
        } else {
            os << ":";
        }
    }
    os << "::" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

IpAddress::ptr IpV6Address::broadcastAddress(uint32_t prefixLen) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefixLen / 8] |= CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IpV6Address::ptr(new IpV6Address(baddr));
}

IpAddress::ptr IpV6Address::networkAddress(uint32_t prefixLen) {
    sockaddr_in6 naddr(m_addr);
    naddr.sin6_addr.s6_addr[prefixLen / 8] &= CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        naddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IpV6Address::ptr(new IpV6Address(naddr));
}

IpAddress::ptr IpV6Address::subnetMask(uint32_t prefixLen) {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefixLen / 8] = ~CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IpV6Address::ptr(new IpV6Address(subnet));
}

uint16_t IpV6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IpV6Address::setPort(uint16_t v) {
    m_addr.sin6_port = byteswapOnLittleEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(std::string &path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;
    if (!path.empty() && path[0] == '\0') {
        --m_length;
    }
    if (m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("PATH TOO LONG");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

const sockaddr *UnixAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *UnixAddress::getAddr() {
    return (sockaddr *)&m_addr;
}

void UnixAddress::setAddrLen(socklen_t len) {
    m_length = len;
}

const socklen_t UnixAddress::getAddrLen() const {
    return m_length;
}

std::string UnixAddress::getPath() const {
    std::stringstream ss;
    if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

std::ostream &UnixAddress::insert(std::ostream &os) const {
    if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

UnknowAddress::UnknowAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknowAddress::UnknowAddress(const sockaddr &addr) {
    m_addr = addr;
}

const sockaddr *UnknowAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *UnknowAddress::getAddr() {
    return (sockaddr *)&m_addr;
}

const socklen_t UnknowAddress::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &UnknowAddress::insert(std::ostream &os) const {
    return os << "UNKNOWADDRESS:\n\t"
              << "family:" << m_addr.sa_family;
}

std::ostream &operator<<(std::ostream &os, const Address &addr) {
    return addr.insert(os);
}

}  // namespace tigerkin
