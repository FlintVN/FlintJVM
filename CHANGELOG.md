# Change Log
## V1.1.7
- Fixed bug in FExec::getOnwerThread function that did not return value.
- Self-implemented mutex (No need to implement when porting).
- Debugger:
  - Fixed bug breakpoints and jumps being jumped in place when static constructor is called.
  - Fixed bug sometimes debugging could not be paused in case step over/in/out was executing.
  - Fixed state synchronization between threads when debugging.
- Slight improvement in bytecode execution performance.
- Fix memory leak in some native methods of java.lang.Class.
- Fix missing unlock in Flint::getClassArray0 method.
## V1.1.6
- Fix VM crash when native method not found.
- Fix stack trace with wrong PC address when debugging.
- Fix stuck at Flint::terminate for single core chips.
- Fixed VM throwing wrong type for some exceptions or not displaying messages.
## V1.1.5
- Fix method not found in invokeInterface (bridge method).
- Support lazy loading for method code (Reduce RAM consumption).
- Fix java.lang.reflect.Array.get returning short instead of character with character input array.
- Fix crash when class file contains long ConstUtf8 string.
- Eliminate the impact of breakpoints on performance when debugging.
- Temporarily dropping support for java.math.BigInteger for future changes and improvements.
- Reduce RAM consumption.
## V1.1.4
- Fix bug when handling finally block in exception.
## V1.1.3
- Fix bug when calling invokevirtual invokeinterface with null object.
- Fix bug stuck in MethodInfo::getAttribute, MethodInfo::getAttributeCode and MethodInfo::getAttributeNative.
- Fix bug related to object synchronization and static method synchronization.
- Fix bug of not calling static constructor in some cases.
- Fix breakpoint not working when doing step in/out/over.
- Improve code and performance for VM.
- Improve garbage collection performance and accuracy.
- Reduced RAM usage, fixed memory leak in FlintClassLoader.
- Verify object address in DBG_CMD_READ_FIELD and DBG_CMD_READ_ARRAY command in debugger.
- Change command format for DBG_CMD_READ_LOCAL (Is there additional information about whether the variable is an object or not).
- Implement some native methods for java.lang.Class
  - Class.forName.
  - Class.getDeclaredMethods0.
  - Class.getDeclaredConstructors0.
  - Class.getDeclaredFields0.
  - Class.getDeclaringClass0.
## V1.1.2
- Fix the bug in the Flint::isInstanceof method when checking with primitive and interface types. It impacts checkcast, instanceof instructions, try...catch in java and more.
- Add method to support create IllegalArgumentException object.
- Add method to support create wrapper classes (Boolean, Byte, Character, Short, Integer, Float, Long, Double).
- Full implementation of all native methods for java.lang.reflect.Array.
- Implement some native method for java.lang.Class:
  - getSuperclass.
  - getModifiers.
  - isInterface.
  - isAssignableFrom.
  - getInterfaces0.
- Improve code and fix other bugs.
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
- Changed CRC calculation algorithm used in JString and debugger.
- Add SEEK_FILE command in debugger.
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