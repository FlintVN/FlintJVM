
import {
    Logger, logger,
    LoggingDebugSession,
    InitializedEvent, TerminatedEvent, StoppedEvent, BreakpointEvent, OutputEvent,
    ProgressStartEvent, ProgressUpdateEvent, ProgressEndEvent, InvalidatedEvent,
    Thread, StackFrame, Scope, Source, Handles, Breakpoint, MemoryEvent,
    Variable
} from '@vscode/debugadapter';
import * as vscode from 'vscode';
import path = require('path');
import { ChildProcess, spawn } from 'child_process';
import { DebugProtocol } from '@vscode/debugprotocol';
import { MjvmClientDebugger } from './mjvm_client_debugger';
import { MjvmClassLoader } from './class_loader/mjvm_class_loader';

interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
    embedded?: boolean;
    install?: boolean;
    'main-class': string;
    'sdk-class-path': string;
    'sdk-source-path': string;
}

export class MjvmDebugSession extends LoggingDebugSession {
    private mjvm?: ChildProcess;
    private mainClass?: string;
    private clientDebugger: MjvmClientDebugger;

    public constructor() {
        super('mjvm-debug.txt');
        this.clientDebugger = new MjvmClientDebugger();

        this.clientDebugger.onStop((reason?: string) => {
            if(reason)
                this.sendEvent(new StoppedEvent(reason, 1));
            else
                this.sendEvent(new StoppedEvent('stop', 1));
        });

        this.clientDebugger.onError(() => {

        });

        this.clientDebugger.onClose(() => {
            vscode.window.showErrorMessage('MJVM Server has been closed');
            this.sendEvent(new TerminatedEvent());
        });
    }

    protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments) {
        response.body = response.body || {};
        response.body.supportsConfigurationDoneRequest = true;
        response.body.supportsEvaluateForHovers = true;
        response.body.supportsStepBack = false;
        response.body.supportsStepInTargetsRequest = true;
        response.body.supportsSteppingGranularity = true;
        response.body.supportsDataBreakpoints = true;
        response.body.supportsCompletionsRequest = true;
        response.body.supportsCancelRequest = true;
        response.body.supportsBreakpointLocationsRequest = true;
        response.body.supportsExceptionFilterOptions = true;
        response.body.supportsExceptionInfoRequest = true;
        response.body.supportsSetVariable = true;
        response.body.supportsSetExpression = true;
        response.body.supportsDisassembleRequest = true;
        response.body.supportsInstructionBreakpoints = true;
        response.body.supportsReadMemoryRequest = false;
        response.body.supportsWriteMemoryRequest = false;
        response.body.supportSuspendDebuggee = true;
        response.body.supportTerminateDebuggee = true;
        response.body.supportsFunctionBreakpoints = true;
        response.body.supportsDelayedStackTraceLoading = false;
        response.body.supportsHitConditionalBreakpoints = true;
        response.body.supportsConditionalBreakpoints = true;
        response.body.supportsLogPoints = true;
        response.body.supportsRestartRequest = true;
        response.body.supportsTerminateRequest = true;
        response.body.supportsGotoTargetsRequest = false;

        response.body.exceptionBreakpointFilters = [{filter: 'all', label: 'Caught Exceptions', default: false}];

        this.sendResponse(response);
    }

    protected async launchRequest(response: DebugProtocol.LaunchResponse, args: LaunchRequestArguments, request?: DebugProtocol.Request) {
        this.mainClass = args['main-class'];
        MjvmClassLoader.sdkClassPath = args['sdk-class-path'].replace(/\//g, '\\');
        MjvmClassLoader.sdkSourcePath = args['sdk-source-path'].replace(/\//g, '\\');
        if(!args.embedded) {
            if(!await this.startMjvmWithDebug()) {
                this.sendErrorResponse(response, 1, 'Cound start MJVM');
                return;
            }
        }
        await this.clientDebugger.connect();
        const terminateRet = await this.clientDebugger.terminateRequest(false);
        if(!terminateRet) {
            this.sendErrorResponse(response, 1, 'Cound terminate current process');
            return;
        }
        if(args.install) {
            const fileName = 'test.class';
            const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
            const fullPath: string = path.join(workspace, fileName);
            const files: string[] = [fullPath];
            if(!(await this.installFiles(files))) {
                this.sendErrorResponse(response, 1, 'Cound install file');
                return;
            }
        }
        const rmBkpRet = await this.clientDebugger.removeAllBreakPoints();
        if(rmBkpRet) {
            this.sendResponse(response);
            this.sendEvent(new InitializedEvent());
        }
        else
            this.sendErrorResponse(response, 1, 'Cound not remove all breakpoint');
    }

    protected async attachRequest(response: DebugProtocol.AttachResponse, args: DebugProtocol.AttachRequestArguments, request?: DebugProtocol.Request) {
        this.sendResponse(response);
    }

    protected async terminateRequest(response: DebugProtocol.TerminateResponse, args: DebugProtocol.TerminateArguments, request?: DebugProtocol.Request) {
        const value = await this.clientDebugger.terminateRequest(true);
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not terminate');
        this.sendEvent(new TerminatedEvent());
    }

    protected async disconnectRequest(response: DebugProtocol.DisconnectResponse, args: DebugProtocol.DisconnectArguments, request?: DebugProtocol.Request) {
        this.clientDebugger.disconnect();
        this.killMjvm();
        this.sendResponse(response);
    }

    protected async configurationDoneRequest(response: DebugProtocol.ConfigurationDoneResponse, args: DebugProtocol.ConfigurationDoneArguments, request?: DebugProtocol.Request | undefined) {
        if(!this.mainClass)
            this.sendErrorResponse(response, 1, 'Could not start. There is no information about the main class');
        else {
            const value = await this.clientDebugger.restartRequest(this.mainClass);
            if(value) {
                this.clientDebugger.startCheckStatus();
                this.sendResponse(response);
            }
            else
                this.sendErrorResponse(response, 1, 'Could not start ' + this.mainClass);
        }
    }

    protected async setExceptionBreakPointsRequest(response: DebugProtocol.SetExceptionBreakpointsResponse, args: DebugProtocol.SetExceptionBreakpointsArguments, request?: DebugProtocol.Request) {
        let isEnabled: boolean = false;
        if(args.filterOptions && args.filterOptions.length > 0 && args.filterOptions[0].filterId === 'all')
            isEnabled = true;
        const value = await this.clientDebugger.setExceptionBreakPointsRequest(isEnabled);
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'An error occurred while ' + isEnabled ? 'enabling' : 'disabling' + ' Caught Exceptions');
    }

    protected async setBreakPointsRequest(response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments, request?: DebugProtocol.Request) {
        if(args.lines && args.source.path) {
            const value = await this.clientDebugger.setBreakPointsRequest(args.lines, args.source.path);
            if(value)
                this.sendResponse(response);
            else
                this.sendErrorResponse(response, 1, 'An error occurred while setting breakpoint');
        }
        else
            this.sendResponse(response);
    }

    protected async setFunctionBreakPointsRequest(response: DebugProtocol.SetFunctionBreakpointsResponse, args: DebugProtocol.SetFunctionBreakpointsArguments, request?: DebugProtocol.Request) {
        this.sendResponse(response);
    }

    protected async pauseRequest(response: DebugProtocol.PauseResponse, args: DebugProtocol.PauseArguments, request?: DebugProtocol.Request | undefined) {
        const value = await this.clientDebugger.stop();
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not pause');
    }

    protected async continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments) {
        const value = await this.clientDebugger.run();
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not continue');
    }

    protected async nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments) {
        const value = await this.clientDebugger.stepOverRequest();
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not next');
    }

    protected async stepInRequest(response: DebugProtocol.StepInResponse, args: DebugProtocol.StepInArguments, request?: DebugProtocol.Request | undefined) {
        const value = await this.clientDebugger.stepInRequest();
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not step in');
    }

    protected async stepOutRequest(response: DebugProtocol.StepOutResponse, args: DebugProtocol.StepOutArguments, request?: DebugProtocol.Request | undefined) {
        const value = await this.clientDebugger.stepOutRequest();
        if(value)
            this.sendResponse(response);
        else
            this.sendErrorResponse(response, 1, 'Cound not step over');
    }

    protected async restartRequest(response: DebugProtocol.RestartResponse, args: DebugProtocol.RestartArguments, request?: DebugProtocol.Request | undefined) {
        if(!this.mainClass)
            this.sendErrorResponse(response, 1, 'Could not restart. There is no information about the main class');
        else {
            const value = await this.clientDebugger.restartRequest(this.mainClass);
            if(value)
                this.sendResponse(response);
            else
                this.sendErrorResponse(response, 1, 'Cound not restart');
        }
    }

    protected async scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments, request?: DebugProtocol.Request) {
        const scopes: DebugProtocol.Scope[] = [
            new Scope("Local", 0x100000000 + args.frameId, true),
            new Scope("Global", 0x200000000, true),
        ];
        response.body = {
            scopes: scopes
        };
        this.sendResponse(response);
    }

    protected async variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments, request?: DebugProtocol.Request) {
        const variableType: bigint = BigInt(args.variablesReference) >> 32n;
        if(variableType === 1n) {
            const frameId = args.variablesReference & 0xFFFFFFFF;
            const result = await this.clientDebugger.readLocalVariables(frameId);
            if(result)
                response.body = {variables: result};
            this.sendResponse(response);
        }
        else if(variableType === 2n)
            this.sendResponse(response);
        else {
            const result = await this.clientDebugger.readVariable(args.variablesReference);
            if(result)
                response.body = {variables: result};
            this.sendResponse(response);
        }
    }

    protected async evaluateRequest(response: DebugProtocol.EvaluateResponse, args: DebugProtocol.EvaluateArguments) {
        this.sendResponse(response);
    }

    protected async stackTraceRequest(response: DebugProtocol.StackTraceResponse, args: DebugProtocol.StackTraceArguments) {
        const start = (args.startFrame !== null) ? args.startFrame : 0;
        const level = ((args.startFrame !== null) && (args.levels !== null)) ? args.levels : 1;
        if(start !== 0) {
            this.sendResponse(response);
            return;
        }
        const frames = await this.clientDebugger.stackFrameRequest();
        if(frames) {
            response.body = {
                stackFrames: frames,
                totalFrames: frames.length,
            };
            this.sendResponse(response);
        }
        else
            this.sendErrorResponse(response, 1, 'Cound not read stack frame');
    }

    protected async exceptionInfoRequest(response: DebugProtocol.ExceptionInfoResponse, args: DebugProtocol.ExceptionInfoArguments) {
        const excpInfo = await this.clientDebugger.readExceptionInfo();
        if(excpInfo) {
            const typeName = excpInfo.type.replace(/\//g, '.');
            response.body = {
                exceptionId: typeName,
                description: excpInfo.message,
                breakMode: 'always',
                details: {
                    message: excpInfo.message,
                    typeName: typeName,
                }
            };
            this.sendResponse(response);
        }
        else
            this.sendErrorResponse(response, 1, 'Cound not read exception information');
    }

    protected threadsRequest(response: DebugProtocol.ThreadsResponse) {
        response.body = {
            threads: [
                new Thread(1, 'thread 1'),
            ]
        };
        this.sendResponse(response);
    }

    private async installFiles(files: string[]): Promise<boolean> {
        return new Promise((resolve) => {
            vscode.window.withProgress({
                location: vscode.ProgressLocation.Window,
                title: "Processing: ",
                cancellable: true
            }, async (progress, token) => {
                token.onCancellationRequested(() => {
                    console.log("User canceled the long running operation");
                });
                progress.report({ increment: 0 });
    
                let oldPercent = 0;
    
                const progressChanged = (process: number, total: number) => {
                    const percent = process * 100 / total;
                    const increment = percent - oldPercent;
                    oldPercent = percent;
                    progress.report({increment: increment, message: `${percent}% completed`});
                }
                for(let i = 0; i < files.length; i++) {
                    const result = await this.clientDebugger.installFile(files[i], progressChanged);
                    if(!result)
                        resolve(false);
                }
                resolve(true);
            });
        });
    }

    private killMjvm() {
        if(this.mjvm) {
            this.mjvm.stdout?.removeAllListeners();
            this.mjvm.removeAllListeners();
            this.mjvm.kill();
            this.mjvm = undefined;
        }
    }

    private async startMjvmWithDebug(): Promise<boolean> {
        return new Promise((resolve) => {
            const workspace = vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : '';
            this.mjvm = spawn('WinMJVM', ['-g'], {
                cwd: workspace,
                stdio: ['inherit', 'pipe', 'inherit'],
                windowsHide: true,
                detached: true,
            });
            const timeoutTask = setTimeout(() => {
                this.killMjvm();
                resolve(false);
            }, 2000);
            if(!this.mjvm.stdout) {
                clearTimeout(timeoutTask);
                this.killMjvm();
                resolve(false);
                return;
            }
            this.mjvm.stdout.on('data', (data) => {
                console.log(data);
                this.mjvm?.stdout?.removeAllListeners();
                this.mjvm?.removeAllListeners();
                if(data.toString().includes('MJVM is started')) {
                    clearTimeout(timeoutTask);
                    resolve(true);
                }
                else
                    resolve(false);
            });
            this.mjvm.on('close', (code) => {
                clearTimeout(timeoutTask);
                this.killMjvm();
                resolve(false);
            });
            this.mjvm.on('error', (err) => {
                clearTimeout(timeoutTask);
                this.killMjvm();
                resolve(false);
            });
        });
    }
}
