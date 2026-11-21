
#include "flint.h"
#include "flint_common.h"
#include "flint_method_info.h"

ExceptionEntry::ExceptionEntry(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

static char parseReturnType(const char *desc) {
    while (*desc != ')')
        desc++;
    return desc[1];
}

MethodInfo::MethodInfo(ClassLoader *loader, MethodAccessFlag accessFlag, const char *name, const char *desc) :
accessFlag(accessFlag), retType(parseReturnType(desc)), loader(loader), name(name), desc(desc),
hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)), exceptions(NULL), codeAttr(NULL), code(NULL) {

}
