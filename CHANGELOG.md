# Change Log
## V1.1.1
- Fix bug relate to VM.
  - Bug when call to methods of an array object.
  - Continuous reset error when java code throws exception with null message.
  - Wrong unit in System.currentTimeMillis.
  - Bug when handling exceptions.
  - Infinite loop in Thread.sleep.
  - Errors related to java stack.
  - Infinite recursion in garbageCollectionProtectObject.
- Improved performance for invokeVirtual and invokeInterace.
- Improved performance for field accessibility.
- Support flint.drawing.Graphics (not completed yet).
## V1.1.0
- Fix free ClassData error when FlintClassLoader throws an exception.
- Workround for ESP32 can't catch exceptions in a top-level of a task.
- Supports additional methods to create exception objects.
- Changed CRC calculation algorithm used in FlintString and debugger.
- Add SEEK_FILE in debugger.
- Fix bug in "lcmp" bytecode instruction.
- Implement Object.identityHashCode native method.
- Add some native methods to support BigInteger (incomplete).
- Add method to support check the terminate request.
- Support thorw InterruptedException in Thread.sleep0 native method.
- Improve and fix other bugs.
## V1.0.0
- Support execution of most java bytecode instructions.
- Support throw and catch mechanism.
- Support simple garbage collectors.
- Support multi-threading.
- Support debugging with Visual Studio Code via UART, USB CDC (may support wifi in the future):
  - Pause, Continue, Restart, Step Over, Step Into, Step Out.
  - Stack trace.
  - Set and remove breakpoints.
  - Stop on exception and display exception information.
  - View local variables and evaluate expressions.
  - Display message printed from java code.