package java.lang;

public final class Boolean implements Comparable<Boolean> {
    public static final Boolean TRUE = true;
    public static final Boolean FALSE = false;
    @SuppressWarnings("unchecked")
    public static final Class<Boolean> TYPE = (Class<Boolean>)Class.getPrimitiveClass("boolean");

    private final boolean value;

    public Boolean(boolean value) {
        this.value = value;
    }

    public Boolean(String s) {
        this(parseBoolean(s));
    }

    public static boolean parseBoolean(String s) {
        return "true".equalsIgnoreCase(s);
    }

    public boolean booleanValue() {
        return value;
    }

    public static Boolean valueOf(boolean b) {
        return (b ? TRUE : FALSE);
    }

    public static Boolean valueOf(String s) {
        return parseBoolean(s) ? TRUE : FALSE;
    }

    public static String toString(boolean b) {
        return String.valueOf(b);
    }

    @Override
    public String toString() {
        return String.valueOf(value);
    }

    @Override
    public int hashCode() {
        return Boolean.hashCode(value);
    }

    public static int hashCode(boolean value) {
        return value ? 1231 : 1237;
    }

    @Override
    public boolean equals(Object obj) {
        if(obj instanceof Boolean)
            return value == ((Boolean)obj).booleanValue();
        return false;
    }

    @Override
    public int compareTo(Boolean b) {
        return compare(this.value, b.value);
    }

    public static int compare(boolean x, boolean y) {
        return (x == y) ? 0 : (x ? 1 : -1);
    }

    public static boolean logicalAnd(boolean a, boolean b) {
        return a && b;
    }

    public static boolean logicalOr(boolean a, boolean b) {
        return a || b;
    }

    public static boolean logicalXor(boolean a, boolean b) {
        return a ^ b;
    }
}
