
import {
    StackFrame, Source,
    Variable
} from '@vscode/debugadapter';
import * as net from 'net';
import { MjvmSemaphore } from './mjvm_semaphone'
import { MjvmValueInfo } from './mjvm_value_info';
import { MjvmDataResponse } from './mjvm_data_response';
import { MjvmExceptionInfo } from './mjvm_exception_info';
import { MjvmLineInfo } from './class_loader/mjvm_line_info'
import { MjvmStackFrame } from './class_loader/mjvm_stack_frame';

export class MjvmClientDebugger {
    private static readonly DBG_READ_STATUS: number = 0;
    private static readonly DBG_READ_STACK_TRACE: number = 1;
    private static readonly DBG_ADD_BKP: number = 2;
    private static readonly DBG_REMOVE_BKP: number = 3;
    private static readonly DBG_REMOVE_ALL_BKP: number = 4;
    private static readonly DBG_RUN: number = 5;
    private static readonly DBG_STOP: number = 6;
    private static readonly DBG_STEP_IN: number = 7;
    private static readonly DBG_STEP_OVER: number = 8;
    private static readonly DBG_STEP_OUT: number = 9;
    private static readonly DBG_SET_EXCP_MODE: number = 10;
    private static readonly DBG_READ_EXCP_INFO: number = 11;
    private static readonly DBG_READ_LOCAL: number = 12;
    private static readonly DBG_WRITE_LOCAL: number = 13;

    private static readonly DBG_STATUS_STOP: number = 0x01;
    private static readonly DBG_STATUS_STOP_SET: number = 0x02;
    private static readonly DBG_STATUS_STEP_IN: number = 0x04;
    private static readonly DBG_STATUS_STEP_OVER: number = 0x08;
    private static readonly DBG_STATUS_STEP_OUT: number = 0x10;
    private static readonly DBG_STATUS_EXCP_EN: number = 0x20;
    private static readonly DBG_STATUS_EXCP: number = 0x40;
    private static readonly DBG_STATUS_DONE: number = 0x80;

    private static readonly DBG_RESP_OK = 0;
    private static readonly DBG_RESP_BUSY = 1;
    private static readonly DBG_RESP_FAIL = 2;
    private static readonly DBG_RESP_UNKNOW = 2;

    private static TCP_RECEIVED_TIMEOUT: number = 100;

    private readonly client: net.Socket;

    private rxResponse?: MjvmDataResponse;

    private requestStatusTask?: NodeJS.Timeout;

    private currentStatus: number = MjvmClientDebugger.DBG_STATUS_STOP;
    private currentStackFrames?: MjvmStackFrame[];
    private currentBreakpoints: MjvmLineInfo[] = [];

    private tcpSemaphore = new MjvmSemaphore(1);

    private stopCallback?: (reason?: string) => void;
    private errorCallback?: () => void;
    private closeCallback?: () => void;
    private receivedCallback?: (response: MjvmDataResponse) => void;

