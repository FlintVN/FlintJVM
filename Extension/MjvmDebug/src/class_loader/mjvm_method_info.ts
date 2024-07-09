import { MjvmCodeAttribute } from "./mjvm_attribute_info";

export class MjvmMethodInfo {
    public static readonly METHOD_PUBLIC = 0x0001;
    public static readonly METHOD_PRIVATE = 0x0002;
    public static readonly METHOD_PROTECTED = 0x0004;
    public static readonly METHOD_STATIC = 0x0008;
    public static readonly METHOD_FINAL = 0x0010;
    public static readonly METHOD_SYNCHRONIZED = 0x0020;
    public static readonly METHOD_BRIDGE = 0x0040;
    public static readonly METHOD_VARARGS = 0x0080;
    public static readonly METHOD_NATIVE = 0x0100;
    public static readonly METHOD_ABSTRACT = 0x0400;
    public static readonly METHOD_STRICT = 0x0800;
    public static readonly METHOD_SYNTHETIC = 0x1000;

    public readonly name: string;
    public readonly descriptor: string;
    public readonly attributeCode?: MjvmCodeAttribute;

    public constructor(name: string, descriptor: string, code?: MjvmCodeAttribute) {
        this.name = name;
        this.descriptor = descriptor;
        this.attributeCode = code;
    }
}
