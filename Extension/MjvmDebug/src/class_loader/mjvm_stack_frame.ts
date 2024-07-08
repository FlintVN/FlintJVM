
import { MjvmLineInfo } from './mjvm_line_info'
import { LocalVariable } from './mjvm_attribute_info'

export class MjvmStackFrame {
    public readonly frameId: number;
    public readonly lineInfo: MjvmLineInfo;
    public readonly isEndFrame: boolean;
    public readonly localVariables?: LocalVariable[];

    public constructor(frameId: number, lineInfo: MjvmLineInfo, isEndFrame: boolean, localVariables: LocalVariable[] | undefined) {
        this.frameId = frameId;
        this.lineInfo = lineInfo;
        this.isEndFrame = isEndFrame;
        this.localVariables = localVariables;
    }

    public getLocalVariableInfo(name: string): LocalVariable | undefined {
        if(this.localVariables) {
            for(let i = 0; i < this.localVariables.length; i++) {
                if(this.localVariables[i].name === name)
                    return this.localVariables[i];
            }
        }
        return undefined;
    }
}
