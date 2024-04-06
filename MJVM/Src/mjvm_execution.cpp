
#include <string.h>
#include "mjvm_opcodes.h"
#include "mjvm_execution.h"

#define FLOAT_NAN                   0x7FC00000
#define DOUBLE_NAN                  0x7FF8000000000000

#define ARRAY_TO_INT16(array)       (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)       (int32_t)(((array)[0] << 24) | ((array)[0] << 16) | ((array)[0] << 8) | (array)[1])

Execution::Execution(void) : stackLength(DEFAULT_STACK_SIZE / sizeof(int32_t)) {
    sp = -1;
    stack = (int32_t *)MjvmHeap::malloc(DEFAULT_STACK_SIZE);
    stackType = (uint8_t *)MjvmHeap::malloc(DEFAULT_STACK_SIZE / sizeof(int32_t) / 8);
    staticClassDataHead = 0;
}

Execution::Execution(uint32_t size) : stackLength(size / sizeof(int32_t)) {
    sp = -1;
    stack = (int32_t *)MjvmHeap::malloc(size);
    stackType = (uint8_t *)MjvmHeap::malloc(size / sizeof(int32_t) / 8);
    staticClassDataHead = 0;
}

MjvmObject *Execution::newObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) const {
    return MjvmHeap::newObject((uint32_t)this, size, type, dimensions);
}

const ClassLoader &Execution::load(const char *className) {
    return load(className, strlen(className));
}

const ClassLoader &Execution::load(const char *className, uint16_t length) {
    for(ClassDataNode *node = staticClassDataHead; node != 0; node = node->next) {
        const ConstUtf8 &name = node->classLoader.getThisClass();
        if(name.length == length && strncmp(name.getText(), className, length))
            return node->classLoader;
    }
    ClassDataNode *newNode = (ClassDataNode *)MjvmHeap::malloc(sizeof(ClassDataNode));
    new (newNode)ClassDataNode(ClassLoader::load(className), 0);
    newNode->next = staticClassDataHead;
    staticClassDataHead = newNode;
    return newNode->classLoader;
}

const ClassLoader &Execution::load(const ConstUtf8 &className) {
    for(ClassDataNode *node = staticClassDataHead; node != 0; node = node->next) {
        const ConstUtf8 &name = node->classLoader.getThisClass();
        if(node->classLoader.getThisClass() == className)
            return node->classLoader;
    }
    ClassDataNode *newNode = (ClassDataNode *)MjvmHeap::malloc(sizeof(ClassDataNode));
    new (newNode)ClassDataNode(ClassLoader::load(className), 0);
    newNode->next = staticClassDataHead;
    staticClassDataHead = newNode;
    return newNode->classLoader;
}

const FieldsData &Execution::getStaticFields(const ConstUtf8 &className) const {
    for(ClassDataNode *node = staticClassDataHead; node != 0; node = node->next) {
        if(className == node->classLoader.getThisClass())
            return *node->filedsData;
    }
    return *(const FieldsData *)0;
}

const FieldsData &Execution::getStaticFields(const ClassLoader &classLoader) const {
    for(ClassDataNode *node = staticClassDataHead; node != 0; node = node->next) {
        if(&classLoader == &node->classLoader)
            return *node->filedsData;
    }
    return *(const FieldsData *)0;
}

void Execution::initStaticField(const ClassLoader &classLoader) {
    ClassDataNode *newNode = (ClassDataNode *)MjvmHeap::malloc(sizeof(ClassDataNode));
    FieldsData *fieldsData = (FieldsData *)MjvmHeap::malloc(sizeof(FieldsData));
    new (fieldsData)FieldsData(*this, classLoader, true);
    new (newNode)ClassDataNode(classLoader, fieldsData);
    newNode->next = staticClassDataHead;
    staticClassDataHead = newNode;
}

