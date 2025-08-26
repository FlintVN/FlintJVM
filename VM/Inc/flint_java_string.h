
#ifndef __FLINT_JAVA_STRING_H
#define __FLINT_JAVA_STRING_H

#include "flint_java_object.h"
#include "flint_array_object.h"

class JString : public JObject {
public:
    JInt8Array *getValue(void) const;
    void setValue(JInt8Array &byteArray);
    char *getText(void) const;
    uint32_t getLength(void) const;
    uint8_t getCoder(void) const;
    void setCoder(uint8_t coder);
    int32_t compareTo(JString *another) const;
    int32_t compareTo(FlintConstUtf8 &utf8) const;
    uint32_t getUft8BuffSize(void);

    static bool isLatin1(const char *utf8);
    static uint8_t getUtf8DecodeSize(char c);
    static uint8_t getUtf8EncodeSize(uint16_t c);
    static uint32_t utf8Decode(const char *c);
    static uint8_t utf8Encode(uint16_t c, char *buff);
    static uint32_t utf8StrLen(const char *utf8);
protected:
    JString(void) = delete;
    JString(const JString &) = delete;
    void operator=(const JString &) = delete;
};

#endif /* __FLINT_JAVA_STRING_H */
