
#ifndef __FLINT_JAVA_STRING_H
#define __FLINT_JAVA_STRING_H

#include <cstdarg>
#include "flint_java_object.h"
#include "flint_array_object.h"

class JString : public JObject {
public:
    bool setUtf8(FExec *ctx, const char *utf8);
    bool setAscii(FExec *ctx, const char *format, va_list args);

    JByteArray *getValue(void) const;
    const char *getAscii(void) const;
    void setValue(JByteArray *value);

    uint32_t getLength(void) const;

    uint8_t getCoder(void) const;
    void setCoder(uint8_t coder);

    uint32_t getHashCode(void);

    uint16_t getCharAt(uint32_t index) const;

    int32_t compareTo(JString *other) const;
    int32_t compareTo(const char *utf8, uint16_t length) const;
private:
    uint32_t getHash(void) const;
    void setHash(uint32_t hash);
    bool getHashIsZero(void) const;
    void setHashIsZero(bool value);

    JString(void) = delete;
    JString(const JString &) = delete;
    void operator=(const JString &) = delete;
};

#endif /* __FLINT_JAVA_STRING_H */
