/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/14
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/bytearray.h"

#include "../src/macro.h"

void test_write_read_uint() {
    tigerkin::ByteArray::ptr ba(new tigerkin::ByteArray(4));
    ba->writeUint8(1);
    ba->writeUint16(2);
    ba->writeUint16(2);
    ba->writeUint32(3);
    ba->writeUint64(4);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "size:" << ba->getSize() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\n\t"
                                                << "capacity:" << ba->getCapacity() << "\n\t"
                                                << "enable read size:" << ba->getEnableReadSize();

    ba->setOffset(0);
    ba->writeToFile("bytearray.txt");
    tigerkin::ByteArray::ptr ba1(new tigerkin::ByteArray(2));
    ba1->readFromFile("bytearray.txt");
    ba1->setOffset(0);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "ba to string:" << ba->toHexString() << "\n\t"
                                                << "ba1 to string:" << ba1->toHexString();

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int8:" << ba->readUint8() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int16:" << ba->readUint16() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int16:" << ba->readUint16() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int32:" << ba->readUint32() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int64:" << ba->readUint64() + 0;
}

void test_write_read_int() {
    tigerkin::ByteArray::ptr ba(new tigerkin::ByteArray(4));
    ba->writeInt8(1);
    ba->writeInt16(2);
    ba->writeInt16(2);
    ba->writeInt32(3);
    ba->writeInt64(4);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "size:" << ba->getSize() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\n\t"
                                                << "capacity:" << ba->getCapacity() << "\n\t"
                                                << "enable read size:" << ba->getEnableReadSize();

    ba->setOffset(0);
    ba->writeToFile("bytearray.txt");
    tigerkin::ByteArray::ptr ba1(new tigerkin::ByteArray(2));
    ba1->readFromFile("bytearray.txt");
    ba1->setOffset(0);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "ba to string:" << ba->toHexString() << "\n\t"
                                                << "ba1 to string:" << ba1->toHexString();

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int8:" << ba->readInt8() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int16:" << ba->readInt16() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int16:" << ba->readInt16() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int32:" << ba->readInt32() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread int64:" << ba->readInt64() + 0;
}

void test_write_read_float() {
    tigerkin::ByteArray::ptr ba(new tigerkin::ByteArray(4));
    ba->writeFloat(3.67);
    ba->writeFloat(4.6);
    ba->writeFloat(7.6);
    ba->writeDouble(6.7765);
    ba->writeDouble(6.77622);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "size:" << ba->getSize() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\n\t"
                                                << "capacity:" << ba->getCapacity() << "\n\t"
                                                << "enable read size:" << ba->getEnableReadSize();

    ba->setOffset(0);
    ba->writeToFile("bytearray.txt");
    tigerkin::ByteArray::ptr ba1(new tigerkin::ByteArray(2));
    ba1->readFromFile("bytearray.txt");
    ba1->setOffset(0);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "ba to string:" << ba->toHexString() << "\n\t"
                                                << "ba1 to string:" << ba1->toHexString();

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread float:" << ba->readFloat() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread float:" << ba->readFloat() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread float:" << ba->readFloat() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread double:" << ba->readDouble() + 0 << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread double:" << ba->readDouble() + 0;
}

void test_write_read_str() {
    tigerkin::ByteArray::ptr ba(new tigerkin::ByteArray(4));
    ba->writeString16("str16");
    ba->writeString32("str32");
    ba->writeString64("str64");
    ba->writeStringVint("stringVint");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "size:" << ba->getSize() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\n\t"
                                                << "capacity:" << ba->getCapacity() << "\n\t"
                                                << "enable read size:" << ba->getEnableReadSize();

    ba->setOffset(0);
    ba->writeToFile("bytearray.txt");
    tigerkin::ByteArray::ptr ba1(new tigerkin::ByteArray(2));
    ba1->readFromFile("bytearray.txt");
    ba1->setOffset(0);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "ba to string:" << ba->toHexString() << "\n\t"
                                                << "ba1 to string:" << ba1->toHexString();

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread string16:" << ba->readString16() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread string32:" << ba->readString32() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread string64:" << ba->readString64() << "\n\t"
                                                << "offset:" << ba->getOffset() << "\tread stringVint:" << ba->readStringVint();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "TEST BYTEARRAY START";
    test_write_read_uint();
    test_write_read_int();
    test_write_read_float();
    test_write_read_str();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "TEST BYTEARRAY END";
    return 0;
}