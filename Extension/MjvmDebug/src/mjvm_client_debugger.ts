
import {
    StackFrame, Source
} from '@vscode/debugadapter';
import * as vscode from 'vscode';
import * as net from 'net';
import { clear } from 'console';
import path = require('path');
import { Semaphore } from './mjvm_semaphone'
import { DebugProtocol } from '@vscode/debugprotocol';
import { DebugLineInfo } from './class_loader/mjvm_debug_line_info'

export class MjvmClientDebugger {
    private readonly client: net.Socket;
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
    private static readonly DBG_READ_VARIABLE: number = 11;
    private static readonly DBG_WRITE_VARIABLE: number = 12;

    private static readonly DBG_STATUS_STOP: number = 0x01;
    private static readonly DBG_STATUS_STOP_SET: number = 0x02;
    private static readonly DBG_STATUS_STEP_IN: number = 0x04;
    private static readonly DBG_STATUS_STEP_OVER: number = 0x04;
    private static readonly DBG_STATUS_STEP_OUT: number = 0x04;

    private static TCP_RECEIVED_TIMEOUT: number = 100;

    private requestStatusTask?: NodeJS.Timeout;

    private currentStatus: number = MjvmClientDebugger.DBG_STATUS_STOP;
    private currentStackFrames?: DebugLineInfo[];
    private currentBreakpoints: DebugLineInfo[] = [];

    private tcpSemaphore = new Semaphore(1);

    private stopCallback?: () => void;
    private errorCallback?: () => void;
    private closeCallback?: () => void;
    private receivedCallback?: (data: Buffer) => void;

