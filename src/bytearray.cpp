/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "bytearray.h"

#include <string.h>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "macro.h"

namespace tigerkin {

ByteArray::Node::Node()
    : ptr(nullptr), size(0), nxt(nullptr) {
}

ByteArray::Node::Node(size_t s)
    : ptr(new char[s]), size(s), nxt(nullptr) {
}

ByteArray::Node::~Node() {
    if (ptr) {
        delete[] ptr;
    }
    ptr = nullptr;
    size = 0;
}

ByteArray::ByteArray(size_t baseSize)
    : m_endian(TIGERKIN_LITTLE_ENDIAN), m_baseSize(baseSize), m_size(0), m_capacity(0), m_offset(0), m_rootNode(nullptr), m_curNode(nullptr) {
}

ByteArray::~ByteArray() {
    clear();
}

void ByteArray::clear() {
    while (m_rootNode) {
        m_curNode = m_rootNode;
        m_rootNode = m_rootNode->nxt;
        delete m_rootNode;
    }
    m_rootNode = nullptr;
    m_endNode = nullptr;
    m_curNode = nullptr;
    m_capacity = 0;
    m_offset = 0;
    m_size = 0;
}

size_t ByteArray::getEnableReadSize() const {
    return m_size - m_offset;
}

void ByteArray::setOffset(size_t offset) {
    if (offset > m_capacity) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "SET OFFSET ERROR:\n\t"
                                                      << "size:" << m_size << "\n\t"
                                                      << "capacity:" << m_capacity << "\n\t"
                                                      << "offset:" << m_offset;
        throw std::out_of_range("SET OFFSET OUT OF RANGE");
    }
    if (offset > m_size) {
        m_size = offset;
    }
    size_t nodeCnt = ceil(offset / m_baseSize);
    m_curNode = m_rootNode;
    for (size_t i = 1; i < nodeCnt; ++i) {
        m_curNode = m_curNode->nxt;
    }
    m_offset = offset;
}

void ByteArray::addCapacity(size_t capacity) {
    addCapacityTo(capacity + m_capacity);
}

void ByteArray::addCapacityTo(size_t capacity) {
    if (!m_rootNode) {
        m_rootNode = new ByteArray::Node(m_baseSize);
        m_endNode = m_rootNode;
        m_curNode = m_rootNode;
        m_size = 0;
        m_offset = 0;
        m_capacity = m_baseSize;
    }
    if (capacity <= m_capacity) {
        return;
    }
    while (m_capacity < capacity) {
        m_endNode->nxt = new ByteArray::Node(m_baseSize);
        m_endNode = m_endNode->nxt;
        m_capacity += m_baseSize;
    }
}

void ByteArray::write(const void *buf, size_t size) {
    if (size == 0) {
        return;
    }
    addCapacityTo(m_size + size);
    size_t hasCpy = 0;
    size_t nodeOffset = m_offset % m_baseSize;
    if (nodeOffset == 0 && m_offset > 0 && m_curNode->nxt) {
        m_curNode = m_curNode->nxt;
    }
    size_t nodeRemain = m_curNode->size - nodeOffset;
    while (size > 0) {
        if (size > nodeRemain) {
            memcpy(m_curNode->ptr + nodeOffset, (const char *)buf + hasCpy, nodeRemain);
            m_size += nodeRemain;
            m_offset += nodeRemain;
            m_curNode = m_curNode->nxt;
            hasCpy += nodeRemain;
            size -= nodeRemain;
            nodeRemain = m_curNode->size;
            nodeOffset = 0;
        } else {
            memcpy(m_curNode->ptr + nodeOffset, (const char *)buf + hasCpy, size);
            m_size += size;
            m_offset += size;
            hasCpy += size;
            size -= size;
            nodeOffset += size;
            nodeRemain = m_curNode->size - nodeOffset;
        }
    }
    if(m_offset > m_size) {
        m_size = m_offset;
    }
}

