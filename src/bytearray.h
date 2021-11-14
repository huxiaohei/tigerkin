/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_BYTEARRAY_H__
#define __TIGERKIN_BYTEARRAY_H__

#include <memory>
#include <string>

#include "endian.h"
#include "hook.h"
#include "stdint.h"

namespace tigerkin {

class ByteArray {
   public:
    std::shared_ptr<ByteArray> ptr;

    ByteArray(size_t baseSize);
    ~ByteArray();

    bool isLittleEndian() const { return m_endian == TIGERKIN_LITTLE_ENDIAN; }
    void setEndian(uint8_t endian) { m_endian = endian; }
    const size_t getBaseSize() const { return m_baseSize; }
    const size_t getSize() const { return m_size; }
    const size_t getCapacity() const { return m_capacity; }
    void clear();
    size_t getEnableReadSize() const;
    void setOffset(size_t offset);
    const size_t getOffset() const { return m_offset; }
    std::string toString() const;
    std::string toHexString() const;

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    void writeUint8(const uint8_t &value);
    void writeUint16(uint16_t value);
    void writeUint32(uint32_t value);
    void writeUint64(uint64_t value);
    void writeInt8(const int8_t &value);
    void writeInt16(int16_t value);
    void writeInt32(int32_t value);
    void writeInt64(int64_t value);

    void writeFloat(float value);
    void writeDouble(double value);

    void writeString16(const std::string &str);
    void writeString32(const std::string &str);
    void writeString64(const std::string &str);
    void writeStringVint(const std::string &str);
    void writeString(const std::string &str);

    uint8_t readUint8();
    uint16_t readUint16();
    uint32_t readUint32();
    uint64_t readUint64();
    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();

    float readFloat();
    double readDouble();

    std::string readString16();
    std::string readString32();
    std::string readString64();
    std::string readStringVint();

   protected:
    struct Node {
        Node();
        Node(size_t s);
        ~Node();

        char *ptr;
        size_t size;
        Node *nxt;
    };

    void addCapacity(size_t capacity);
    void addCapacityTo(size_t capacity);
    void write(const void *buf, size_t size);
    void read(void *buf, size_t size);
    void read(void* buf, size_t size, size_t m_offset) const;

   private:
    int8_t m_endian;
    size_t m_baseSize;
    size_t m_curNodePos;
    size_t m_size;
    size_t m_capacity;
    size_t m_offset;
    ByteArray::Node *m_rootNode;
    ByteArray::Node *m_curNode;
    ByteArray::Node *m_endNode;
};

}  // namespace tigerkin

#endif