
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
import { MjvmClassLoader } from './class_loader/mjvm_class_loader';
import { MjvmFieldInfo } from './class_loader/mjvm_field_info';

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
    private static readonly DBG_READ_FIELD: number = 14;
    private static readonly DBG_WRITE_FIELD: number = 15;
    private static readonly DBG_READ_ARRAY: number = 16;
    private static readonly DBG_READ_SIZE_AND_TYPE: number = 17;

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

    private variableReferenceMap = new Map<number, MjvmValueInfo>;

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

    private convertToVariable(valueInfos: MjvmValueInfo[]) : Variable[] {
        const ret: Variable[] = [];
        for(let i = 0; i < valueInfos.length; i++) {
            if(this.isPrimType(valueInfos[i].type))
                ret.push({name: valueInfos[i].name, value: valueInfos[i].value.toString(), variablesReference: 0});
            else {
                let value;
                if(typeof valueInfos[i].value === 'string')
                    value = valueInfos[i].value;
                else if(valueInfos[i].reference === 0)
                    value = 'null';
                else {
                    let type = this.getSimpleNames(valueInfos[i].type)[0];
                    type = this.getShortenName(type);
                    if(this.isArrayType(valueInfos[i].type)) {
                        const arrayLength = valueInfos[i].size / this.getElementTypeSize(valueInfos[i].type);
                        type = type.replace('[]', '[' + arrayLength + ']');
                    }
                    value = type;
                }
                ret.push({name: valueInfos[i].name, value: value.toString(), variablesReference: valueInfos[i].reference});
            }
        }
        return ret;
    }

    private addToRefMap(valueInfos: MjvmValueInfo[]) {
        for(let i = 0; i < valueInfos.length; i++) {
            if(!this.isPrimType(valueInfos[i].type)) {
                const reference = valueInfos[i].reference;
                if(reference !== 0) {
                    if(!this.variableReferenceMap.has(reference))
                        this.variableReferenceMap.set(reference, valueInfos[i]);
                }
            }
        }
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

    private readObjSizeAndType(reference: number): Thenable<[number, string] | undefined> {
        return new Promise((resolve) => {
            const txBuff = Buffer.alloc(5);
            txBuff[0] = MjvmClientDebugger.DBG_READ_SIZE_AND_TYPE;
            txBuff[1] = (reference >>> 0) & 0xFF;
            txBuff[2] = (reference >>> 8) & 0xFF;
            txBuff[3] = (reference >>> 16) & 0xFF;
            txBuff[4] = (reference >>> 24) & 0xFF;
            this.sendCmd(txBuff).then((resp) => {
                if(!(resp && resp.cmd === MjvmClientDebugger.DBG_READ_SIZE_AND_TYPE && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)) {
                    resolve(undefined);
                    return;
                }
                const size = this.readU32(resp.data, 0);
                const typeLength = this.readU16(resp.data, 4);
                const typeName = resp.data.toString('utf-8', 8, 8 + typeLength);
                resolve([size, typeName]);
            });
        });
    }

    private readStringValue(strObj: MjvmValueInfo): Thenable<string | undefined> {
        return new Promise((resolve) => {
            const clsName = this.getSimpleNames(strObj.type)[0];
            const clsPath = MjvmClassLoader.findClassFile(clsName);
            if(!clsPath) {
                resolve(undefined);
                return;
            }
            const clsLoader = MjvmClassLoader.load(clsPath);
            const fieldInfos = clsLoader.getFieldList(true);
            if(!fieldInfos) {
                resolve(undefined);
                return;
            }
            this.readFields(strObj.reference, fieldInfos).then((result) => {
                if(result === undefined) {
                    resolve(undefined);
                    return;
                }
                let tmp = result.find(u => u.name === 'coder');
                if(tmp === undefined) {
                    resolve(undefined);
                    return;
                }
                const coder = tmp.value as number;
                tmp = result.find(u => u.name === 'value');
                if(tmp === undefined) {
                    resolve(undefined);
                    return;
                }
                const value = tmp as MjvmValueInfo;
                if(coder === undefined || value === undefined) {
                    resolve(undefined);
                    return;
                }
                this.readArray(value.reference, 0, value.size, value.type).then((result) => {
                    if(result === undefined) {
                        resolve(undefined);
                        return;
                    }
                    const byteArray: number[] = [];
                    if(coder === 0) {
                        for(let i = 0; i < result.length; i++)
                            byteArray.push(result[i].value as number);
                    }
                    else for(let i = 0; i < result.length; i++) {
                        const low = result[i * 2 + 0].value as number;
                        const hight = result[i * 2 + 1].value as number;
                        byteArray.push(low | (hight << 8));
                    }
                    resolve(String.fromCharCode(...byteArray));
                });
            });
        });
    }

    private readLocal(stackFrame: MjvmStackFrame, variable: number | string): Thenable<MjvmValueInfo | undefined> {
        return new Promise((resolve) => {
            const localVariableInfo = stackFrame.getLocalVariableInfo(variable);
            if(!localVariableInfo) {
                resolve(undefined);
                return;
            }
            const isU64 = localVariableInfo.descriptor === 'J' || localVariableInfo.descriptor === 'D'
            const txBuff = Buffer.alloc(10);
            txBuff[0] = MjvmClientDebugger.DBG_READ_LOCAL;
            txBuff[1] = isU64 ? 1 : 0;
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
                let value: number | bigint | string = isU64 ? this.readU64(resp.data, 4) : this.readU32(resp.data, 4);
                const name = localVariableInfo.name;
                if(this.isPrimType(localVariableInfo.descriptor)) {
                    if(localVariableInfo.descriptor === 'F')
                        value = this.binaryToFloat32(value as number);
                    else if(localVariableInfo.descriptor === 'D')
                        value = this.binaryToFloat64(value as bigint);
                    else if(localVariableInfo.descriptor === 'C')
                        value = '\'' + String.fromCharCode(value as number) + '\'';
                    else if(localVariableInfo.descriptor === 'Z')
                        value = (value === 0) ? 'false' : 'true';
                    resolve(new MjvmValueInfo(name, localVariableInfo.descriptor, value, size, 0));
                }
                else {
                    let type: string;
                    if(!isU64 && resp.receivedLength > 13) {
                        const typeLength = this.readU16(resp.data, 8);
                        type = resp.data.toString('utf-8', 12, 12 + typeLength);
                    }
                    else
                        type = localVariableInfo.descriptor;
                    const reference = value as number;
                    resolve(new MjvmValueInfo(name, type, reference ? 0 : 'null', size, reference));
                }
            });
        });
    }

    private readField(reference: number, fieldInfo: MjvmFieldInfo): Thenable<MjvmValueInfo | undefined> {
        return new Promise((resolve) => {
            const txBuff = Buffer.alloc(5 + 4 + fieldInfo.name.length + 1);
            txBuff[0] = MjvmClientDebugger.DBG_READ_FIELD;
            txBuff[1] = (reference >>> 0) & 0xFF;
            txBuff[2] = (reference >>> 8) & 0xFF;
            txBuff[3] = (reference >>> 16) & 0xFF;
            txBuff[4] = (reference >>> 24) & 0xFF;
            this.putConstUtf8ToBuffer(txBuff, fieldInfo.name, 5);
            this.sendCmd(txBuff).then((resp) => {
                if(!(resp && resp.cmd === MjvmClientDebugger.DBG_READ_FIELD && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)) {
                    resolve(undefined);
                    return;
                }
                const isU64 = fieldInfo.descriptor === 'J' || fieldInfo.descriptor === 'D';
                const size = this.readU32(resp.data, 0);
                let value: number | bigint | string = isU64 ? this.readU64(resp.data, 4) : this.readU32(resp.data, 4);
                const name = fieldInfo.name;
                if(this.isPrimType(fieldInfo.descriptor)) {
                    if(fieldInfo.descriptor === 'F')
                        value = this.binaryToFloat32(value as number);
                    else if(fieldInfo.descriptor === 'D')
                        value = this.binaryToFloat64(value as bigint);
                    else if(fieldInfo.descriptor === 'C')
                        value = '\'' + String.fromCharCode(value as number) + '\'';
                    else if(fieldInfo.descriptor === 'Z')
                        value = (value === 0) ? 'false' : 'true';
                    resolve(new MjvmValueInfo(name, fieldInfo.descriptor, value, size, 0));
                }
                else {
                    let type: string;
                    if(!isU64 && resp.receivedLength > 13) {
                        const typeLength = this.readU16(resp.data, 8);
                        type = resp.data.toString('utf-8', 12, 12 + typeLength);
                    }
                    else
                        type = fieldInfo.descriptor;
                    const reference = value as number;
                    resolve(new MjvmValueInfo(name, type, reference ? 0 : 'null', size, reference));
                }
            });
        });
    }

    private readFields(reference: number, fieldInfos: MjvmFieldInfo[]): Thenable<MjvmValueInfo[] | undefined> {
        return new Promise((resolve) => {
            const ret: MjvmValueInfo[] = [];
            const readFieldTask = () => {
                const fieldInfo = fieldInfos.shift() as MjvmFieldInfo;
                this.readField(reference, fieldInfo).then((result) => {
                    if(result !== undefined) {
                        ret.push(result);
                        if(fieldInfos.length > 0)
                            readFieldTask();
                        else
                            resolve(ret);
                    }
                    else {
                        resolve(undefined);
                        return;
                    }
                });
            }
            readFieldTask();
        });
    }

    private readArray(reference: number, index: number, length: number, arrayType: string): Thenable<MjvmValueInfo[] | undefined> {
        return new Promise((resolve) => {
            const txBuff = Buffer.alloc(12);
            txBuff[0] = MjvmClientDebugger.DBG_READ_ARRAY;
            txBuff[1] = (length >>> 0) & 0xFF;
            txBuff[2] = (length >>> 8) & 0xFF;
            txBuff[3] = (length >>> 16) & 0xFF;
            txBuff[4] = (index >>> 0) & 0xFF;
            txBuff[5] = (index >>> 8) & 0xFF;
            txBuff[6] = (index >>> 16) & 0xFF;
            txBuff[7] = (index >>> 24) & 0xFF;
            txBuff[8] = (reference >>> 0) & 0xFF;
            txBuff[9] = (reference >>> 8) & 0xFF;
            txBuff[10] = (reference >>> 16) & 0xFF;
            txBuff[11] = (reference >>> 24) & 0xFF;
            this.sendCmd(txBuff).then((resp) => {
                if(!(resp && resp.cmd === MjvmClientDebugger.DBG_READ_ARRAY && resp.responseCode === MjvmClientDebugger.DBG_RESP_OK)) {
                    resolve(undefined);
                    return;
                }
                const elementSize = this.getElementTypeSize(arrayType);
                const elementType = arrayType.substring(1);
                const actualLength = resp.data.length / elementSize;
                const ret: MjvmValueInfo[] = [];
                if(elementSize === 1) {
                    for(let i = 0; i < actualLength; i++) {
                        const name = '[' + i + ']';
                        const value = (elementType === 'Z') ? ((resp.data[i] === 0) ? 'false' : 'true') : resp.data[i];
                        ret.push(new MjvmValueInfo(name, elementType, value, 1, 0));
                    }
                    resolve(ret);
                }
                else if(elementSize === 2) {
                    let index = 0;
                    for(let i = 0; i < actualLength; i++) {
                        const name = '[' + i + ']';
                        let value: number | bigint | string = this.readU16(resp.data, index);
                        index += 2;
                        if(elementType === 'C')
                            value = '\'' + String.fromCharCode(value as number) + '\'';
                        '\'' + String.fromCharCode(value as number) + '\'';
                        ret.push(new MjvmValueInfo(name, elementType, value, 1, 0));
                    }
                    resolve(ret);
                }
                else if(elementSize === 4) {
                    let index = 0;
                    if(this.isPrimType(elementType)) {
                        for(let i = 0; i < actualLength; i++) {
                            const name = '[' + i + ']';
                            let value: number | bigint = this.readU32(resp.data, index);
                            if(elementType === 'F')
                                value = this.binaryToFloat32(value as number);
                            index += 4;
                            ret.push(new MjvmValueInfo(name, elementType, value, 4, 0));
                        }
                        resolve(ret);
                    }
                    else {
                        const referenceList: number[] = [];
                        for(let i = 0; i < actualLength; i++) {
                            referenceList.push(this.readU32(resp.data, index));
                            index += 4;
                        }
                        index = 0;
                        const readSizeAndTypeTask = () => {
                            const reference = referenceList[index];
                            this.readObjSizeAndType(reference).then((result) => { 
                                if(result === undefined) {
                                    resolve(undefined);
                                    return;
                                }
                                const name = '[' + index + ']';
                                const size = result[0];
                                const type = result[1];
                                ret.push(new MjvmValueInfo(name, type, reference ? 0 : 'null', size, reference));
                                index++;
                                if(index < referenceList.length)
                                    readSizeAndTypeTask();
                                else
                                    resolve(ret);
                            });
                        }
                        readSizeAndTypeTask();
                    }
                }
                else if(elementSize === 8) {
                    let index = 0;
                    for(let i = 0; i < actualLength; i++) {
                        const name = '[' + i + ']';
                        let value: number | bigint  = this.readU64(resp.data, index);
                        if(elementType === 'D')
                            value = this.binaryToFloat64(value as bigint);
                        index += 8;
                        ret.push(new MjvmValueInfo(name, elementType, value, 8, 0));
                    }
                    resolve(ret);
                }
            });
        });
    }

    public readVariable(reference: number): Thenable<Variable[] | undefined> {
        return new Promise((resolve) => {
            if(!this.variableReferenceMap.has(reference)) {
                resolve(undefined);
                return;
            }
            const valueInfo = this.variableReferenceMap.get(reference) as MjvmValueInfo;
            if(this.isPrimType(valueInfo.type)) {
                resolve(undefined);
                return;
            }
            if(!this.isArrayType(valueInfo.type)) {
                const clsName = this.getSimpleNames(valueInfo.type)[0];
                const clsPath = MjvmClassLoader.findClassFile(clsName);
                if(!clsPath) {
                    resolve(undefined);
                    return;
                }
                const clsLoader = MjvmClassLoader.load(clsPath);
                const fieldInfos = clsLoader.getFieldList(true);
                if(!fieldInfos) {
                    resolve(undefined);
                    return;
                }
                this.readFields(reference, fieldInfos).then((result) => {
                    if(result === undefined) {
                        resolve(undefined);
                        return;
                    }
                    this.addToRefMap(result);
                    resolve(this.convertToVariable(result));
                });
            }
            else {
                const length = valueInfo.size / this.getElementTypeSize(valueInfo.type);
                this.readArray(reference, 0, length, valueInfo.type).then((result) => {
                    if(result === undefined) {
                        resolve(undefined);
                        return;
                    }
                    this.addToRefMap(result);
                    resolve(this.convertToVariable(result));
                });
            }
        });
    }

    public readLocalVariables(frameId: number): Thenable<Variable[] | undefined> {
        return new Promise((resolve) => {
            this.variableReferenceMap.clear();
            const valueInfos: MjvmValueInfo[] = [];
            const readLocalTask = (stackFrame: MjvmStackFrame, variableIndex: number) => {
                if(stackFrame.localVariables && variableIndex < stackFrame.localVariables.length) {
                    const localVariablesLength = stackFrame.localVariables.length;
                    this.readLocal(stackFrame, variableIndex).then((result) => {
                        if(result === undefined) {
                            resolve(undefined);
                            return;
                        }
                        valueInfos.push(result);
                        variableIndex++;
                        if(variableIndex < localVariablesLength)
                            readLocalTask(stackFrame, variableIndex);
                        else {
                            this.addToRefMap(valueInfos);
                            resolve(this.convertToVariable(valueInfos));
                        }
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