    public constructor() {
        this.client = new net.Socket();

        this.requestStatusTask = undefined;

        this.client.on('connect', () => {
            this.requestStatusTask = setInterval(() => {
                if(!this.client.destroyed && this.client.connecting === false) {
                    this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_READ_STATUS])).then((data) => {
                        if(data && data[0] === MjvmClientDebugger.DBG_READ_STATUS && data[1] === 0) {
                            const tmp = this.currentStatus;
                            this.currentStatus = data[2];
                            if((data[2] & MjvmClientDebugger.DBG_STATUS_STOP_SET) && (data[2] & MjvmClientDebugger.DBG_STATUS_STOP)) {
                                this.currentStackFrames = undefined;
                                if(this.stopCallback)
                                    this.stopCallback();
                            }
                            else if((tmp & MjvmClientDebugger.DBG_STATUS_STOP) !== (data[2] & MjvmClientDebugger.DBG_STATUS_STOP)) {
                                this.currentStackFrames = undefined;
                                if(this.stopCallback && (data[2] & MjvmClientDebugger.DBG_STATUS_STOP))
                                    this.stopCallback();
                            }
                        }
                    });
                }
            }, 100);
        });

        this.client.on('data', (data: Buffer) => {
            if(this.receivedCallback) {
                this.receivedCallback(data);
                this.receivedCallback = undefined;
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

    private onReceived(callback: (data: Buffer) => void) {
        this.receivedCallback = callback;
    }

    public onStop(callback: () => void) {
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

    private sendCmd(data: Buffer): Thenable<Buffer | null> {
        return new Promise((resolve) => {
            this.tcpSemaphore.acquire().then(() => {
                const timeout = setTimeout(() => {
                    this.tcpSemaphore.release();
                    resolve(null);
                }, MjvmClientDebugger.TCP_RECEIVED_TIMEOUT);
                this.onReceived((data) => {
                    this.tcpSemaphore.release();
                    clearTimeout(timeout);
                    resolve(data);
                });
                if(!this.client.write(data)) {
                    this.tcpSemaphore.release();
                    clearTimeout(timeout);
                    resolve(null);
                }
            });
        });
    }

    public run(): Thenable<boolean> {
        this.currentStackFrames = undefined;
        return new Promise((resolve) => {
            if(!(this.currentStatus & MjvmClientDebugger.DBG_STATUS_STOP))
                resolve(true);
            else this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_RUN])).then((data) => {
                if(data && data.length === 2 && data[0] === MjvmClientDebugger.DBG_RUN && data[1] === 0)
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
            else this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_STOP])).then((data) => {
                if(!(data && data.length === 2 && data[0] === MjvmClientDebugger.DBG_STOP && data[1] === 0))
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
            this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_REMOVE_ALL_BKP])).then((data) => {
                if(data && data[0] === MjvmClientDebugger.DBG_REMOVE_ALL_BKP && data[1] === 0)
                    resolve(true);
                else
                    resolve(false);
            });
        });
    }

    private getRemoveBreakpointList(lines: number[], source: string): DebugLineInfo[] | null {
        const ret: DebugLineInfo[] = [];
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

    private getAddBreakpointList(lines: number[], source: string): DebugLineInfo[] | null {
        const ret: DebugLineInfo[] = [];
        for(let i = 0; i < lines.length; i++) {
            let isContain = false;
            for(let j = 0; j < this.currentBreakpoints.length; j++) {
                if(source === this.currentBreakpoints[j].sourcePath && this.currentBreakpoints[j].line === lines[i]) {
                    isContain = true;
                    break;
                }
            }
            if(!isContain) {
                const lineInfo = DebugLineInfo.getLineInfoFromLine(lines[i], source);
                if(lineInfo)
                    ret.push(lineInfo);
                else
                    return null;
            }
        }
        return ret;
    }

    private removeBreakPoints(lineInfo: DebugLineInfo[]): Thenable<boolean> {
        return new Promise((resolve) => {
            const sendRemoveBkpTask = () => {
                const line = lineInfo.shift();
                if(line) {
                    let bufferSize = 1 + 4;
                    bufferSize += 4 + line.className.length + 1;
                    bufferSize += 4 + line.methodName.length + 1;
                    bufferSize += 4 + line.descriptor.length + 1;

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
                    const className = line.className.replace(/\\/g, '/');
                    index = this.putConstUtf8ToBuffer(txBuff, className, index);

                    /* method name */
                    index = this.putConstUtf8ToBuffer(txBuff, line.methodName, index);

                    /* descriptor */
                    index = this.putConstUtf8ToBuffer(txBuff, line.descriptor, index);

                    this.sendCmd(txBuff).then((data) => {
                        if(data && data[0] === MjvmClientDebugger.DBG_REMOVE_BKP && data[1] === 0) {
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

    private addBreakPoints(lineInfo: DebugLineInfo[]): Thenable<boolean> {
        return new Promise((resolve) => {
            const sendAddBkpTask = () => {
                const line = lineInfo.shift();
                if(line) {
                    let bufferSize = 1 + 4;
                    bufferSize += 4 + line.className.length + 1;
                    bufferSize += 4 + line.methodName.length + 1;
                    bufferSize += 4 + line.descriptor.length + 1;

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
                    const className = line.className.replace(/\\/g, '/');
                    index = this.putConstUtf8ToBuffer(txBuff, className, index);

                    /* method name */
                    index = this.putConstUtf8ToBuffer(txBuff, line.methodName, index);

                    /* descriptor */
                    index = this.putConstUtf8ToBuffer(txBuff, line.descriptor, index);

                    this.sendCmd(txBuff).then((data) => {
                        if(data && data[0] === MjvmClientDebugger.DBG_ADD_BKP && data[1] === 0) {
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
            this.sendCmd(Buffer.from([MjvmClientDebugger.DBG_SET_EXCP_MODE, isEnabled ? 1 : 0], )).then((data) => {
                if(data && data[0] === MjvmClientDebugger.DBG_SET_EXCP_MODE && data[1] === 0)
                    resolve(true);
                else
                    resolve(false);
            });
        });
    }

    public setBreakPointsRequest(lines: number[], source: string): Thenable<boolean> {
        return new Promise((resolve) => {
            let bkps = this.getRemoveBreakpointList(lines, source);
            if(bkps === null) {
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
            if(bkps === null) {
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

    private readStackFrame(stackIndex: number): Thenable<[boolean, DebugLineInfo] | null> {
        return new Promise((resolve) => {
            const txData: Buffer = Buffer.alloc(5);
            txData[0] = MjvmClientDebugger.DBG_READ_STACK_TRACE;
            txData[1] = stackIndex & 0xFF;
            txData[2] = (stackIndex >>> 8) & 0xFF;
            txData[3] = (stackIndex >>> 16) & 0xFF;
            txData[4] = (stackIndex >>> 24) & 0xFF;
            this.sendCmd(txData).then((data) => {
                if(data && data[0] === MjvmClientDebugger.DBG_READ_STACK_TRACE && data[1] === 0) {
                    let index = 2;
                    const currentStack = this.readU32(data, index);
                    const currentStackIndex = currentStack & 0x7FFFFFFF;
                    const isEndStack = (currentStack & 0x80000000) ? true : false;
                    if(currentStackIndex !== stackIndex) {
                        resolve(null);
                        return;
                    }
                    index += 4;
                    const pc = this.readU32(data, index);
                    index += 4;
                    const classNameLength = this.readU16(data, index);
                    index += 2 + 2;
                    const className = data.toString('utf-8', index, index + classNameLength);
                    index += classNameLength + 1;
                    const nameLength = this.readU16(data, index);
                    index += 2 + 2;
                    const name = data.toString('utf-8', index, index + nameLength);
                    index += nameLength + 1;
                    const descriptorLength = this.readU16(data, index);
                    index += 2 + 2;
                    const descriptor = data.toString('utf-8', index, index + descriptorLength);

                    const lineInfo = DebugLineInfo.getLineInfoFromPc(pc, className, name, descriptor);
                    if(lineInfo) {
                        resolve([isEndStack, lineInfo]);
                        return;
                    }
                }
                resolve(null);
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
            this.sendCmd(Buffer.from(txData)).then((data) => {
                if(!(data && data[0] === stepCmd && data[1] === 0))
                    resolve(false);
                else
                    resolve(true);
            });
        });
    }

    public stepInRequest(): Thenable<boolean> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                this.stepRequest(MjvmClientDebugger.DBG_STEP_IN, this.currentStackFrames[0].codeLength).then((value) => resolve(value));
            else this.readStackFrame(0).then((currentPoint) => {
                if(!currentPoint)
                    resolve(false);
                else
                    this.stepRequest(MjvmClientDebugger.DBG_STEP_IN, currentPoint[1].codeLength).then((value) => resolve(value));
            });
        });
    }

    public stepOverRequest(): Thenable<boolean> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                this.stepRequest(MjvmClientDebugger.DBG_STEP_OVER, this.currentStackFrames[0].codeLength).then((value) => resolve(value));
            else this.readStackFrame(0).then((currentPoint) => {
                if(!currentPoint)
                    resolve(false);
                else
                    this.stepRequest(MjvmClientDebugger.DBG_STEP_OVER, currentPoint[1].codeLength).then((value) => resolve(value));
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
            while(ch == '[') {
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
            const dotIndexLastIndex = simpleName.lastIndexOf('\/');
            if(dotIndexLastIndex >= 0)
                simpleName = simpleName.substring(dotIndexLastIndex + 1);
            ret.push(simpleName);
        }
        return ret;
    }

    private convertToStackFrame(linesInfo: DebugLineInfo[]): StackFrame[] {
        const ret: StackFrame[] = [];
        for(let i = 0; i < linesInfo.length; i++) {
            const src = new Source(linesInfo[i].className + ".java", linesInfo[i].sourcePath);
            let methodName = linesInfo[i].className;
            let dotIndexLastIndex = methodName.lastIndexOf('\/');
            dotIndexLastIndex = (dotIndexLastIndex < 0) ? 0 : (dotIndexLastIndex + 1);
            methodName = methodName.substring(dotIndexLastIndex, methodName.length);
            methodName += '.' + linesInfo[i].methodName + '(';
            const descriptor = linesInfo[i].descriptor;
            const names = this.getSimpleNames(descriptor.substring(1, descriptor.lastIndexOf(')')));
            for(let i = 0; i < names.length; i++) {
                methodName += names[i];
                if((i + 1) < names.length)
                    methodName += ', ';
            }
            methodName += ')';
            const sf = new StackFrame(i, methodName, src, linesInfo[i].line);
            sf.instructionPointerReference = linesInfo[i].pc.toString();
            ret.push(sf);
        }
        return ret;
    }

    public stackFrameRequest(): Thenable<StackFrame[] | null> {
        return new Promise((resolve) => {
            if(this.currentStackFrames)
                resolve(this.convertToStackFrame(this.currentStackFrames));
            else {
                const ret: DebugLineInfo[] = [];
                const readStackFrameTask = (stackIndex: number) => this.readStackFrame(stackIndex).then((lineInfo) => {
                    if(lineInfo && lineInfo[1].sourcePath) {
                        ret.push(lineInfo[1]);
                        if(lineInfo[0]) {
                            this.currentStackFrames = ret;
                            resolve(this.convertToStackFrame(this.currentStackFrames));
                        }
                        else
                            readStackFrameTask(stackIndex + 1);
                    }
                    else
                        resolve(null);
                });
                readStackFrameTask(0);
            }
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
        return ret;
    }
}
