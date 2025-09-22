
#include "flint_java_thread.h"
#include "flint_fields_data.h"

JObject *JThread::getTask(void) const {
    return getFieldObjByIndex(0)->value;
}