    public constructor() {
        this.client = new net.Socket();
        this.requestStatusTask = undefined;

        this.client.on('connect', () => {
            this.requestStatusTask = setInterval(() => {
                if(!this.client.destroyed && this.client.connecting === false) {
                    this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_READ_STATUS])).then((resp) => {
                        if(resp && resp.cmd === MjvmClientDebugger.DBG_READ_STATUS && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK) {
                            const tmp = this.currentStatus;
                            this.currentStatus = resp.data[0];
                            if((this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP_SET) && (this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP)) {
                                this.currentStackFrames = undefined;
                                if(this.stopCallback) {
                                    let reason = undefined;
                                    if(this.currentStatus & MjvmClientDebugger.DBG_STATUS_EXCP)
                                        reason = 'exception';
                                    this.stopCallback(reason);
                                }
                            }
                            else if((tmp & MjvmClientDebugger.DBG_STATUS_STOP) !== (this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP)) {
                                this.currentStackFrames = undefined;
                                if(this.stopCallback && (this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP))
                                    this.stopCallback();
                            }
                        }
                    });
                }
            }, 100);
        });

        this.client.on('data', (data: Buffer) => {
            if(this.receivedCallback) {
                if(!this.rxResponse) {
                    const cmd = data[0] & 0x7F;
                    const dataLength = data[1] | (data[2] << 8) | (data[3] << 16);
                    const responseCode = data[4];
                    this.rxResponse = new MjvmDataResponse(cmd, responseCode, dataLength);
                    if(cmd === MjvmClientDebugger.DBG_READ_STACK_TRACE)
                        this.rxResponse.receivedLength = 0;
                    let index = 0;
                    for(let i = 0; i < (data.length - 5); i++)
                        this.rxResponse.data[index++] = data[i + 5];
                    this.rxResponse.receivedLength = index;
                }
                else {
                    let index = this.rxResponse.receivedLength;
                    for(let i = 0; i < data.length; i++)
                        this.rxResponse.data[index++] = data[i];
                    this.rxResponse.receivedLength = index;
                }
                if(this.rxResponse.receivedLength >= this.rxResponse.data.length) {
                    this.receivedCallback(this.rxResponse);
                    this.rxResponse = undefined;
                    this.receivedCallback = undefined;
                }
            }
        });

        this.client.on('error', (err) => {
            if(this.errorCallback)
                this.errorCallback();
        });

        this.client.on('close', () => {
            if(this.closeCallback) {
                if(this.requestStatusTask) {
                    clearInterval(this.requestStatusTask);
                    this.requestStatusTask = undefined;
                }
                this.closeCallback();
            }
        });
    }

    private onReceived(callback: (response: MjvmDataResponse) => void) {
        this.receivedCallback = callback;
    }

    public onStop(callback: (reason?: string) => void) {
        this.stopCallback = callback;
    }

    public onError(callback: () => void) {
        this.errorCallback = callback;
    }

    public onClose(callback: () => void) {
        this.closeCallback = callback;
    }

    public async connect() {
        await this.client.connect(5555, '127.0.0.1');
    }

    private sendCmd(data: Buffer): Thenable<MjvmDataResponse | undefined> {
        return new Promise((resolve) => {
            this.tcpSemaphore.acquire().then(() => {
                const timeout = setTimeout(() => {
                    this.tcpSemaphore.release();
                    resolve(undefined);
                }, MjvmClientDebugger.TCP_RECEIVED_TIMEOUT);
                this.onReceived((resp) => {
                    this.tcpSemaphore.release();
                    clearTimeout(timeout);
                    resolve(resp);
                });
                if(!this.client.write(data)) {
                    this.tcpSemaphore.release();
                    clearTimeout(timeout);
                    resolve(undefined);
                }
            });
        });
    }

    public run(): Thenable<boolean> {
        this.currentStackFrames = undefined;
        return new Promise((resolve) => {
            if(!(this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP))
                resolve(true);
            else this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_RUN])).then((resp) => {
                if(resp && resp.cmd === MjvmClientDebugger.DBG_RUN && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)
                    resolve(true);
                else
                    resolve(false);
            });
        });
    }

    public stop(): Thenable<boolean> {
        this.currentStackFrames = undefined;
        return new Promise((resolve) => {
            if(this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP)
                resolve(true);
            else this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_STOP])).then((resp) => {
                if(!(resp && resp.cmd === MjvmClientDebugger.DBG_STOP && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK))
                    resolve(false);
                else
                    resolve(true);
            });
        });
    }

    private calcCrc(str: string): number {
        let crc: number = 0;
        for(let i = 0; i < str.length; i++)
            crc += str.charCodeAt(i);
        return crc;
    }

    private putConstUtf8ToBuffer(buff: Buffer, str: string, offset: number): number {
        buff[offset++] = (str.length >>> 0) & 0xFF;
        buff[offset++] = (str.length >>> 8) & 0xFF;
        const crc = this.calcCrc(str);
        buff[offset++] = (crc >>> 0) & 0xFF;
        buff[offset++] = (crc >>> 8) & 0xFF;
        const data = Buffer.from(str);
        data.copy(buff, offset);
        return offset + data.length + 1;
    }

    public removeAllBreakPoints(): Thenable<boolean> {
        return new Promise((resolve) => {
            this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_REMOVE_ALL_BKP])).then((resp) => {
                if(resp && resp.cmd === MjvmClientDebugger.DBG_REMOVE_ALL_BKP && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)
                    resolve(true);
                else
                    resolve(false);
            });
        });
    }

    private getRemoveBreakpointList(lines: number[], source: string): MjvmLineInfo[] | undefined {
        const ret: MjvmLineInfo[] = [];
        for(let i = 0; i < this.currentBreakpoints.length; i++) {
            if(source === this.currentBreakpoints[i].sourcePath) {
                let isContain = false;
                for(let j = 0; j < lines.length; j++) {
                    if(this.currentBreakpoints[i].line === lines[j]) {
                        isContain = true;
                        break;
                    }
                }
                if(!isContain)
                    ret.push(this.currentBreakpoints[i]);
            }
        }
        return ret;
    }

    private getAddBreakpointList(lines: number[], source: string): MjvmLineInfo[] | undefined {
        const ret: MjvmLineInfo[] = [];
        for(let i = 0; i < lines.length; i++) {
            let isContain = false;
            for(let j = 0; j < this.currentBreakpoints.length; j++) {
                if(source === this.currentBreakpoints[j].sourcePath && this.currentBreakpoints[j].line === lines[i]) {
                    isContain = true;
                    break;
                }
            }
            if(!isContain) {
                const lineInfo = MjvmLineInfo.getLineInfoFromLine(lines[i], source);
                if(lineInfo)
                    ret.push(lineInfo);
                else
                    return undefined;
            }
        }
        return ret;
    }

    private removeBreakPoints(lineInfo: MjvmLineInfo[]): Thenable<boolean> {
        return new Promise((resolve) => {
            const sendRemoveBkpTask = () => {
                const line = lineInfo.shift();
                if(line) {
                    let bufferSize = 1 + 4;
                    const className = line.classLoader.thisClass.replace(/\\/g, '/');
                    const methodName = line.methodInfo.name;
                    const descriptor = line.methodInfo.descriptor;
                    bufferSize += 4 + className.length + 1;
                    bufferSize += 4 + methodName.length + 1;
                    bufferSize += 4 + descriptor.length + 1;

                    const txBuff = Buffer.alloc(bufferSize);
                    let index = 0;

                    /* command code */
                    txBuff[index++] = MjvmClientDebugger.DBG_REMOVE_BKP;

                    /* pc value */
                    txBuff[index++] = (line.pc >>> 0) & 0xFF;
                    txBuff[index++] = (line.pc >>> 8) & 0xFF;
                    txBuff[index++] = (line.pc >>> 16) & 0xFF;
                    txBuff[index++] = (line.pc >>> 24) & 0xFF;

                    /* class name */
                    index = this.putConstUtf8ToBuffer(txBuff, className, index);

                    /* method name */
                    index = this.putConstUtf8ToBuffer(txBuff, methodName, index);

                    /* descriptor */
                    index = this.putConstUtf8ToBuffer(txBuff, descriptor, index);

                    this.sendCmd(txBuff).then((resp) => {
                        if(resp && resp.cmd === MjvmClientDebugger.DBG_REMOVE_BKP && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK) {
                            const index = this.currentBreakpoints.findIndex(item => item === line);
                            this.currentBreakpoints.splice(index, 1);
                            sendRemoveBkpTask();
                        }
                        else
                            resolve(false);
                    });
                }
                else
                    resolve(true);
            };
            sendRemoveBkpTask();
        });
    }

    private addBreakPoints(lineInfo: MjvmLineInfo[]): Thenable<boolean> {
        return new Promise((resolve) => {
            const sendAddBkpTask = () => {
                const line = lineInfo.shift();
                if(line) {
                    let bufferSize = 1 + 4;
                    const className = line.classLoader.thisClass.replace(/\\/g, '/');
                    const methodName = line.methodInfo.name;
                    const descriptor = line.methodInfo.descriptor;
                    bufferSize += 4 + className.length + 1;
                    bufferSize += 4 + methodName.length + 1;
                    bufferSize += 4 + descriptor.length + 1;

                    const txBuff = Buffer.alloc(bufferSize);
                    let index = 0;

                    /* command code */
                    txBuff[index++] = MjvmClientDebugger.DBG_ADD_BKP;

                    /* pc value */
                    txBuff[index++] = (line.pc >>> 0) & 0xFF;
                    txBuff[index++] = (line.pc >>> 8) & 0xFF;
                    txBuff[index++] = (line.pc >>> 16) & 0xFF;
                    txBuff[index++] = (line.pc >>> 24) & 0xFF;

                    /* class name */
                    index = this.putConstUtf8ToBuffer(txBuff, className, index);

                    /* method name */
                    index = this.putConstUtf8ToBuffer(txBuff, methodName, index);

                    /* descriptor */
                    index = this.putConstUtf8ToBuffer(txBuff, descriptor, index);

                    this.sendCmd(txBuff).then((resp) => {
                        if(resp && resp.cmd === MjvmClientDebugger.DBG_ADD_BKP && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK) {
                            this.currentBreakpoints.push(line);
                            sendAddBkpTask();
                        }
                        else
                            resolve(false);
                    });
                }
                else
                    resolve(true);
            };
            sendAddBkpTask();
        });
    }

    public setExceptionBreakPointsRequest(isEnabled: boolean): Thenable<boolean> {
        return new Promise((resolve) => {
            this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_SET_EXCP_MODE, isEnabled ? 1 : 0], )).then((resp) => {
                if(resp && resp.cmd === MjvmClientDebugger.DBG_SET_EXCP_MODE && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)
                    resolve(true);
                else
                    resolve(false);
            });
        });
    }

    public readExceptionInfo(): Thenable<MjvmExceptionInfo | undefined> {
        return new Promise((resolve) => {
            this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_READ_EXCP_INFO])).then((resp) => {
                if(resp && resp.cmd === MjvmClientDebugger.DBG_READ_EXCP_INFO && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK) {
                    let index = 0;
                    const typeLength = this.readU16(resp.data, index);
                    index += 4;
                    const type = resp.data.toString('utf-8', index, index + typeLength);
                    index += typeLength + 1;
                    const messageLength = this.readU16(resp.data, index);
                    index += 4;
                    const message = resp.data.toString('utf-8', index, index + messageLength);
                    resolve(new MjvmExceptionInfo(type, message));
                }
                else
                    resolve(undefined);
            });
        });
    }

    public setBreakPointsRequest(lines: number[], source: string): Thenable<boolean> {
        return new Promise((resolve) => {
            let bkps = this.getRemoveBreakpointList(lines, source);
            if(bkps === undefined) {
                resolve(false);
                return;
            }
            else if(bkps.length > 0) {
                this.removeBreakPoints(bkps).then((value) => {
                    if(!value) {
                        resolve(false);
                        return;
                    }
                });
            }
            bkps = this.getAddBreakpointList(lines, source);
            if(bkps === undefined) {
                resolve(false);
                return;
            }
            else if(bkps.length > 0) {
                this.addBreakPoints(bkps).then((value) => {
                    if(!value) {
                        resolve(false);
                        return;
                    }
                });
            }
            resolve(true);
        });
    }

    private readStackFrame(frameId: number): Thenable<MjvmStackFrame | undefined> {
        return new Promise((resolve) => {
            const txData: Buffer = Buffer.alloc(5);
            txData[0] = MjvmClientDebugger.DBG_READ_STACK_TRACE;
            txData[1] = frameId & 0xFF;
            txData[2] = (frameId >>> 8) & 0xFF;
            txData[3] = (frameId >>> 16) & 0xFF;
            txData[4] = (frameId >>> 24) & 0xFF;
            this.sendCmd(txData).then((resp) => {
                if(resp && resp.cmd === MjvmClientDebugger.DBG_READ_STACK_TRACE && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK) {
                    let index = 0;
                    const currentStack = this.readU32(resp.data, index);
                    const currentStackIndex = currentStack & 0x7FFFFFFF;
                    const isEndStack = (currentStack & 0x80000000) ? true : false;
                    if(currentStackIndex !== frameId) {
                        resolve(undefined);
                        return;
                    }
                    index += 4;
                    const pc = this.readU32(resp.data, index);
                    index += 4;
                    const classNameLength = this.readU16(resp.data, index);
                    index += 2 + 2;
                    const className = resp.data.toString('utf-8', index, index + classNameLength);
                    index += classNameLength + 1;
                    const nameLength = this.readU16(resp.data, index);
                    index += 2 + 2;
                    const name = resp.data.toString('utf-8', index, index + nameLength);
                    index += nameLength + 1;
                    const descriptorLength = this.readU16(resp.data, index);
                    index += 2 + 2;
                    const descriptor = resp.data.toString('utf-8', index, index + descriptorLength);

                    const lineInfo = MjvmLineInfo.getLineInfoFromPc(pc, className, name, descriptor);
                    if(lineInfo) {
                        const methodInfo = lineInfo.methodInfo;
                        let localVar = undefined;
                        if(methodInfo.attributeCode) {
                            const localVarAttr = methodInfo.attributeCode.getLocalVariables();
                            if(localVarAttr) {
                                localVar = [];
                                for(let i = 0; i < localVarAttr.localVariables.length; i++) {
                                    const tmp = localVarAttr.localVariables[i];
                                    if(tmp.startPc <= pc && pc < (tmp.startPc + tmp.length))
                                        localVar.push(tmp);
                                }
                                if(localVar.length === 0)
                                    localVar = undefined;
                            }
                        }
                        resolve(new MjvmStackFrame(frameId, lineInfo, isEndStack, localVar));
                        return;
                    }
                }
                resolve(undefined);
            });
        });
    }

    private stepRequest(stepCmd: number, stepCodeLength: number): Thenable<boolean> {
        this.currentStackFrames = undefined;
        return new Promise((resolve) => {
            const txData = Buffer.alloc(5);
            txData[0] = stepCmd;
            txData[1] = stepCodeLength & 0xFF;
            txData[2] = (stepCodeLength >>> 8) & 0xFF;
            txData[3] = (stepCodeLength >>> 16) & 0xFF;
            txData[4] = (stepCodeLength >>> 24) & 0xFF;
            this.sendCmd(Buffer.from(txData)).then((resp) => {
                if(!(resp && resp.cmd === stepCmd && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK))
                    resolve(false);
                else
                    resolve(true);
            });
        });
    }

    public stepInRequest(): Thenable<boolean> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                this.stepRequest(MjvmClientDebugger.DBG_STEP_IN, this.currentStackFrames[0].lineInfo.codeLength).then((value) => resolve(value));
            else this.readStackFrame(0).then((currentPoint) => {
                if(!currentPoint)
                    resolve(false);
                else
                    this.stepRequest(MjvmClientDebugger.DBG_STEP_IN, currentPoint.lineInfo.codeLength).then((value) => resolve(value));
            });
        });
    }

    public stepOverRequest(): Thenable<boolean> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                this.stepRequest(MjvmClientDebugger.DBG_STEP_OVER, this.currentStackFrames[0].lineInfo.codeLength).then((value) => resolve(value));
            else this.readStackFrame(0).then((currentPoint) => {
                if(!currentPoint)
                    resolve(false);
                else
                    this.stepRequest(MjvmClientDebugger.DBG_STEP_OVER, currentPoint.lineInfo.codeLength).then((value) => resolve(value));
            });
        });
    }

    public stepOutRequest(): Thenable<boolean> {
        return this.stepRequest(MjvmClientDebugger.DBG_STEP_OUT, 0);
    }

    private getSimpleNames(name: string): string[] {
        const ret: string[] = [];
        let index = 0;
        let simpleName = '';
        while(index < name.length) {
            let arrayCount = 0;
            let ch = name.charAt(index++);
            while(ch === '[') {
                arrayCount++;
                ch = name.charAt(index++);
            }
            if(ch !== 'L') {
                if(ch === 'Z')
                    simpleName = "boolean";
                else if(ch === 'C')
                    simpleName = "char";
                else if(ch === 'F')
                    simpleName = "float";
                else if(ch === 'D')
                    simpleName = "double";
                else if(ch === 'B')
                    simpleName = "byte";
                else if(ch === 'S')
                    simpleName = "short";
                else if(ch === 'I')
                    simpleName = "int";
                else if(ch === 'J')
                    simpleName = "long";
                else
                    simpleName = ch;
            }
            else {
                ch = name.charAt(index++);
                while(ch !== ';') {
                    simpleName += ch;
                    ch = name.charAt(index++);
                }
            }
            if(arrayCount > 0)
                simpleName = simpleName.concat("[]".repeat(arrayCount));
            ret.push(simpleName);
        }
        return ret;
    }

    private getShortenName(name: string): string {
        let dotIndexLastIndex = name.lastIndexOf('\/');
        if(dotIndexLastIndex < 0)
            dotIndexLastIndex = name.lastIndexOf('.');
        if(dotIndexLastIndex >= 0)
            return name.substring(dotIndexLastIndex + 1);
        return name;
    }

    private convertToStackFrame(stackFrames: MjvmStackFrame[]): StackFrame[] {
        const ret: StackFrame[] = [];
        for(let i = 0; i < stackFrames.length; i++) {
            const lineInfo = stackFrames[i].lineInfo;
            const src = new Source(lineInfo.classLoader.thisClass + ".java", lineInfo.sourcePath);
            let methodName = lineInfo.classLoader.thisClass;
            let dotIndexLastIndex = methodName.lastIndexOf('\/');
            dotIndexLastIndex = (dotIndexLastIndex < 0) ? 0 : (dotIndexLastIndex + 1);
            methodName = methodName.substring(dotIndexLastIndex, methodName.length);
            methodName += '.' + lineInfo.methodInfo.name + '(';
            const descriptor = lineInfo.methodInfo.descriptor;
            const names = this.getSimpleNames(descriptor.substring(1, descriptor.lastIndexOf(')')));
            for(let i = 0; i < names.length; i++) {
                methodName += this.getShortenName(names[i]);
                if((i + 1) < names.length)
                    methodName += ', ';
            }
            methodName += ')';
            const sf = new StackFrame(stackFrames[i].frameId, methodName, src, lineInfo.line);
            sf.instructionPointerReference = lineInfo.pc.toString();
            ret.push(sf);
        }
        return ret;
    }

    public stackFrameRequest(): Thenable<StackFrame[] | undefined> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                resolve(this.convertToStackFrame(this.currentStackFrames));
            else {
                const ret: MjvmStackFrame[] = [];
                const readStackFrameTask = (frameId: number) => this.readStackFrame(frameId).then((stackFrame) => {
                    if(stackFrame && stackFrame.lineInfo.sourcePath) {
                        ret.push(stackFrame);
                        if(stackFrame.isEndFrame) {
                            this.currentStackFrames = ret;
                            resolve(this.convertToStackFrame(this.currentStackFrames));
                        }
                        else
                            readStackFrameTask(frameId + 1);
                    }
                    else
                        resolve(undefined);
                });
                readStackFrameTask(0);
            }
        });
    }

    private isPrimType(descriptor: string): boolean {
        if(descriptor.length === 1) {
            switch(descriptor) {
                case 'Z':
                case 'C':
                case 'F':
                case 'D':
                case 'B':
                case 'S':
                case 'I':
                case 'J':
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }

    private isBooleanType(descriptor: string): boolean {
        if(descriptor.length === 1) {
            switch(descriptor) {
                case 'Z':
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }

    private isArrayType(descriptor: string): boolean {
        if(descriptor.length >= 1) {
            switch(descriptor.charAt(0)) {
                case '[':
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }

    private getElementTypeSize(arrayDescriptor: string): number {
        let index = 0;
        if(arrayDescriptor.charAt(index) === '[')
            index++;
        switch(arrayDescriptor.charAt(index)) {
            case 'Z':
            case 'B':
                return 1;
            case 'C':
            case 'S':
                return 2;
            case 'J':
            case 'D':
                return 8;
            default:
                return 4;
        }
    }

    private binaryToFloat32(binary: number): number {
        const buffer = new ArrayBuffer(4);
        const view = new DataView(buffer);
        view.setUint32(0, binary);
        return view.getFloat32(0);
    }

    private binaryToFloat64(binary: bigint): number {
        const buffer = new ArrayBuffer(8);
        const view = new DataView(buffer);
        view.setUint32(0, Number(binary >> 32n));
        view.setUint32(4, Number(binary >> 0xFFFFFFFFn));
        return view.getFloat64(0);
    }

    private readLocal(stackFrame: MjvmStackFrame, variable: number | string): Thenable<undefined | MjvmValueInfo> {
        return new Promise((resolve) => {
            const localVariableInfo = stackFrame.getLocalVariableInfo(variable);
            if(!localVariableInfo) {
                resolve(undefined);
                return;
            }
            const isReadU64 = localVariableInfo.descriptor === 'J' || localVariableInfo.descriptor === 'D'
            const txBuff = Buffer.alloc(10);
            txBuff[0] = MjvmClientDebugger.DBG_READ_LOCAL;
            txBuff[1] = isReadU64 ? 1 : 0;
            txBuff[2] = (stackFrame.frameId >>> 0) & 0xFF;
            txBuff[3] = (stackFrame.frameId >>> 8) & 0xFF;
            txBuff[4] = (stackFrame.frameId >>> 16) & 0xFF;
            txBuff[5] = (stackFrame.frameId >>> 24) & 0xFF;
            txBuff[6] = (localVariableInfo.index >>> 0) & 0xFF;
            txBuff[7] = (localVariableInfo.index >>> 8) & 0xFF;
            txBuff[8] = (localVariableInfo.index >>> 16) & 0xFF;
            txBuff[9] = (localVariableInfo.index >>> 24) & 0xFF;
            this.sendCmd(txBuff).then((resp) => {
                if(!(resp && resp.cmd === MjvmClientDebugger.DBG_READ_LOCAL && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)) {
                    resolve(undefined);
                    return;
                }
                const size = this.readU32(resp.data, 0);
                let value = isReadU64 ? this.readU64(resp.data, 4) : this.readU32(resp.data, 4);
                if(this.isPrimType(localVariableInfo.descriptor)) {
                    if(localVariableInfo.descriptor === 'F')
                        value = this.binaryToFloat32(value as number);
                    else if(localVariableInfo.descriptor === 'D')
                        value = this.binaryToFloat64(value as bigint);
                    resolve(new MjvmValueInfo(value, localVariableInfo.descriptor, size));
                }
                else {
                    let type: string;
                    if(!isReadU64 && resp.receivedLength > 13) {
                        const typeLength = this.readU16(resp.data, 8);
                        type = resp.data.toString('utf-8', 12, 12 + typeLength);
                    }
                    else
                        type = localVariableInfo.descriptor;
                    resolve(new MjvmValueInfo(value, type, size));
                }
            });
        });
    }

    public readLocalVariable(frameId: number): Thenable<Variable[] | undefined> {
        return new Promise((resolve) => {
            const ret: Variable[] = [];
            const readLocalTask = (stackFrame: MjvmStackFrame, variableIndex: number) => {
                if(stackFrame.localVariables && variableIndex < stackFrame.localVariables.length) {
                    const variableName = stackFrame.localVariables[variableIndex].name;
                    const localVariablesLength = stackFrame.localVariables.length;
                    this.readLocal(stackFrame, variableIndex).then((result) => {
                        if(result !== undefined) {
                            if(this.isPrimType(result.type)) {
                                const value = result.value;
                                if(this.isBooleanType(result.type))
                                    ret.push({name: variableName, value: (value === 0) ? 'false' : 'true', variablesReference: 0});
                                else if(result.type === 'C')
                                    ret.push({name: variableName, value: '\'' + String.fromCharCode(value as number) + '\'', variablesReference: 0});
                                else
                                    ret.push({name: variableName, value: value.toString(), variablesReference: 0});
                            }
                            else {
                                if(result.value === 0)
                                    ret.push({name: variableName, value: 'null', variablesReference: 0});
                                else {
                                    let type = this.getSimpleNames(result.type)[0];
                                    type = this.getShortenName(type);
                                    if(this.isArrayType(result.type)) {
                                        const arrayLength = result.size / this.getElementTypeSize(result.type);
                                        type = type.replace('[]', '[' + arrayLength + ']');
                                    }
                                    ret.push({name: variableName, value: type, variablesReference: result.value as number});
                                }
                            }
                            variableIndex++;
                            if(variableIndex < localVariablesLength)
                                readLocalTask(stackFrame, variableIndex);
                            else
                                resolve(ret);
                        }
                        else
                            resolve(undefined);
                    });
                }
                else
                    resolve(undefined);
            };
            if(this.currentStackFrames && this.currentStackFrames.length > frameId)
                readLocalTask(this.currentStackFrames[frameId], 0);
            else this.readStackFrame(frameId).then((stackFrame) => {
                if(stackFrame)
                    readLocalTask(stackFrame, 0);
                else
                    resolve(undefined);
            });
        });
    }

    public disconnect() {
        this.currentStackFrames = undefined;
        this.currentStatus = MjvmClientDebugger.DBG_STATUS_STOP;
        if(this.requestStatusTask) {
            clearInterval(this.requestStatusTask);
            this.requestStatusTask = undefined;
        }
        this.client.end();
    }

    private readU16(data: Buffer, offset : number): number {
        let ret = data[offset];
        ret |= data[offset + 1] << 8;
        return ret;
    }

    private readU32(data: Buffer, offset : number): number {
        let ret = data[offset];
        ret |= data[offset + 1] << 8;
        ret |= data[offset + 2] << 16;
        ret |= data[offset + 3] << 24;
        return ret >>> 0;
    }

    private readU64(data: Buffer, offset : number): bigint {
        let ret = BigInt(data[offset]);
        ret |= BigInt(data[offset + 1]) << 8n;
        ret |= BigInt(data[offset + 2]) << 16n;
        ret |= BigInt(data[offset + 3]) << 24n;
        ret |= BigInt(data[offset + 4]) << 32n;
        ret |= BigInt(data[offset + 5]) << 40n;
        ret |= BigInt(data[offset + 6]) << 48n;
        ret |= BigInt(data[offset + 7]) << 56n;
        return ret;
    }
}
