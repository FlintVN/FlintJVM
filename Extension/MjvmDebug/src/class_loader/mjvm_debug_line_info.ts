
import fs = require('fs');
import path = require('path');
import { ClassLoader } from './mjvm_class_loader';
import * as vscode from 'vscode';
import { MethodInfo } from './mjvm_method_info';
import { AttributeLineNumber } from './mjvm_attribute_info';

export class DebugLineInfo {
    public readonly pc: number;
    public readonly line: number;
    public readonly codeLength: number;
    public readonly methodName: string;
    public readonly descriptor: string;
    public readonly className: string;
    public readonly classPath: string;
    public readonly sourcePath: string;

    private static classLoaderDictionary: Record<string, ClassLoader> = {};

    private constructor(pc: number, line: number, codeLength: number, methodName: string, descriptor: string, clsName: string, clsPath: string, srcPath: string) {
        this.pc = pc;
        this.line = line;
        this.codeLength = codeLength;
        this.methodName = methodName;
        this.descriptor = descriptor;
        this.className = clsName;
        this.classPath = clsPath;
        this.sourcePath = srcPath;
    }

    private static load(classFile: string): ClassLoader {
        if(!(classFile in DebugLineInfo.classLoaderDictionary))
            DebugLineInfo.classLoaderDictionary[classFile] = new ClassLoader(classFile);
        return DebugLineInfo.classLoaderDictionary[classFile];
    }

    private static findSourceFile(name: string) : string | null {
        const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
        const srcPath: string = path.join(workspace, name) + ".java";
        if(fs.existsSync(srcPath))
            return srcPath;
        return null;
    }

    private static findClassFileFromName(name: string) : string | null {
        const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
        const srcPath: string = path.join(workspace, name) + ".class";
        if(fs.existsSync(srcPath))
            return srcPath;
        return null;
    }

    private static getClassNameFormSource(source: string) : string | null {
        const lastDotIndex = source.lastIndexOf('.');
        if(lastDotIndex < 0)
            return null;
        const extensionName = source.substring(lastDotIndex, source.length);
        if(extensionName.toLowerCase() !== '.java')
            return null;
        const fileNameWithoutExtension = source.substring(0, lastDotIndex);
        const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath + "\\" : null;
        if(workspace && fileNameWithoutExtension.indexOf(workspace) === 0)
            return fileNameWithoutExtension.substring(workspace.length, fileNameWithoutExtension.length);
        return fileNameWithoutExtension;
    }

    public static getLineInfoFromPc(pc: number, className: string, method: string, descriptor: string) : DebugLineInfo | null {
        const clsPath = this.findClassFileFromName(className);
        if(clsPath) {
            const srcPath = this.findSourceFile(className);
            if(!srcPath)
                return null;
            const classLoader: ClassLoader = DebugLineInfo.load(clsPath);
            const methodInfo: MethodInfo | null = classLoader.getMethodInfo(method, descriptor);
            if(methodInfo && methodInfo.attributeCode) {
                const attrLinesNumber = methodInfo.attributeCode.getLinesNumber();
                if(!attrLinesNumber)
                    return null;
                const linesNumber = attrLinesNumber.linesNumber;
                const length: number = linesNumber.length;
                for(let i = length - 1; i >= 0; i--) {
                    if(pc >= linesNumber[i].startPc) {
                        const line = linesNumber[i].line;
                        const codeLength = (((i + 1) < linesNumber.length) ? linesNumber[i + 1].startPc : methodInfo.attributeCode.code.length) - pc;
                        return new DebugLineInfo(pc, line, codeLength, method, descriptor, className, clsPath, srcPath);
                    }
                }
            }
            return null;
        }
        return null;
    }

    public static getLineInfoFromLine(line: number, srcPath: string) : DebugLineInfo | null {
        const className = this.getClassNameFormSource(srcPath);
        const clsPath = className ? this.findClassFileFromName(className) : null;
        if(className && clsPath) {
            const classLoader: ClassLoader = DebugLineInfo.load(clsPath);
            for(let i = 0; i < classLoader.methodsInfos.length; i++) {
                const methodInfo = classLoader.methodsInfos[i];
                if(methodInfo.attributeCode) {
                    const attrLineNumber = methodInfo.attributeCode.getLinesNumber();
                    if(!attrLineNumber)
                        return null;
                    const linesNumber = attrLineNumber.linesNumber;
                    if(linesNumber[linesNumber.length - 1].line < line)
                        continue;
                    for(let j = 0; j < linesNumber.length; j++) {
                        if(linesNumber[j].line >= line) {
                            const pc = linesNumber[j].startPc;
                            const method = methodInfo.name;
                            const descriptor = methodInfo.descriptor;
                            const codeLength = (((i + 1) < linesNumber.length) ? linesNumber[i + 1].startPc : methodInfo.attributeCode.code.length) - pc;
                            return new DebugLineInfo(pc, line, codeLength, method, descriptor, className, clsPath, srcPath);
                        }
                    }
                }
            }
            return null;
        }
        return null;
    }
}
