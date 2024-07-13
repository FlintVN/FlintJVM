
export class MjvmDataResponse {
    public cmd: number;
    public data: Buffer;
    public responseCode: number;
    public receivedLength: number = 0;

    public constructor(cmd: number, responseCode: number, dataLength: number) {
        this.cmd = cmd;
        this.responseCode = responseCode;
        this.data = Buffer.alloc(dataLength);
    }
}
