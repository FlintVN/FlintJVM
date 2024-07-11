
export class MjvmValueInfo {
    public readonly value: number | bigint;
    public readonly type: string;
    public readonly size: number;

    public constructor(value: number | bigint, type: string, size: number) {
        this.value = value;
        this.type = type;
        this.size = size;
    }
}
