
export class MjvmFieldInfo {
    public readonly accessFlag: number;
    public readonly name: string;
    public readonly descriptor: string;
    public readonly constValue?: number | bigint;

    public constructor(name: string, descriptor: string, accessFlag: number, constValue?: number | bigint) {
        this.name = name;
        this.descriptor = descriptor;
        this.accessFlag = accessFlag;
        this.constValue = constValue;
    }
}
