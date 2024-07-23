
import fs = require('fs');
import path = require('path');
import * as vscode from 'vscode';
import {
    MjvmConstClass,
    MjvmConstSting,
    MjvmConstMethodType,
    MjvmConstField,
    MjvmConstMethod,
    MjvmConstInterfaceMethod,
    MjvmConstNameAndType,
    MjvmConstInvokeDynamic,
    MjvmConstMethodHandle
} from './mjvm_const_pool';
import {
    MjvmAttribute,
    MjvmCodeAttribute,
    MjvmLineNumber,
    MjvmLineNumberAttribute,
    MjvmLocalVariable,
    MjvmLocalVariableAttribute
} from './mjvm_attribute_info';
import { MjvmMethodInfo } from './mjvm_method_info';
import { MjvmFieldInfo } from './mjvm_field_info';

export class MjvmClassLoader {
    public readonly magic: number;
    public readonly minorVersion: number;
    public readonly majorVersion: number;
    public readonly accessFlags: number;
    public readonly thisClass: string;
    public readonly superClass?: string;
    public readonly interfacesCount: number;
    public readonly classPath: string;
    public readonly sourcePath?: string;

    private fieldInfos?: MjvmFieldInfo[];
    public methodsInfos: MjvmMethodInfo[];

    private readonly poolTable: (
        number |
        bigint |
        string |
        MjvmConstClass |
        MjvmConstSting |
        MjvmConstMethodType |
        MjvmConstField |
        MjvmConstMethod |
        MjvmConstInterfaceMethod |
        MjvmConstNameAndType |
        MjvmConstInvokeDynamic |
        MjvmConstMethodHandle
    )[] = [];

    public static sdkClassPath?: string;
    public static sdkSourcePath?: string;

    private static readonly CONST_UTF8 = 1;
    private static readonly CONST_INTEGER = 3;
    private static readonly CONST_FLOAT = 4;
    private static readonly CONST_LONG = 5;
    private static readonly CONST_DOUBLE = 6;
    private static readonly CONST_CLASS = 7;
    private static readonly CONST_STRING = 8;
    private static readonly CONST_FIELD = 9;
    private static readonly CONST_METHOD = 10;
    private static readonly CONST_INTERFACE_METHOD = 11;
    private static readonly CONST_NAME_AND_TYPE = 12;
    private static readonly CONST_METHOD_HANDLE = 15;
    private static readonly CONST_METHOD_TYPE = 16;
    private static readonly CONST_INVOKE_DYNAMIC = 18;

    private static readonly CLASS_PUBLIC = 0x0001;
    private static readonly CLASS_FINAL = 0x0010;
    private static readonly CLASS_SUPER = 0x0020;
    private static readonly CLASS_INTERFACE = 0x0200;
    private static readonly CLASS_ABSTRACT = 0x0400;
    private static readonly CLASS_SYNTHETIC = 0x1000;
    private static readonly CLASS_ANNOTATION = 0x2000;
    
    private static readonly FIELD_PUBLIC = 0x0001;
    private static readonly FIELD_PRIVATE = 0x0002;
    private static readonly FIELD_PROTECTED = 0x0004;
    private static readonly FIELD_STATIC = 0x0008;
    private static readonly FIELD_FINAL = 0x0010;
    private static readonly FIELD_VOLATILE = 0x0040;
    private static readonly FIELD_TRANSIENT = 0x0080;
    private static readonly FIELD_SYNTHETIC = 0x1000;
    private static readonly FIELD_ENUM = 0x4000;
    private static readonly FIELD_UNLOAD = 0x8000;

    private static classLoaderDictionary: Record<string, MjvmClassLoader> = {};

    private static findSourceFile(name: string): string | undefined {
        name += '.java';
        const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
        let fullPath = path.join(workspace, name);
        if(fs.existsSync(fullPath))
            return fullPath;
        else if(MjvmClassLoader.sdkSourcePath) {
            fullPath = path.join(MjvmClassLoader.sdkSourcePath, name);
            if(fs.existsSync(fullPath))
                return fullPath;
        }
        return undefined;
    }

    private static findClassFile(name: string): string | undefined {
        name += '.class';
        const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
        let fullPath = path.join(workspace, name);
        if(fs.existsSync(fullPath))
            return fullPath;
        else if(MjvmClassLoader.sdkClassPath) {
            fullPath = path.join(MjvmClassLoader.sdkClassPath, name);
            if(fs.existsSync(fullPath))
                return fullPath;
        }
        return undefined;
    }

