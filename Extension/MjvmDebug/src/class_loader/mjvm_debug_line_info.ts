
import fs = require('fs');
import path = require('path');
import { ClassLoader } from './mjvm_class_loader';
import * as vscode from 'vscode';
import { MethodInfo } from './mjvm_method_info';
import { AttributeLineNumber } from './mjvm_attribute_info';

export class DebugLineInfo {
    public readonly pc: number;
    public readonly line: number;
    public readonly methodName: string;
    public readonly descriptor: string;
    public readonly className: string;
    public readonly classPath: string;
    public readonly sourcePath: string | null;

    private static classLoaderDictionary: Record<string, ClassLoader> = {};

    private constructor(pc: number, line: number, methodName: string, descriptor: string, clsName: string, clsPath: string, srcPath: string | null) {
        this.pc = pc;
        this.line = line;
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
            const classLoader: ClassLoader = DebugLineInfo.load(clsPath);
            const methodInfo: MethodInfo | null = classLoader.getMethodInfo(method, descriptor);
            if(methodInfo && methodInfo.attributeCode) {
                const attrLineNumber: AttributeLineNumber | null = methodInfo.attributeCode.getLinesNumber();
                if(!attrLineNumber)
                    return null;
                const length: number = attrLineNumber.linesNumber.length;
                for(let i = length - 1; i >= 0; i--) {
                    if(pc >= attrLineNumber.linesNumber[i].startPc) {
                        const line = attrLineNumber.linesNumber[i].line;
                        return new DebugLineInfo(pc, line, method, descriptor, className, clsPath, srcPath);
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
            for(let i = classLoader.methodsInfos.length - 1; i >= 0; i--) {
                const methodInfo = classLoader.methodsInfos[i];
                if(methodInfo.attributeCode) {
                    const attrLineNumber: AttributeLineNumber | null = methodInfo.attributeCode.getLinesNumber();
                    if(!attrLineNumber)
                        return null;
                    if(line < attrLineNumber.linesNumber[0].line)
                        continue;
                    for(let j = attrLineNumber.linesNumber.length - 1; j >= 0; j--) {
                        if(line >= attrLineNumber.linesNumber[j].line) {
                            const pc = attrLineNumber.linesNumber[j].startPc;
                            const method = methodInfo.name;
                            const descriptor = methodInfo.descriptor;
                            return new DebugLineInfo(pc, line, method, descriptor, className, clsPath, srcPath);
                        }
                    }
                }
            }
            return null;
        }
        return null;
    }
}
