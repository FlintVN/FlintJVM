package java.lang;

public final class Short extends Number implements Comparable<Short> {
    public static final short MIN_VALUE = -32768;
    public static final short MAX_VALUE = 32767;
    public static final int SIZE = 16;
    public static final int BYTES = SIZE / Byte.SIZE;
    public static final Class<Short> TYPE = (Class<Short>) Class.getPrimitiveClass("short");

    public static String toString(short s) {
        return Integer.toString(s);
    }

    public static short parseShort(String s, int radix)
        throws NumberFormatException {
        int i = Integer.parseInt(s, radix);
        if (i < MIN_VALUE || i > MAX_VALUE)
            throw new NumberFormatException("Value out of range. Value:\"" + s + "\" Radix:" + radix);
        return (short)i;
    }

    public static short parseShort(String s) throws NumberFormatException {
        return parseShort(s, 10);
    }

    public static Short valueOf(String s, int radix) throws NumberFormatException {
        return valueOf(parseShort(s, radix));
    }

    public static Short valueOf(String s) throws NumberFormatException {
        return valueOf(s, 10);
    }

    public static Short valueOf(short s) {
        return new Short(s);
    }

    public static Short decode(String nm) throws NumberFormatException {
        int i = Integer.decode(nm);
        if (i < MIN_VALUE || i > MAX_VALUE)
            throw new NumberFormatException("Value " + i + " out of range from input " + nm);
        return valueOf((short)i);
    }

    private final short value;

    public Short(short value) {
        this.value = value;
    }

    public Short(String s) throws NumberFormatException {
        this.value = parseShort(s, 10);
    }

    public byte byteValue() {
        return (byte)value;
    }

    public short shortValue() {
        return value;
    }

    public int intValue() {
        return (int)value;
    }

    public long longValue() {
        return (long)value;
    }

    public float floatValue() {
        return (float)value;
    }

    public double doubleValue() {
        return (double)value;
    }

    public String toString() {
        return Integer.toString(value);
    }

    public int hashCode() {
        return Short.hashCode(value);
    }

    public static int hashCode(short value) {
        return (int)value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Short)
            return value == ((Short)obj).shortValue();
        return false;
    }

    public int compareTo(Short anotherShort) {
        return compare(this.value, anotherShort.value);
    }

    public static int compare(short x, short y) {
        return x - y;
    }

    public static int compareUnsigned(short x, short y) {
        return Short.toUnsignedInt(x) - Short.toUnsignedInt(y);
    }

    public static short reverseBytes(short i) {
        return (short) (((i & 0xFF00) >> 8) | (i << 8));
    }

    public static int toUnsignedInt(short x) {
        return ((int) x) & 0xffff;
    }

    public static long toUnsignedLong(short x) {
        return ((long) x) & 0xffffL;
    }
}
