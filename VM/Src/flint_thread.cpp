
#include "flint_thread.h"
#include "flint_fields_data.h"

static const uint32_t threadTaskFieldName[] = {
    (uint32_t)"\x04\x00\xB3\x01""task",                 /* field name */
    (uint32_t)"\x14\x00\x60\x07""Ljava/lang/Runnable;"  /* field type */
};

FlintObject *FlintThread::getTask(void) const {
    return ((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)threadTaskFieldName).object;
}
