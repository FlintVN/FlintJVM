
#include "flint_thread.h"
#include "flint_fields_data.h"

FlintObject *FlintThread::getTask(void) const {
    return getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\xD1\x2A""task").object;
}