StackType Execution::getStackType(uint32_t index) {
    return (stackType[index / 8] & (1 << (index % 8))) ? STACK_TYPE_OBJECT : STACK_TYPE_NON_OBJECT;
}

StackValue Execution::getStackValue(uint32_t index) {
    StackValue ret = {
        .type = (stackType[index / 8] & (1 << (index % 8))) ? STACK_TYPE_OBJECT : STACK_TYPE_NON_OBJECT,
        .value = stack[index],
    };
    return ret;
}

void Execution::setStackValue(uint32_t index, const StackValue &value) {
    stack[index] = value.value;
    if(value.type == STACK_TYPE_OBJECT)
        stackType[index / 8] |= (1 << (index % 8));
    else
        stackType[index / 8] &= ~(1 << (index % 8));
}

void Execution::stackPush(const StackValue &value) {
    sp++;
    stack[sp] = value.value;
    if(value.type == STACK_TYPE_OBJECT)
        stackType[sp / 8] |= (1 << (sp % 8));
    else
        stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushInt32(int32_t value) {
    sp++;
    stack[sp] = value;
    stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushInt64(int64_t value) {
    if((sp + 2) < stackLength) {
        sp++;
        stack[sp] = ((uint32_t *)&value)[0];
        stackType[sp / 8] &= ~(1 << (sp % 8));
        sp++;
        stack[sp] = ((uint32_t *)&value)[1];
        stackType[sp / 8] &= ~(1 << (sp % 8));
    }
    else
        throw "stack overflow";
}

void Execution::stackPushFloat(float value) {
    sp++;
    stack[sp] = *(uint32_t *)&value;
    stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushDouble(double value) {
    if((sp + 2) < stackLength) {
        sp++;
        stack[sp] = ((uint32_t *)&value)[0];
        stackType[sp / 8] &= ~(1 << (sp % 8));
        sp++;
        stack[sp] = ((uint32_t *)&value)[1];
        stackType[sp / 8] &= ~(1 << (sp % 8));
    }
}

void Execution::stackPushObject(MjvmObject *obj) {
    sp++;
    stack[sp] = (int32_t)obj;
    stackType[sp / 8] |= (1 << (sp % 8));
}

int32_t Execution::stackPopInt32(void) {
    return stack[sp--];
}

int64_t Execution::stackPopInt64(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[0] = stack[sp + 1];
    ((uint32_t *)&ret)[1] = stack[sp + 2];
    return ret;
}

float Execution::stackPopFloat(void) {
    return *(float *)&stack[sp--];
}

double Execution::stackPopDouble(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[0] = stack[sp + 1];
    ((uint32_t *)&ret)[1] = stack[sp + 2];
    return *(double *)&ret;
}

MjvmObject *Execution::stackPopObject(void) {
    return (MjvmObject *)stack[sp--];
}

void Execution::stackSaveContext(uint32_t retPc) {
    stackPushInt32((int32_t)method);
    stackPushInt32(retPc);
    stackPushInt32(startSp);
    stackPushInt32((int32_t)locals);
    startSp = sp;
}

void Execution::stackRestoreContext(void) {
    sp = startSp;
    locals = (int32_t *)stackPopInt32();
    startSp = stackPopInt32();
    pc = stackPopInt32();
    method = (const MethodInfo *)stackPopInt32();
    code = method->getAttributeCode().code;
}

void Execution::initNewContext(const MethodInfo &methodInfo, uint16_t argc) {
    const AttributeCode &attributeCode = methodInfo.getAttributeCode();
    if((sp + attributeCode.maxLocals + attributeCode.maxStack) >= stackLength)
        throw "stack overflow";
    method = &methodInfo;
    code = attributeCode.code;
    startSp = sp;
    pc = 0;
    locals = &stack[sp + 1];
    for(uint32_t i = argc; i < attributeCode.maxLocals; i++)
        stack[sp + i + 1] = 0;
    sp += attributeCode.maxLocals;
}

const MethodInfo &Execution::findMethod(const ConstMethod &constMethod) {
    const ClassLoader *loader;
    if(constMethod.className == method->classLoader.getThisClass())
        loader = (const ClassLoader *)&method->classLoader;
    else
        loader = &load(constMethod.className);
    while(loader) {
        const MethodInfo *methodInfo = (const MethodInfo *)&loader->getMethodInfo(constMethod.nameAndType);
        if(methodInfo)
            return *methodInfo;
        else
            loader = (const ClassLoader *)&load(loader->getSupperClass());
    }
    throw "can't find the method";
}

void Execution::callMethod(const MethodInfo &methodInfo, uint32_t retPc) {
    uint8_t argc = methodInfo.parseParamInfo().argc;
    if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC)
        argc++;
    for(uint32_t i = 0; i < argc; i++)
        stack[sp - i + 4] = stack[sp - i];
    sp -= argc;
    stackSaveContext(retPc);
    initNewContext(methodInfo, argc);
}

int64_t Execution::run(const char *mainClass) {
    static const void *opcodes[256] = {
        &&op_nop, &&op_aconst_null, &&op_iconst_m1, &&op_iconst_0, &&op_iconst_1, &&op_iconst_2, &&op_iconst_3, &&op_iconst_4, &&op_iconst_5,
        &&op_lconst_0, &&op_lconst_1, &&op_fconst_0, &&op_fconst_1, &&op_fconst_2, &&op_dconst_0, &&op_dconst_1, &&op_bipush, &&op_sipush,
        &&op_ldc, &&op_ldc_w, &&op_ldc2_w, &&op_iload, &&op_lload, &&op_fload, &&op_dload, &&op_aload, &&op_iload_0, &&op_iload_1, &&op_iload_2,
        &&op_iload_3, &&op_lload_0, &&op_lload_1, &&op_lload_2, &&op_lload_3, &&op_fload_0, &&op_fload_1, &&op_fload_2, &&op_fload_3,
        &&op_dload_0, &&op_dload_1, &&op_dload_2, &&op_dload_3, &&op_aload_0, &&op_aload_1, &&op_aload_2, &&op_aload_3, &&op_iaload, &&op_laload,
        &&op_faload, &&op_daload, &&op_aaload, &&op_baload, &&op_caload, &&op_saload, &&op_istore, &&op_lstore, &&op_fstore, &&op_dstore,
        &&op_astore, &&op_istore_0, &&op_istore_1, &&op_istore_2, &&op_istore_3, &&op_lstore_0, &&op_lstore_1, &&op_lstore_2, &&op_lstore_3,
        &&op_fstore_0, &&op_fstore_1, &&op_fstore_2, &&op_fstore_3, &&op_dstore_0, &&op_dstore_1, &&op_dstore_2, &&op_dstore_3, &&op_astore_0,
        &&op_astore_1, &&op_astore_2, &&op_astore_3, &&op_iastore, &&op_lastore, &&op_fastore, &&op_dastore, &&op_aastore, &&op_bastore,
        &&op_castore, &&op_sastore, &&op_pop, &&op_pop2, &&op_dup, &&op_dup_x1, &&op_dup_x2, &&op_dup2, &&op_dup2_x1, &&op_dup2_x2, &&op_unknow,
        &&op_iadd, &&op_ladd, &&op_fadd, &&op_dadd, &&op_isub, &&op_lsub, &&op_fsub, &&op_dsub, &&op_imul, &&op_lmul, &&op_fmul, &&op_dmul,
        &&op_idiv, &&op_ldiv, &&op_fdiv, &&op_ddiv, &&op_irem, &&op_lrem, &&op_frem, &&op_drem, &&op_ineg, &&op_lneg, &&op_fneg, &&op_dneg,
        &&op_ishl, &&op_lshl, &&op_ishr, &&op_lshr, &&op_iushr, &&op_lushr, &&op_iand, &&op_land, &&op_ior, &&op_lor, &&op_ixor, &&op_lxor,
        &&op_iinc, &&op_i2l, &&op_i2f, &&op_i2d, &&op_l2i, &&op_l2f, &&op_l2d, &&op_f2i, &&op_f2l, &&op_f2d, &&op_d2i, &&op_d2l, &&op_d2f,
        &&op_i2b, &&op_i2c, &&op_i2s, &&op_lcmp, &&op_fcmpl, &&op_fcmpg, &&op_dcmpl, &&op_dcmpg, &&op_ifeq, &&op_ifne, &&op_iflt, &&op_ifge,
        &&op_ifgt, &&op_ifle, &&op_if_icmpeq, &&op_if_icmpne, &&op_if_icmplt, &&op_if_icmpge, &&op_if_icmpgt, &&op_if_icmple, &&op_if_acmpeq,
        &&op_if_acmpne, &&op_goto, &&op_jsr, &&op_ret, &&op_tableswitch, &&op_lookupswitch, &&op_ireturn, &&op_lreturn, &&op_freturn,
        &&op_dreturn, &&op_areturn, &&op_return, &&op_getstatic, &&op_putstatic, &&op_getfield, &&op_putfield, &&op_invokevirtual,
        &&op_invokespecial, &&op_invokestatic, &&op_invokeinterface, &&op_invokedynamic, &&op_new, &&op_newarray, &&op_anewarray,
        &&op_arraylength, &&op_athrow, &&op_checkcast, &&op_instanceof, &&op_monitorenter, &&op_monitorexit, &&op_wide, &&op_multianewarray,
        &&op_ifnull, &&op_ifnonnull, &&op_goto_w, &&op_jsrw, &&op_breakpoint, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
    };
    static const uint8_t primitiveTypeSize[8] = {
        sizeof(int8_t), sizeof(int16_t), sizeof(float), sizeof(double),
        sizeof(int8_t), sizeof(int16_t), sizeof(int32_t), sizeof(int64_t)
    };
    static const ConstUtf8 *primTypeConstUtf8List[] = {
        (ConstUtf8 *)"\1\0Z",
        (ConstUtf8 *)"\1\0C",
        (ConstUtf8 *)"\1\0F",
        (ConstUtf8 *)"\1\0D",
        (ConstUtf8 *)"\1\0B",
        (ConstUtf8 *)"\1\0S",
        (ConstUtf8 *)"\1\0I",
        (ConstUtf8 *)"\1\0J",
    };

    method = &load(mainClass).getMainMethodInfo();

    initNewContext(*method);

    goto *opcodes[code[pc]];
    op_nop:
        goto *opcodes[code[pc]];
    op_iconst_m1:
        stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_0:
    op_aconst_null:
        stackPushInt32(0);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_1:
        stackPushInt32(1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_2:
        stackPushInt32(2);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_3:
        stackPushInt32(3);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_4:
        stackPushInt32(4);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_5:
        stackPushInt32(5);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_0:
        stackPushInt64(0);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_1:
        stackPushInt64(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_0:
        stackPushFloat(0);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_1:
        stackPushFloat(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_2:
        stackPushFloat(2);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_0:
        stackPushDouble(0);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_1:
        stackPushDouble(1);
        pc++;
        goto *opcodes[code[pc]];
    op_bipush:
        stackPushInt32(code[pc + 1]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_sipush:
        stackPushInt32(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        goto *opcodes[code[pc]];
    op_ldc: {
        const ConstPool &constPool = method->classLoader.getConstPool(code[pc + 1]);
        pc += 2;
        switch(constPool.tag) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING:
                stackPushInt32((int32_t)&method->classLoader.getConstString(constPool));
                goto new_string;
            case CONST_CLASS:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_ldc_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING:
                stackPushInt32((int32_t)&method->classLoader.getConstString(constPool));
                goto new_string;
            case CONST_CLASS:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_ldc2_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag) {
            case CONST_LONG:
                stackPushInt64(method->classLoader.getConstLong(constPool));
                goto *opcodes[code[pc]];
            case CONST_DOUBLE:
                stackPushDouble(method->classLoader.getConstDouble(constPool));
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_iload:
    op_fload:
        stackPushInt32(locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_iload_0:
    op_fload_0:
        stackPushInt32(locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_1:
    op_fload_1:
        stackPushInt32(locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_2:
    op_fload_2:
        stackPushInt32(locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_3:
    op_fload_3:
        stackPushInt32(locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload:
    op_dload:
        stackPushInt64(*(int64_t *)&locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_lload_0:
    op_dload_0:
        stackPushInt64(*(int64_t *)&locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_1:
    op_dload_1:
        stackPushInt64(*(int64_t *)&locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_2:
    op_dload_2:
        stackPushInt64(*(int64_t *)&locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_3:
    op_dload_3:
        stackPushInt64(*(int64_t *)&locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload:
        stackPushObject((MjvmObject *)locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_aload_0:
        stackPushObject((MjvmObject *)locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_1:
        stackPushObject((MjvmObject *)locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_2:
        stackPushObject((MjvmObject *)locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_3:
        stackPushObject((MjvmObject *)locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_iaload:
    op_faload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_laload:
    op_daload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt64(((int64_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_aaload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushObject((MjvmObject *)((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_baload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int8_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_caload:
    op_saload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int16_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore:
    op_fstore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        index = &locals[index] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_lstore:
    op_dstore: {
        uint32_t index = code[pc + 1];
        *(uint64_t *)&locals[index] = stackPopInt64();
        index = &locals[index] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_astore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        index = &locals[index] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_istore_0:
    op_fstore_0: {
        locals[0] = stackPopInt32();
        uint32_t index = &locals[0] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_1:
    op_fstore_1: {
        locals[1] = stackPopInt32();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_2:
    op_fstore_2: {
        locals[2] = stackPopInt32();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_3:
    op_fstore_3: {
        locals[3] = stackPopInt32();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_0:
    op_dstore_0: {
        *(uint64_t *)&locals[0] = stackPopInt64();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_1:
    op_dstore_1: {
        *(uint64_t *)&locals[1] = stackPopInt64();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_2:
    op_dstore_2: {
        *(uint64_t *)&locals[2] = stackPopInt64();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_3:
    op_dstore_3: {
        *(uint64_t *)&locals[3] = stackPopInt64();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_0: {
        locals[0] = stackPopInt32();
        uint32_t index = &locals[0] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_1: {
        locals[1] = stackPopInt32();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_2: {
        locals[2] = stackPopInt32();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_3: {
        locals[3] = stackPopInt32();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iastore:
    op_fastore:
    op_aastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int32_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lastore:
    op_dastore: {
        int64_t value = stackPopInt64();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if((index < 0) || (index >= (obj->size / 8))) {
            // TODO
        }
        ((int64_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_bastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int8_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_castore:
    op_sastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int16_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_pop:
        stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    op_pop2:
        stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    op_dup:
        stackPush(getStackValue(sp));
        pc++;
        goto *opcodes[code[pc]];
    op_dup_x1: {
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        setStackValue(sp - 1, value1);
        setStackValue(sp - 0, value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x2: {
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        setStackValue(sp - 2, value1);
        setStackValue(sp - 1, value3);
        setStackValue(sp - 0, value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2: {
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x1: {
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        setStackValue(sp - 2, value2);
        setStackValue(sp - 1, value1);
        setStackValue(sp - 0, value3);
        stackPush(value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x2: {
        StackValue value4 = getStackValue(sp - 3);
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        setStackValue(sp - 2, value2);
        setStackValue(sp - 2, value1);
        setStackValue(sp - 1, value4);
        setStackValue(sp - 0, value3);
        stackPush(value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iadd: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ladd: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fadd: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dadd: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_isub: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lsub: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fsub: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dsub: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_imul: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lmul: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fmul: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dmul: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_idiv: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt32(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ldiv: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt64(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fdiv: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ddiv: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushFloat(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_irem: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt32(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lrem: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt64(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_frem: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        int32_t temp = (int32_t)(value1 / value2);
        stackPushFloat(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_drem: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        int64_t temp = (int64_t)(value1 / value2);
        stackPushDouble(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ineg:
        stack[sp] = -stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lneg:
        stackPushInt64(-stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_fneg:
        stackPushFloat(-stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_dneg:
        stackPushDouble(-stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_ishl: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshl: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ishr: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshr: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iushr: {
        int32_t position = stackPopInt32();
        uint32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lushr: {
        int32_t position = stackPopInt32();
        uint64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iand: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_land: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ior: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ixor: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lxor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iinc:
        locals[code[pc + 1]] += (int8_t )code[pc + 2];
        pc += 3;
        goto *opcodes[code[pc]];
    op_i2l:
        stackPushInt64(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_i2f: {
        float value = stack[sp];
        stack[sp] = *(int32_t *)&value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_i2d:
        stackPushDouble(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_l2i:
        stackPushInt32(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2f:
        stackPushFloat(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2d:
        stackPushDouble(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_f2i:
        stack[sp] = *(float *)&stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_f2l:
        stackPushInt64(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_f2d:
        stackPushDouble(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_d2i:
        stackPushInt32(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2l:
        stackPushInt64(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2f:
        stackPushFloat(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_i2b:
        stack[sp] = (int8_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_i2c:
    op_i2s:
        stack[sp] = (int16_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lcmp: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt32((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 0));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fcmpl:
    op_fcmpg: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        if((*(uint32_t *)&value1 == FLOAT_NAN) || (*(uint32_t *)&value2 == FLOAT_NAN))
            stackPushInt32((code[pc] == OP_FCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dcmpl:
    op_dcmpg: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        if((*(uint64_t *)&value1 == DOUBLE_NAN) || (*(uint64_t *)&value2 == DOUBLE_NAN))
            stackPushInt32((code[pc] == OP_DCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ifeq:
    op_ifnull:
        pc += (!stackPopInt32()) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifne:
    op_ifnonnull:
        pc += stackPopInt32() ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_iflt:
        pc += (stackPopInt32() < 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifge:
        pc += (stackPopInt32() >= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifgt:
        pc += (stackPopInt32() > 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifle:
        pc += (stackPopInt32() <= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_if_icmpeq:
    op_if_acmpeq: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 == value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpne:
    op_if_acmpne: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 != value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmplt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 < value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpge: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 >= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpgt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 > value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmple: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 <= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_goto:
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_goto_w:
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsr:
        stackPushInt32(pc + 3);
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsrw:
        stackPushInt32(pc + 5);
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_ret:
        pc = locals[code[pc + 1]];
        goto *opcodes[code[pc]];
    op_tableswitch: {
        int32_t index = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t low = ARRAY_TO_INT32(&table[4]);
        int32_t height = ARRAY_TO_INT32(&table[8]);
        if(index < low || index > height) {
            int32_t defaultOffset = ARRAY_TO_INT32(table);
            pc += defaultOffset;
            goto *opcodes[code[pc]];
        }
        table = &table[12 + (index - low) * 4];
        pc += ARRAY_TO_INT32(table);
        goto *opcodes[code[pc]];
    }
    op_lookupswitch: {
        int32_t key = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t defaultPc = ARRAY_TO_INT32(table);
        int32_t npairs = ARRAY_TO_INT32(&table[4]);
        table = &table[8];
        while(npairs--) {
            int32_t pairs = ARRAY_TO_INT32(table);
            if(key == pairs) {
                pc = ARRAY_TO_INT32(&table[4]);
                goto *opcodes[code[pc]];
            }
            table = &table[8];
        }
        table = &code[pc + padding + 1];
        pc = ARRAY_TO_INT32(table);
        goto *opcodes[code[pc]];
    }
    op_ireturn:
    op_freturn: {
        int32_t retVal = stackPopInt32();
        if(startSp < 0)
            return retVal;
        stackRestoreContext();
        stackPushInt32(retVal);
        goto *opcodes[code[pc]];
    }
    op_lreturn:
    op_dreturn: {
        int64_t retVal = stackPopInt64();
        if(startSp < 0)
            return retVal;
        stackRestoreContext();
        stackPushInt64(retVal);
        goto *opcodes[code[pc]];
    }
    op_areturn: {
        int32_t retVal = (int32_t)stackPopObject();
        if(startSp < 0)
            return retVal;
        stackRestoreContext();
        stackPushObject((MjvmObject *)retVal);
        goto *opcodes[code[pc]];
    }
    op_return: {
        if(startSp < 0)
            return 0;
        stackRestoreContext();
        goto *opcodes[code[pc]];
    }
    op_getstatic: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        const FieldsData &fields = getStaticFields(constField.className);
        if((int32_t)&fields == 0) {
            stackPushInt32((int32_t)&load(constField.className));
            goto init_static_field;
        }
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                stackPushInt32(fields.getFieldData8(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                stackPushInt32(fields.getFieldData16(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                stackPushInt64(fields.getFieldData64(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                stackPushObject(fields.getFieldObject(constField.nameAndType).object);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                stackPushInt32(fields.getFieldData32(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putstatic: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        const FieldsData &fields = getStaticFields(constField.className);
        if((int32_t)&fields == 0) {
            stackPushInt32((int32_t)&load(constField.className));
            goto init_static_field;
        }
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                fields.getFieldData8(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                fields.getFieldData16(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                fields.getFieldData64(constField.nameAndType).value = stackPopInt64();
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                fields.getFieldObject(constField.nameAndType).object = stackPopObject();
                goto *opcodes[code[pc]];
            }
            default: {
                fields.getFieldData32(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
        }
    }
    op_getfield: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData8(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData16(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt64(fields.getFieldData64(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushObject(fields.getFieldObject(constField.nameAndType).object);
                goto *opcodes[code[pc]];
            }
            default: {
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData32(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putfield: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData8(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData16(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = stackPopInt64();
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData64(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                MjvmObject *value = stackPopObject();
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldObject(constField.nameAndType).object = value;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0) {
                    // TODO
                }
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData32(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_invokevirtual: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        ParamInfo paramInfo = constMethod.parseParamInfo();
        MjvmObject *obj = (MjvmObject *)stack[sp - paramInfo.argc];
        uint32_t virtualConstMethod[] = {
            (uint32_t)&obj->type,                /* class name */
            (uint32_t)&constMethod.nameAndType   /* name and type */
        };
        const MethodInfo &methodInfo = findMethod(*(const ConstMethod *)virtualConstMethod);
        if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC)
            callMethod(methodInfo, pc + 3);
        else
            throw "invoke special to static method";
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        const MethodInfo &methodInfo = findMethod(constMethod);
        if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC)
            callMethod(methodInfo, pc + 3);
        else
            throw "invoke special to static method";
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        const MethodInfo &methodInfo = findMethod(constMethod);
        if((methodInfo.accessFlag & METHOD_STATIC) == METHOD_STATIC)
            callMethod(methodInfo, pc + 3);
        else
            throw "invoke static to non-static method";
        goto *opcodes[code[pc]];
    }
    op_invokeinterface:
        // TODO
        goto *opcodes[code[pc]];
    op_invokedynamic:
        // TODO
        goto *opcodes[code[pc]];
    op_new: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstUtf8 &constClass =  method->classLoader.getConstClass(poolIndex);
        MjvmObject *obj = newObject(sizeof(FieldsData), constClass);
        new ((FieldsData *)obj->data)FieldsData(*this, load(constClass), false);
        stackPushObject(obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_newarray: {
        int32_t count = stackPopInt32();
        if(count < 0) {
            // TODO
        }
        uint8_t atype = code[pc + 1];
        uint8_t typeSize = primitiveTypeSize[atype - 4];
        MjvmObject *obj = newObject(typeSize * count, *primTypeConstUtf8List[atype - 4], 1);
        memset(obj->data, 0, obj->size);
        stackPushObject(obj);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = stackPopInt32();
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstUtf8 &constClass =  method->classLoader.getConstClass(poolIndex);
        MjvmObject *obj = newObject(4 * count, constClass, 1);
        memset(obj->data, 0, obj->size);
        stackPushObject(obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            // TODO
        }
        stackPushInt32(obj->size / obj->parseTypeSize());
        pc++;
        goto *opcodes[code[pc]];
    }
    op_athrow:
        // TODO
        goto *opcodes[code[pc]];
    op_checkcast:
        // TODO
        goto *opcodes[code[pc]];
    op_instanceof:
        // TODO
        goto *opcodes[code[pc]];
    op_monitorenter:
        // TODO
        goto *opcodes[code[pc]];
    op_monitorexit:
        // TODO
        goto *opcodes[code[pc]];
    op_wide:
        // TODO
        goto *opcodes[code[pc]];
    op_multianewarray:
        // TODO
        goto *opcodes[code[pc]];
    op_breakpoint:
        pc++;
        goto *opcodes[code[pc]];
    op_unknow:
        throw "unknow opcode";
    init_static_field: {
        const ClassLoader &classToInit = *(const ClassLoader *)stackPopInt32();
        const MethodInfo &staticContructor = classToInit.getStaticContructor();
        if((int32_t)&staticContructor)
            callMethod(staticContructor, pc);
        goto *opcodes[code[pc]];
    }
    new_string: {
        static const uint32_t nameAndType[] = {
            (uint32_t)"\6\0<init>",             /* method name */
            (uint32_t)"\5\0([B)V"               /* method type */
        };
        static const uint32_t strCtorConstMethod[] = {
            (uint32_t)"\16\0java/lang/String",  /* class name */
            (uint32_t)nameAndType               /* name and type */
        };
        const ConstUtf8 &text = *(const ConstUtf8 *)stackPopInt32();
        /* create new string object and call string contructor */
        const MethodInfo &methodInfo = findMethod(*(ConstMethod *)strCtorConstMethod);
        MjvmObject *strObj = newObject(sizeof(FieldsData), methodInfo.classLoader.getThisClass());
        new ((FieldsData *)strObj->data)FieldsData(*this, methodInfo.classLoader, false);
        stackPushObject(strObj);
        stackPushObject(strObj);                /* Dup */
        /* create new byte array to store text */
        MjvmObject *byteArray = newObject(text.length, *primTypeConstUtf8List[4], 1);
        memcpy(byteArray->data, text.getText(), text.length);
        stackPushObject(byteArray);
        if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC)
            callMethod(methodInfo, pc);
        else
            throw "invoke special to static method";
        goto *opcodes[code[pc]];
    }
}

Execution::~Execution(void) {
    MjvmHeap::free(stack);
    MjvmHeap::free(stackType);
    if(staticClassDataHead) {
        for(ClassDataNode *node = staticClassDataHead; node != 0;) {
            ClassDataNode *next = node->next;
            ClassLoader::destroy(node->classLoader);
            if(node->filedsData) {
                node->filedsData->~FieldsData();
                MjvmHeap::free(node->filedsData);
            }
            node->~ClassDataNode();
            MjvmHeap::free(node);
            node = next;
        }
    }
    MjvmHeap::freeAllObject((uint32_t)this);
}
