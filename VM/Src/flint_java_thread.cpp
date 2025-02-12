
#include "flint_java_thread.h"
#include "flint_fields_data.h"

FlintJavaObject *FlintJavaThread::getTask(void) const {
    return getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\xD1\x2A""task").object;
}
