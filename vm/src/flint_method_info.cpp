
#include "flint.h"
#include "flint_common.h"
#include "flint_method_info.h"

ExceptionEntry::ExceptionEntry(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

MethodInfo::MethodInfo(ClassLoader *loader, MethodAccessFlag accessFlag, const char *name, const char *desc) :
accessFlag(accessFlag), loader(loader), name(name), desc(desc),
hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)), exceptions(NULL), retType(NULL), code(NULL) {

}
