
import { MjvmLineInfo } from './mjvm_line_info'
import { MjvmLocalVariable } from './mjvm_attribute_info'

export class MjvmStackFrame {
    public readonly frameId: number;
    public readonly lineInfo: MjvmLineInfo;
    public readonly isEndFrame: boolean;
    public readonly localVariables?: MjvmLocalVariable[];

    public constructor(frameId: number, lineInfo: MjvmLineInfo, isEndFrame: boolean, localVariables: MjvmLocalVariable[] | undefined) {
        this.frameId = frameId;
        this.lineInfo = lineInfo;
        this.isEndFrame = isEndFrame;
        this.localVariables = localVariables;
    }

    public getLocalVariableInfo(variable: number | string): MjvmLocalVariable | undefined {
        if(this.localVariables) {
            if(typeof variable === 'number') {
                if(variable < this.localVariables.length)
                    return this.localVariables[variable];
                return undefined;
            }
            else {
                for(let i = 0; i < this.localVariables.length; i++) {
                    if(this.localVariables[i].name === variable)
                        return this.localVariables[i];
                }
            }
        }
        return undefined;
    }
}
