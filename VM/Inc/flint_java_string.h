
#ifndef __FLINT_JAVA_STRING_H
#define __FLINT_JAVA_STRING_H

#include "flint_java_object.h"
#include "flint_array_object.h"

class FlintJavaString : public FlintJavaObject {
public:
    FlintInt8Array *getValue(void) const;
    void setValue(FlintInt8Array &byteArray);
    const char *getText(void) const;
    uint32_t getLength(void) const;
    uint8_t getCoder(void) const;
    void setCoder(uint8_t coder);
    int32_t compareTo(FlintJavaString &another) const;
    int32_t compareTo(const FlintConstUtf8 &utf8) const;
    uint32_t getUft8BuffSize(void);

    static bool isLatin1(const char *utf8);
    static uint8_t getUtf8DecodeSize(char c);
    static uint8_t getUtf8EncodeSize(uint16_t c);
    static uint32_t utf8Decode(const char *c);
    static uint8_t utf8Encode(uint16_t c, char *buff);
    static uint32_t utf8StrLen(const char *utf8);
protected:
    FlintJavaString(void) = delete;
    FlintJavaString(const FlintJavaString &) = delete;
    void operator=(const FlintJavaString &) = delete;
};

#endif /* __FLINT_JAVA_STRING_H */
