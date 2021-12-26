/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_ADDRESS_H__
#define __TIGERKIN_ADDRESS_H__

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tigerkin {

class Address {
   public:
    typedef std::shared_ptr<Address> ptr;

    ~Address(){};

    static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);
    static bool Lookup(std::vector<Address::ptr> &result, const ::std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
    static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

    int getFamily() const;
    virtual const sockaddr *getAddr() const = 0;
    virtual sockaddr *getAddr() = 0;
    virtual const socklen_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString();

    bool operator<(const Address &rhs) const;
    bool operator==(const Address &rhs) const;
    bool operator!=(const Address &ths) const;
};

class IpAddress : public Address {
   public:
    typedef std::shared_ptr<IpAddress> ptr;

    static IpAddress::ptr Create(const char *address, uint16_t port = 0);

    static IpAddress::ptr LookupAnyIpAddress(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
    static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<IpAddress::ptr, uint32_t>> &rsts, int family = AF_INET);
    static bool GetInterfaceAddresses(std::vector<std::pair<IpAddress::ptr, uint32_t>> &result, const std::string &iface, int family = AF_INET);

    virtual IpAddress::ptr broadcastAddress(uint32_t prefixLen) = 0;
    virtual IpAddress::ptr networkAddress(uint32_t prefixLen) = 0;
    virtual IpAddress::ptr subnetMask(uint32_t prefixLen) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
};

class IpV4Address : public IpAddress {
   public:
    typedef std::shared_ptr<IpV4Address> ptr;

    IpV4Address(const sockaddr_in &addr);
    IpV4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    IpV4Address(const char *address, uint16_t port = 0);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    const socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IpAddress::ptr broadcastAddress(uint32_t prefixLen) override;
    IpAddress::ptr networkAddress(uint32_t prefixLen) override;
    IpAddress::ptr subnetMask(uint32_t prefixLen) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;

   private:
    sockaddr_in m_addr;
};

class IpV6Address : public IpAddress {
   public:
    typedef std::shared_ptr<IpV6Address> ptr;

    static IpV6Address::ptr Create(const char *address, uint16_t port = 0);

    IpV6Address();
    IpV6Address(const sockaddr_in6 &addr);
    IpV6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    const socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IpAddress::ptr broadcastAddress(uint32_t prefixLen) override;
    IpAddress::ptr networkAddress(uint32_t prefixLen) override;
    IpAddress::ptr subnetMask(uint32_t prefixLen) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;

   private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
   public:
    typedef std::shared_ptr<UnixAddress> ptr;

    UnixAddress();
    UnixAddress(std::string &path);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    void setAddrLen(socklen_t len);
    const socklen_t getAddrLen() const override;

    std::string getPath() const;
    std::ostream &insert(std::ostream &os) const override;

   private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknowAddress : public Address {
   public:
    typedef std::shared_ptr<UnknowAddress> ptr;

    UnknowAddress(int family);
    UnknowAddress(const sockaddr &addr);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    const socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

   private:
    sockaddr m_addr;
};

std::ostream &operator<<(std::ostream &os, const Address &addr);

}  // namespace tigerkin

#endif  // !__TIGERKIN_ADDRESS_H__