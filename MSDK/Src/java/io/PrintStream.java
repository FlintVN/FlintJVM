package java.io;

public class PrintStream {
    private static final native void write(String str);
    private static final native void writeln(String str);

    public final void print(String str) {
        write(str);
    }

    public final void print(boolean b) {
        write(String.valueOf(b));
    }

    public final void print(byte b) {
        write(String.valueOf(b));
    }

    public final void print(short s) {
        write(String.valueOf(s));
    }

    public final void print(char[] c) {
        write(String.valueOf(c));
    }

    public final void print(int i) {
        write(String.valueOf(i));
    }

    public final void print(long l) {
        write(String.valueOf(l));
    }

    public final void print(float f) {
        write(String.valueOf(f));
    }

    public final void print(double d) {
        write(String.valueOf(d));
    }

    public final void print(Object obj) {
        write(String.valueOf(obj));
    }

    public final void println() {
        writeln("");
    }

    public final void println(String str) {
        writeln(String.valueOf(str));
    }

    public final void println(boolean b) {
        writeln(String.valueOf(b));
    }

    public final void println(byte b) {
        writeln(String.valueOf(b));
    }

    public final void println(short s) {
        writeln(String.valueOf(s));
    }

    public final void println(char[] c) {
        writeln(String.valueOf(c));
    }

    public final void println(int i) {
        writeln(String.valueOf(i));
    }

    public final void println(long l) {
        writeln(String.valueOf(l));
    }

    public final void println(float f) {
        writeln(String.valueOf(f));
    }

    public final void println(double d) {
        writeln(String.valueOf(d));
    }

    public final void println(Object obj) {
        writeln(String.valueOf(obj));
    }
}
