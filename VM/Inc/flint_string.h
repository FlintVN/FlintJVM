
#ifndef __FLINT_STRING_H
#define __FLINT_STRING_H

#include "flint_object.h"

class FlintString : public FlintObject {
public:
    FlintObject *getValue(void) const;
    void setValue(FlintObject &byteArray);
    const char *getText(void) const;
    uint32_t getLength(void) const;
    uint8_t getCoder(void) const;
    void setCoder(uint8_t coder);
    bool equals(const char *text, uint32_t length) const;
    bool equals(const FlintConstUtf8 &utf8) const;
    bool equals(FlintString &utf8) const;

    static bool isLatin1(const char *utf8);
    static uint8_t getUtf8DecodeSize(char c);
    static uint8_t getUtf8EncodeSize(uint16_t c);
    static uint32_t utf8Decode(const char *c);
    static uint8_t utf8Encode(uint16_t c, char *buff);
    static uint32_t utf8StrLen(const char *utf8);
    static uint32_t getUft8BuffSize(FlintString &str);
protected:
    FlintString(void) = delete;
    FlintString(const FlintString &) = delete;
    void operator=(const FlintString &) = delete;
};

class FlintConstString {
private:
    FlintConstString *next;
public:
    FlintString &flintString;
private:
    FlintConstString(FlintString &flintString);
    FlintConstString(const FlintConstString &) = delete;
    void operator=(const FlintConstString &) = delete;

    friend class Flint;
};

#endif /* __FLINT_STRING_H */
