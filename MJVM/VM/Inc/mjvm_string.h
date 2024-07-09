
#ifndef __MJVM_STRING_H
#define __MJVM_STRING_H

#include "mjvm_object.h"

class MjvmString : public MjvmObject {
public:
    const char *getText(void) const;
    uint32_t getLength(void) const;
    uint8_t getCoder(void) const;
    bool equals(const char *text, uint32_t length) const;
    bool equals(const MjvmConstUtf8 &utf8) const;
    bool equals(MjvmString &utf8) const;

    static bool isLatin1(const char *utf8);
    static uint8_t getUtf8DecodeSize(char c);
    static uint8_t getUtf8EncodeSize(uint16_t c);
    static uint32_t utf8Decode(const char *c);
    static uint8_t utf8Encode(uint16_t c, char *buff);
    static uint32_t utf8StrLen(const char *utf8);
protected:
    MjvmString(void) = delete;
    MjvmString(const MjvmString &) = delete;
    void operator=(const MjvmString &) = delete;
};

class MjvmConstString {
private:
    MjvmConstString *next;
public:
    MjvmString &mjvmString;
private:
    MjvmConstString(MjvmString &mjvmString);
    MjvmConstString(const MjvmConstString &) = delete;
    void operator=(const MjvmConstString &) = delete;

    friend class MjvmExecution;
};

#endif /* __MJVM_STRING_H */