    public static load(className: string): MjvmClassLoader {
        className = className.replace(/\\/g, '\/');
        if(!(className in MjvmClassLoader.classLoaderDictionary)) {
            const classPath = MjvmClassLoader.findClassFile(className);
            if(classPath)
                MjvmClassLoader.classLoaderDictionary[className] = new MjvmClassLoader(classPath);
            else
                throw 'Could not find ' + '\"' + className + '\"' + 'class file';
        }
        return MjvmClassLoader.classLoaderDictionary[className];
    }

    private constructor(filePath: string) {
        this.classPath = filePath;
        const data = fs.readFileSync(filePath, undefined);

        let index = 0;
        this.magic = this.readU32(data, index);
        index += 4;
        this.minorVersion = this.readU16(data, index);
        index += 2;
        this.majorVersion = this.readU16(data, index);
        index += 2;
        const poolCount = this.readU16(data, index) - 1;
        index += 2;

        for(let i = 0; i < poolCount; i++) {
            const tag = data[index];
            index++;
            switch(tag) {
                case MjvmClassLoader.CONST_UTF8: {
                    const length = this.readU16(data, index);
                    index += 2;
                    this.poolTable.push(data.toString('utf-8', index, index + length));
                    index += length;
                    break;
                }
                case MjvmClassLoader.CONST_INTEGER:
                case MjvmClassLoader.CONST_FLOAT:
                    this.poolTable.push(this.readU32(data, index));
                    index += 4;
                    break;
                case MjvmClassLoader.CONST_FIELD:
                case MjvmClassLoader.CONST_METHOD:
                case MjvmClassLoader.CONST_INTERFACE_METHOD:
                case MjvmClassLoader.CONST_NAME_AND_TYPE:
                case MjvmClassLoader.CONST_INVOKE_DYNAMIC:
                    const index1 = this.readU16(data, index);
                    index += 2;
                    const index2 = this.readU16(data, index);
                    index += 2;
                    if(tag === MjvmClassLoader.CONST_FIELD)
                        this.poolTable.push(new MjvmConstField(index1, index2));
                    else if(tag === MjvmClassLoader.CONST_METHOD)
                        this.poolTable.push(new MjvmConstMethod(index1, index2));
                    else if(tag === MjvmClassLoader.CONST_INTERFACE_METHOD)
                        this.poolTable.push(new MjvmConstInterfaceMethod(index1, index2));
                    else if(tag === MjvmClassLoader.CONST_NAME_AND_TYPE)
                        this.poolTable.push(new MjvmConstNameAndType(index1, index2));
                    else if(tag === MjvmClassLoader.CONST_INVOKE_DYNAMIC)
                        this.poolTable.push(new MjvmConstInvokeDynamic(index1, index2));
                    break;
                case MjvmClassLoader.CONST_LONG:
                case MjvmClassLoader.CONST_DOUBLE: {
                    this.poolTable.push(this.readU64(data, index));
                    index += 8;
                    i++;
                    this.poolTable.push(0);
                    break;
                }
                case MjvmClassLoader.CONST_CLASS:
                case MjvmClassLoader.CONST_STRING:
                case MjvmClassLoader.CONST_METHOD_TYPE: {
                    const constUtf8Index = this.readU16(data, index);
                    index += 2;
                    if(tag === MjvmClassLoader.CONST_CLASS)
                        this.poolTable.push(new MjvmConstClass(constUtf8Index));
                    else if(tag === MjvmClassLoader.CONST_STRING)
                        this.poolTable.push(new MjvmConstSting(constUtf8Index));
                    else if(tag === MjvmClassLoader.CONST_METHOD_TYPE)
                        this.poolTable.push(new MjvmConstMethodType(constUtf8Index));
                    break;
                }
                case MjvmClassLoader.CONST_METHOD_HANDLE: {
                    const index1 = data[index];
                    index++;
                    const index2 = this.readU16(data, index);
                    index += 2;
                    this.poolTable.push(new MjvmConstMethodHandle(index1, index2));
                    break;
                }
                default:
                    throw "uknow pool type";
            }
        }

        this.accessFlags = this.readU16(data, index);
        index += 2;
        const thisClass = this.poolTable[this.readU16(data, index) - 1] as MjvmConstClass;
        this.thisClass = this.poolTable[thisClass.constUtf8Index - 1] as string;
        this.sourcePath = MjvmClassLoader.findSourceFile(this.thisClass);
        index += 2;
        const superClassIndex = this.readU16(data, index);
        index += 2;
        if(superClassIndex) {
            const superClass = this.poolTable[superClassIndex - 1] as MjvmConstClass;
            this.superClass = this.poolTable[superClass.constUtf8Index - 1] as string;
        }
        this.interfacesCount = this.readU16(data, index);
        index += 2;

        if(this.interfacesCount)
            index += this.interfacesCount * 2;

        const fieldsCount = this.readU16(data, index);
        index += 2;
        if(fieldsCount) {
            const fieldInfos: MjvmFieldInfo[] = [];
            for(let i = 0; i < fieldsCount; i++) {
                const flag = this.readU16(data, index);
                index += 2;
                const fieldsNameIndex = this.readU16(data, index);
                index += 2;
                const fieldsDescriptorIndex = this.readU16(data, index);
                index += 2;
                let fieldsAttributesCount = this.readU16(data, index);
                index += 2;
                while(fieldsAttributesCount--) {
                    const tmp = this.readAttribute(data, index);
                    index = tmp[0];
                }
                const fieldName: string = this.poolTable[fieldsNameIndex - 1] as string;
                const fieldDescriptor: string = this.poolTable[fieldsDescriptorIndex - 1] as string;
                fieldInfos.push(new MjvmFieldInfo(fieldName, fieldDescriptor, flag));
            }
            this.fieldInfos = fieldInfos;
        }
        const methodsCount = this.readU16(data, index);
        index += 2;
        const methodsInfos: MjvmMethodInfo[] = [];
        if(methodsCount) {
            for(let i = 0; i < methodsCount; i++) {
                const flag = this.readU16(data, index);
                index += 2;
                const methodNameIndex = this.readU16(data, index);
                index += 2;
                const methodDescriptorIndex = this.readU16(data, index);
                index += 2;
                let methodAttributesCount = this.readU16(data, index);
                index += 2;
                let attributeCode: MjvmCodeAttribute | undefined = undefined;
                while(methodAttributesCount--) {
                    const tmp = this.readAttribute(data, index);
                    index = tmp[0];
                    if(tmp[1] && !attributeCode) {
                        if(tmp[1].tag === MjvmAttribute.ATTRIBUTE_CODE)
                            attributeCode = tmp[1] as MjvmCodeAttribute;
                    }
                }
                const methodName: string = this.poolTable[methodNameIndex - 1] as string;
                const methodDescriptor: string = this.poolTable[methodDescriptorIndex - 1] as string;
                if(!(flag & (MjvmMethodInfo.METHOD_NATIVE | MjvmMethodInfo.METHOD_BRIDGE)))
                    methodsInfos.push(new MjvmMethodInfo(methodName, methodDescriptor, flag, attributeCode));
            }
        }
        this.methodsInfos = methodsInfos;
    }

