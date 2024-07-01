
export class AttributeInfo {
    public static readonly ATTRIBUTE_CONSTANT_VALUE = 0;
    public static readonly ATTRIBUTE_CODE = 1;
    public static readonly ATTRIBUTE_STACK_MAP_TABLE = 2;
    public static readonly ATTRIBUTE_EXCEPTIONS = 3;
    public static readonly ATTRIBUTE_INNER_CLASSES = 4;
    public static readonly ATTRIBUTE_ENCLOSING_METHOD = 5;
    public static readonly ATTRIBUTE_SYNTHETIC = 6;
    public static readonly ATTRIBUTE_SIGNATURE = 7;
    public static readonly ATTRIBUTE_SOURCE_FILE = 8;
    public static readonly ATTRIBUTE_SOURCE_DEBUG_EXTENSION = 9;
    public static readonly ATTRIBUTE_LINE_NUMBER_TABLE = 10;
    public static readonly ATTRIBUTE_LOCAL_VARIABLE_TABLE = 11;
    public static readonly ATTRIBUTE_LOCAL_VARIABLE_TYPE_TABLE = 12;
    public static readonly ATTRIBUTE_DEPRECATED = 13;
    public static readonly ATTRIBUTE_RUNTIME_VISIBLE_ANNOTATIONS = 14;
    public static readonly ATTRIBUTE_RUNTIME_INVISIBLE_ANNOTATIONS = 15;
    public static readonly ATTRIBUTE_RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS = 16;
    public static readonly ATTRIBUTE_RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS = 17;
    public static readonly ATTRIBUTE_ANNOTATION_DEFAULT = 18;
    public static readonly ATTRIBUTE_BOOTSTRAP_METHODS = 19;
    public static readonly ATTRIBUTE_NEST_MEMBERS = 20;
    public static readonly ATTRIBUTE_NATIVE = 21;
    public static readonly ATTRIBUTE_UNKNOW = 0xFF;

    public tag: number;

    protected constructor(tag: number) {
        this.tag = tag;
    }

    public static parseAttributeType(name: string) : number {
        if(name === "Code")
            return AttributeInfo.ATTRIBUTE_CODE;
        else if(name === "LineNumberTable")
                return AttributeInfo.ATTRIBUTE_LINE_NUMBER_TABLE;
        else if(name === "LocalVariableTable")
            return AttributeInfo.ATTRIBUTE_LOCAL_VARIABLE_TABLE;
        else if(name === "ConstantValue")
            return AttributeInfo.ATTRIBUTE_CONSTANT_VALUE;
        return AttributeInfo.ATTRIBUTE_UNKNOW;
    }
}


export class LineNumber {
    public readonly startPc: number;
    public readonly line: number;

    public constructor(startPc: number, line: number) {
        this.startPc = startPc;
        this.line = line;
    }
}

export class AttributeLineNumber extends AttributeInfo {
    public linesNumber: LineNumber[];

    public constructor(linesNumber: LineNumber[]) {
        super(AttributeInfo.ATTRIBUTE_LINE_NUMBER_TABLE);
        this.linesNumber = linesNumber;
    }
}

export class AttributeCode extends AttributeInfo {
    public readonly maxStack: number;
    public readonly maxLocals: number;
    public readonly code: Buffer | null;
    public attributes: AttributeInfo[] | null;

    public constructor(maxStack: number, maxLocals: number, code: Buffer | null, attributes: AttributeInfo[] | null) {
        super(AttributeInfo.ATTRIBUTE_CODE);
        this.maxStack = maxStack;
        this.maxLocals = maxLocals;
        this.code = code;
        this.attributes = attributes;
    }

    public getLinesNumber() : AttributeLineNumber | null {
        if(this.attributes) {
            for(let i = 0; i < this.attributes.length; i++) {
                if(this.attributes[i].tag === AttributeInfo.ATTRIBUTE_LINE_NUMBER_TABLE)
                    return this.attributes[i] as AttributeLineNumber;
            };
        }
        return null;
    }
}
