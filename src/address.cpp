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
#include <sstream>

#include "endian.h"
#include "macro.h"

namespace tigerkin {

template <class T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
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

bool IpAddress::Lookup(std::vector<IpAddress::ptr> &rsts, const std::string &hosts, const std::string &service, int flags, int family, int type, int protocol) {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = flags;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;

    struct addrinfo *rst = NULL;
    struct addrinfo *nxt = NULL;

    int ret = getaddrinfo(hosts.c_str(), service.c_str(), &hints, &rst);
    if (ret != 0) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET ADDR INFO ERROR:\n\t"
                                                      << "ret:" << ret << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    for (nxt = rst; nxt != NULL; nxt = nxt->ai_next) {
        if (nxt->ai_family == family) {
            switch (nxt->ai_family) {
                case AF_INET:
                    rsts.push_back(IpV4Address::ptr(new IpV4Address(*(const sockaddr_in *)nxt->ai_addr)));
                    break;
                case AF_INET6:
                    rsts.push_back(IpV6Address::ptr(new IpV6Address(*(const sockaddr_in6 *)nxt->ai_addr)));
                default:
                    break;
            }
        }
    }
    freeaddrinfo(rst);
    return true;
}

bool IpAddress::GetInterfaceAddress(std::multimap<std::string, std::pair<IpAddress::ptr, uint32_t>> &rsts, int family) {
    struct ifaddrs *nxt, *rst;
    if (getifaddrs(&rst) != 0) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET IF ADDRS ERROR:\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    try {
        for (nxt = rst; nxt != NULL; nxt = nxt->ifa_next) {
            if (!nxt->ifa_addr || nxt->ifa_addr->sa_family != family) {
                continue;
            }
            if (nxt->ifa_addr->sa_family == AF_INET) {
                uint32_t netmask = ((sockaddr_in *)nxt->ifa_netmask)->sin_addr.s_addr;
                uint32_t prefixLen = 0;
                while (netmask) {
                    ++prefixLen;
                    netmask &= (netmask - 1);
                }
                rsts.insert(std::make_pair(nxt->ifa_name,
                                           std::make_pair(IpV4Address::ptr(new IpV4Address(*(const sockaddr_in *)nxt->ifa_addr)), prefixLen)));
            } else if (nxt->ifa_addr->sa_family == AF_INET6) {
                in6_addr &addr = ((sockaddr_in6 *)nxt->ifa_netmask)->sin6_addr;
                uint32_t prefixLen = 0;
                for (int i = 0; i < 16; ++i) {
                    uint8_t netmask = addr.s6_addr[i];
                    while (netmask) {
                        ++prefixLen;
                        netmask &= (netmask - 1);
                    }
                }
                rsts.insert(std::make_pair(nxt->ifa_name,
                                           std::make_pair(IpV6Address::ptr(new IpV6Address(*(const sockaddr_in6 *)nxt->ifa_addr, prefixLen)), prefixLen)));
            }
        }
        return true;
    } catch (const std::exception &e) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET INTERFACE ADDRESS ERROR:\n\t"
                                                      << "error:" << e.what();
        return false;
    }
}

IpV4Address::IpV4Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
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

Address::ptr IpV4Address::broadcastAddress(uint32_t prefixLen) {
    if (prefixLen > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(baddr));
}

Address::ptr IpV4Address::networkAddress(uint32_t prefixLen) {
    if (prefixLen > 32) {
        return nullptr;
    }
    sockaddr_in naddr(m_addr);
    naddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(naddr));
}

Address::ptr IpV4Address::subnetMask(uint32_t prefixLen) {
    if (prefixLen > 32) {
        return nullptr;
    }
    sockaddr_in saddr(m_addr);
    saddr.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefixLen));
    return IpV4Address::ptr(new IpV4Address(saddr));
}

uint16_t IpV4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IpV4Address::setPort(uint16_t v) {
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

IpV6Address::IpV6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_prefixLen = 0;
}

IpV6Address::IpV6Address(const sockaddr_in6 &addr, const uint32_t prefixLen) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr = addr;
    m_prefixLen = prefixLen;
}

IpV6Address::IpV6Address(const uint8_t address[16], uint16_t port, const uint32_t prefixLen) {
    m_prefixLen = prefixLen;
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

IpV6Address::IpV6Address(const char *address, uint16_t port, const uint32_t prefixLen) {
    m_prefixLen = prefixLen;
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    int rt = inet_pton(AF_INET6, address, &m_addr.sin6_addr);
    if (rt <= 0) {
        std::stringstream ss;
        ss << "IPV6 ADDRESS INIT ERROR:\n\t"
           << "address:" << address << "\n\t"
           << "port:" << port << "\n\t"
           << "errno:" << errno << "\n\t"
           << "strerror" << strerror(errno);
        throw std::logic_error(ss.str());
    }
}

const sockaddr *IpV6Address::getAddr() const {
    return (sockaddr *)&m_addr;
}

const socklen_t IpV6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &IpV6Address::insert(std::ostream &os) const {
    // [AD80:0000:0000:0000:ABAA:0000:00C2:0002]
    const uint16_t *addr = (uint16_t *)m_addr.sin6_addr.s6_addr;
    os << "[";
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

Address::ptr IpV6Address::broadcastAddress(uint32_t prefixLen) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefixLen / 8] |= CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IpV6Address::ptr(new IpV6Address(baddr));
}

Address::ptr IpV6Address::networkAddress(uint32_t prefixLen) {
    sockaddr_in6 naddr(m_addr);
    naddr.sin6_addr.s6_addr[prefixLen / 8] |= CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        naddr.sin6_addr.s6_addr[i] = 0x11;
    }
    return IpV6Address::ptr(new IpV6Address(naddr));
}

Address::ptr IpV6Address::subnetMask(uint32_t prefixLen) {
    sockaddr_in6 naddr(m_addr);
    naddr.sin6_addr.s6_addr[prefixLen / 8] |= CreateMask<uint8_t>(prefixLen % 8);
    for (int i = prefixLen / 8 + 1; i < 16; ++i) {
        naddr.sin6_addr.s6_addr[i] = 0x11;
    }
    return IpV6Address::ptr(new IpV6Address(naddr));
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

UnknowAddress::UnknowAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
}

const sockaddr *UnknowAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

const socklen_t UnknowAddress::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &UnknowAddress::insert(std::ostream &os) const {
    return os << "UNKNOWADDRESS:\n\t"
              << "family:" << m_addr.sa_family;
}

}  // namespace tigerkin