    private readAttribute(data: Buffer, index: number): [number, MjvmAttribute | undefined] {
        const nameIndex = this.readU16(data, index);
        index += 2;
        const length = this.readU32(data, index);
        index += 4;
        const type = MjvmAttribute.parseAttributeType(this.poolTable[nameIndex - 1] as string);
        switch(type) {
            case MjvmAttribute.ATTRIBUTE_CODE:
                return this.readAttributeCode(data, index);
            case MjvmAttribute.ATTRIBUTE_LINE_NUMBER_TABLE:
                return this.readAttributeLineNumberTable(data, index);
            case MjvmAttribute.ATTRIBUTE_LOCAL_VARIABLE_TABLE:
                return this.readAttributeLocalVariableTable(data, index);
            default:
                index += length;
                return [index, undefined];
        }
    }

    private readAttributeCode(data: Buffer, index: number): [number, MjvmCodeAttribute] {
        const maxStack: number = this.readU16(data, index);
        index += 2;
        const maxLocals: number = this.readU16(data, index);
        index += 2;
        const codeLength: number = this.readU32(data, index);
        index += 4;
        const code = Buffer.alloc(codeLength)
        data.copy(code, 0, index, index + codeLength);
        index += codeLength;
        const exceptionTableLength = this.readU16(data, index);
        index += 2;
        index += exceptionTableLength * 8;
        let attrbutesCount = this.readU16(data, index);
        index += 2;
        if(attrbutesCount) {
            const attr: MjvmAttribute[] = [];
            while(attrbutesCount--) {
                const tmp = this.readAttribute(data, index);
                index = tmp[0];
                if(tmp[1])
                    attr.push(tmp[1]);
            }
            return [index, new MjvmCodeAttribute(maxStack, maxLocals, code, attr)];
        }
        return [index, new MjvmCodeAttribute(maxStack, maxLocals, code, undefined)];
    }