void ByteArray::read(void *buf, size_t size) {
    if (size > getEnableReadSize()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "READ ERROR:\n\t"
                                                      << "size:" << m_size << "\n\t"
                                                      << "capacity:" << m_capacity << "\n\t"
                                                      << "read size:" << size << "\n\t"
                                                      << "enable read size:" << getEnableReadSize();
        throw std::out_of_range("READ OUT OF RANGE");
    }
    size_t nodeOffset = m_offset % m_baseSize;
    if (nodeOffset == 0 && m_offset > 0 && m_curNode->nxt) {
        m_curNode = m_curNode->nxt;
    }
    size_t nodeRemain = m_curNode->size - nodeOffset;
    size_t hasCpy = 0;
    while (size > 0) {
        if (size > nodeRemain) {
            memcpy((char *)buf + hasCpy, m_curNode->ptr + nodeOffset, nodeRemain);
            m_offset += nodeRemain;
            m_curNode = m_curNode->nxt;
            hasCpy += nodeRemain;
            size -= nodeRemain;
            nodeOffset = 0;
            nodeRemain = m_curNode->size;
        } else {
            memcpy((char *)buf + hasCpy, m_curNode->ptr + nodeOffset, size);
            m_offset += size;
            hasCpy += size;
            size -= size;
            nodeOffset += size;
            nodeRemain = m_curNode->size - nodeOffset;
        }   
    }
}

void ByteArray::read(void *buf, size_t size, size_t offset) const {
    if (size > getEnableReadSize()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "READ ERROR:\n\t"
                                                      << "size:" << m_size << "\n\t"
                                                      << "capacity:" << m_capacity << "\n\t"
                                                      << "read size:" << size << "\n\t"
                                                      << "enable read size:" << getEnableReadSize();
        throw std::out_of_range("READ OUT OF RANGE");
    }
    ByteArray::Node *tmp = m_curNode;
    size_t nodeOffset = offset % m_baseSize;
    if (nodeOffset == 0 && offset > 0 && tmp->nxt) {
        tmp = tmp->nxt;
    }
    size_t nodeRemain = tmp->size - nodeOffset;
    size_t hasCpy = 0;
    while (size > 0) {
        if (size > nodeRemain) {
            memcpy((char *)buf + hasCpy, tmp->ptr + nodeOffset, nodeRemain);
            tmp = tmp->nxt;
            hasCpy += nodeRemain;
            size -= nodeRemain;
            nodeOffset = 0;
            nodeRemain = tmp->size;
        } else {
            memcpy((char *)buf + hasCpy, tmp->ptr + nodeOffset, size);
            nodeOffset += size;
            nodeRemain = tmp->size - nodeOffset;
            hasCpy += size;
            size -= size;
        }
    }
}

void ByteArray::writeUint8(const uint8_t &value) {
    write(&value, 1);
}

void ByteArray::writeUint16(uint16_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 2);
}

void ByteArray::writeUint32(uint32_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 4);
}

void ByteArray::writeUint64(uint64_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 8);
}

void ByteArray::writeInt8(const int8_t &value) {
    write(&value, sizeof(value));
}

void ByteArray::writeInt16(int16_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 2);
}

void ByteArray::writeInt32(int32_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 4);
}

void ByteArray::writeInt64(int64_t value) {
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, 8);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeUint32(v);
}

void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeUint64(v);
}

void ByteArray::writeString16(const std::string &str) {
    writeUint16(str.size());
    write(str.c_str(), str.size());
}

void ByteArray::writeString32(const std::string &str) {
    writeUint32(str.size());
    write(str.c_str(), str.size());
}

void ByteArray::writeString64(const std::string &str) {
    writeUint64(str.size());
    write(str.c_str(), str.size());
}

void ByteArray::writeStringVint(const std::string &str) {
    writeUint64(str.size());
    write(str.c_str(), str.size());
}

void ByteArray::writeString(const std::string &str) {
    write(str.c_str(), str.size());
}

uint8_t ByteArray::readUint8() {
    uint8_t v = 0;
    read(&v, 1);
    return v;
}

