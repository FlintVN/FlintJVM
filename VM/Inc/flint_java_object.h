
#ifndef __FLINT_JAVA_OBJECT_H
#define __FLINT_JAVA_OBJECT_H

#include "flint_std.h"
#include "flint_list_node.h"
#include "flint_fields_data.h"

class JObject : public ListNode<JObject> {
protected:
    const uint32_t size : 30;
private:
    uint32_t prot : 2;
public:
    class JClass * const type;  /* NULL if JClass */
private:
    uint32_t monitorCount;
    uint32_t ownId;
protected:
    uint8_t data[];
public:
    const char *getTypeName(void) const;

    FieldsData *getFields(void) const;
    bool initFields(class FExec *ctx, class ClassLoader *loader);

    void clearData(void);

    bool isArray(void) const;

    void clearProtected(void);
    void setProtected(void);
    uint8_t getProtected(void) const;
protected:
    JObject(uint32_t size, class JClass *type);
    JObject(const JObject &) = delete;
    void operator=(const JObject &) = delete;

    ~JObject(void);

    friend class Flint;
    friend class FExec;
};

#endif /* __FLINT_JAVA_OBJECT_H */
