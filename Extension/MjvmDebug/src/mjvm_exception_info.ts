
export class MjvmExceptionInfo {
    public readonly type: string;
    public readonly message?: string;

    public constructor(type: string, message?: string) {
        this.type = type;
        this.message = message;
    }
}