    private readAttributeLineNumberTable(data: Buffer, index: number): [number, MjvmLineNumberAttribute] {
        const lineNumberTableLength = this.readU16(data, index);
        index += 2;
        const linesNumber: MjvmLineNumber[] = [];
        for(let i = 0; i < lineNumberTableLength; i++) {
            const startPc = this.readU16(data, index);
            index += 2;
            const lineNumber = this.readU16(data, index);
            index += 2;
            linesNumber.push(new MjvmLineNumber(startPc, lineNumber));
        }
        return [index, new MjvmLineNumberAttribute(linesNumber)];
    }

    private readAttributeLocalVariableTable(data: Buffer, index: number): [number, MjvmLocalVariableAttribute] {
        const localVariableTableLength = this.readU16(data, index);
        index += 2;
        const localVariables: MjvmLocalVariable[] = [];
        for(let i = 0; i < localVariableTableLength; i++) {
            const startPc = this.readU16(data, index);
            index += 2;
            const length = this.readU16(data, index);
            index += 2;
            const nameIndex = this.readU16(data, index);
            index += 2;
            const descriptorIndex = this.readU16(data, index);
            index += 2;
            const variableIndex = this.readU16(data, index);
            index += 2;
            const name = this.poolTable[nameIndex - 1] as string;
            const descriptor = this.poolTable[descriptorIndex - 1] as string;
            localVariables.push(new MjvmLocalVariable(startPc, length, variableIndex, name, descriptor));
        }
        return [index, new MjvmLocalVariableAttribute(localVariables)];
    }

    public getFieldList(includeParent: boolean): MjvmFieldInfo[] | undefined {
        if(includeParent) {
            const ret: MjvmFieldInfo[] = [];
            if(this.superClass) {
                const parentFields = MjvmClassLoader.load(this.superClass).getFieldList(true);
                if(parentFields) {
                    for(let i = 0; i < parentFields.length; i++)
                        ret.push(parentFields[i]);
                }
            }
            if(this.fieldInfos) {
                for(let i = 0; i < this.fieldInfos.length; i++)
                    ret.push(this.fieldInfos[i]);
            }
            if(ret.length > 0)
                return ret;
            return undefined;
        }
        else
            return this.fieldInfos;
    }

    public getFieldInfo(name: string, descriptor?: string): MjvmFieldInfo | undefined {
        if(!this.fieldInfos)
            return undefined;
        if(descriptor) {
            for(let i = 0; i < this.fieldInfos.length; i++) {
                if(this.fieldInfos[i].name === name && this.fieldInfos[i].descriptor === descriptor)
                    return this.fieldInfos[i];
            }
        }
        else for(let i = 0; i < this.fieldInfos.length; i++) {
            if(this.fieldInfos[i].name === name)
                return this.fieldInfos[i];
        }
        return undefined;
    }

    public getMethodInfo(name: string, descriptor: string): MjvmMethodInfo | undefined {
        if(!this.methodsInfos)
            return undefined;
        for(let i = 0; i < this.methodsInfos.length; i++) {
            if(this.methodsInfos[i].name === name && this.methodsInfos[i].descriptor === descriptor)
                return this.methodsInfos[i];
        }
        return undefined;
    }

    public isClassOf(parentClassName: string): boolean {
        let thisClass: string | undefined = this.thisClass;
        let superClass = this.superClass;
        while(true) {
            if(thisClass === parentClassName)
                return true;
            else if(!superClass)
                return false;
            const classLoader = MjvmClassLoader.load(superClass);
            thisClass = classLoader.thisClass;
            superClass = classLoader.superClass;
        }
    }

    private readU16(data: Buffer, offset : number): number {
        let ret = data[offset + 1];
        ret |= data[offset] << 8;
        return ret;
    }

    private readU32(data: Buffer, offset : number): number {
        let ret = data[offset + 3];
        ret |= data[offset + 2] << 8;
        ret |= data[offset + 1] << 16;
        ret |= data[offset] << 24;
        return ret;
    }

    private readU64(data: Buffer, offset : number): bigint {
        let ret = BigInt(data[offset + 7]);
        ret |= BigInt(data[offset + 6]) << 8n;
        ret |= BigInt(data[offset + 5]) << 16n;
        ret |= BigInt(data[offset + 4]) << 24n;
        ret |= BigInt(data[offset + 3]) << 32n;
        ret |= BigInt(data[offset + 2]) << 40n;
        ret |= BigInt(data[offset + 1]) << 48n;
        ret |= BigInt(data[offset]) << 56n;
        return ret;
    }
}
