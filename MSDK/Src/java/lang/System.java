package java.lang;

import java.io.PrintStream;

public final class System {
    public static final PrintStream out = new PrintStream();

    public static native long currentTimeMillis();

    public static native long nanoTime();

    public static native final void arraycopy(Object src, int srcPos, Object dest, int destPos, int length);

    public static void exit(int status) {
        // TODO
    }

    public static void gc() {
        // TODO
    }
}
