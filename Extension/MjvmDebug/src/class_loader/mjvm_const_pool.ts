
export class MjvmConstClass {
    public readonly constUtf8Index: number;

    public constructor(constUtf8Index: number) {
        this.constUtf8Index = constUtf8Index;
    }
}

export class MjvmConstSting {
    public readonly constUtf8Index: number;

    public constructor(constUtf8Index: number) {
        this.constUtf8Index = constUtf8Index;
    }
}

export class MjvmConstMethodType {
    public readonly descriptorIndex: number;

    public constructor(descriptorIndex: number) {
        this.descriptorIndex = descriptorIndex;
    }
}

export class MjvmConstField {
    public readonly classIndex: number;
    public readonly nameAndTypeIndex: number;

    public constructor(classIndex: number, nameAndTypeIndex: number) {
        this.classIndex = classIndex;
        this.nameAndTypeIndex = nameAndTypeIndex;
    }
}

export class MjvmConstMethod {
    public readonly classIndex: number;
    public readonly nameAndTypeIndex: number;

    public constructor(classIndex: number, nameAndTypeIndex: number) {
        this.classIndex = classIndex;
        this.nameAndTypeIndex = nameAndTypeIndex;
    }
}

export class MjvmConstInterfaceMethod {
    public readonly classIndex: number;
    public readonly nameAndTypeIndex: number;

    public constructor(classIndex: number, nameAndTypeIndex: number) {
        this.classIndex = classIndex;
        this.nameAndTypeIndex = nameAndTypeIndex;
    }
}

export class MjvmConstNameAndType {
    public readonly nameIndex: number;
    public readonly descriptorIndex: number;

    public constructor(nameIndex: number, descriptorIndex: number) {
        this.nameIndex = nameIndex;
        this.descriptorIndex = descriptorIndex;
    }
}

export class MjvmConstInvokeDynamic {
    public readonly bootstrapMethodAttrIndex: number;
    public readonly nameAndTypeIndex: number;

    public constructor(bootstrapMethodAttrIndex: number, nameAndTypeIndex: number) {
        this.bootstrapMethodAttrIndex = bootstrapMethodAttrIndex;
        this.nameAndTypeIndex = nameAndTypeIndex;
    }
}

export class MjvmConstMethodHandle {
    public readonly referenceKind: number;
    public readonly referenceIndex: number;

    public constructor(referenceKind: number, referenceIndex: number) {
        this.referenceKind = referenceKind;
        this.referenceIndex = referenceIndex;
    }
}