uint16_t ByteArray::readUint16() {
    uint16_t v = 0;
    read(&v, 2);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

uint32_t ByteArray::readUint32() {
    uint32_t v = 0;
    read(&v, 4);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

uint64_t ByteArray::readUint64() {
    uint64_t v = 0;
    read(&v, 8);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int8_t ByteArray::readInt8() {
    int8_t v = 0;
    read(&v, 1);
    return v;
}

int16_t ByteArray::readInt16() {
    int16_t v = 0;
    read(&v, 2);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int32_t ByteArray::readInt32() {
    int32_t v = 0;
    read(&v, 4);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int64_t ByteArray::readInt64() {
    int64_t v = 0;
    read(&v, 8);
    if (m_endian != TIGERKIN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

float ByteArray::readFloat() {
    uint32_t v = readUint32();
    float value;
    memcpy(&value, &v, 4);
    return value;
}

double ByteArray::readDouble() {
    uint64_t v = readUint64();
    double value;
    memcpy(&value, &v, 8);
    return value;
}

std::string ByteArray::readString16() {
    uint16_t len = readUint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readString32() {
    uint32_t len = readUint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readString64() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::toString() const {
    std::string s;
    s.resize(getEnableReadSize());
    if (s.empty()) {
        return s;
    }
    read(&s[0], s.size(), m_offset);
    return s;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

bool ByteArray::writeToFile(const std::string &name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "WRITE TO FILE ERROR:\n\t"
                                                      << "file name:" << name << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    size_t len = getEnableReadSize();
    ByteArray::Node *tmp = m_curNode;
    size_t nodeOffset = m_offset % m_baseSize;
    if (nodeOffset == 0 && m_offset > 0 && tmp->nxt) {
        tmp = tmp->nxt;
    }
    size_t nodeRemain = m_baseSize - nodeOffset;
    while (len > 0) {
        if (len > nodeRemain) {
            ofs.write(tmp->ptr + nodeOffset, nodeRemain);
            tmp = tmp->nxt;
            len -= nodeRemain;
            nodeRemain = tmp->size;
            nodeOffset = 0;
        } else {
            ofs.write(tmp->ptr + nodeOffset, len);
            len -= len;
            nodeRemain = tmp->size;
            nodeOffset = 0;
        }
    }
    ofs.close();
    return true;
}

bool ByteArray::readFromFile(const std::string &name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "READ FROM FILE ERROR:\n\t"
                                                      << "file name:" << name << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    std::shared_ptr<char> buff(new char[m_baseSize], [](char *ptr) { delete[] ptr; });
    while (!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    ifs.close();
    return true;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len) const {
    return getReadBuffers(buffers, len, m_offset);
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len, uint64_t offset) const {
    if (offset + len > m_size || len == 0) {
        return 0;
    }
    Node *tmpCur = m_rootNode;
    size_t nodeCnt = ceil(offset / m_baseSize);
    for (size_t i = 1; i < nodeCnt; ++i) {
        tmpCur = tmpCur->nxt;
    }
    struct iovec iov;
    int rst = 0;
    size_t nodePos = offset % m_baseSize;
    size_t nodeRemain = tmpCur->size - nodePos;
    while (len > 0) {
        if (len > nodeRemain) {
            iov.iov_base = tmpCur->ptr + nodePos;
            iov.iov_len = nodeRemain;
            tmpCur = tmpCur->nxt;
            len -= nodeRemain;
            rst += nodeRemain;
            nodePos = 0;
            nodeRemain = tmpCur->size;
        } else {
            iov.iov_base = tmpCur->ptr + nodePos;
            iov.iov_len = len;
            len -= len;
            rst += len;
        }
        buffers.push_back(iov);
    }
    return rst;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec> &buffers, uint64_t len) {
    if (len == 0) {
        return 0;
    }
    addCapacityTo(m_offset + len);
    Node *tmpCur = m_curNode;
    struct iovec iov;
    int rst = 0;
    size_t nodePos = m_offset % m_baseSize;
    size_t nodeRemain = m_curNode->size - nodePos;
    while (len > 0) {
        if (len > nodeRemain) {
            iov.iov_base = tmpCur->ptr + nodePos;
            iov.iov_len = nodeRemain;
            tmpCur = tmpCur->nxt;
            len -= nodeRemain;
            rst += nodeRemain;
            nodePos = 0;
            nodeRemain = tmpCur->size;
        } else {
            iov.iov_base = tmpCur->ptr + nodePos;
            iov.iov_len = len;
            len -= len;
            rst += len;
        }
        buffers.push_back(iov);
    }
    return rst;
}

}  // namespace tigerkin