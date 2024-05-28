package java.lang;

public final class Byte extends Number implements Comparable<Byte> {
    public static final byte MIN_VALUE = -128;
    public static final byte MAX_VALUE = 127;
    public static final int SIZE = 8;
    public static final int BYTES = SIZE / Byte.SIZE;
    @SuppressWarnings("unchecked")
    public static final Class<Byte> TYPE = (Class<Byte>)Class.getPrimitiveClass("byte");

    private final byte value;

    public static String toString(byte b) {
        return Integer.toString(b);
    }

    public static Byte valueOf(byte b) {
        return new Byte(b);
    }

    public static byte parseByte(String s, int radix) throws NumberFormatException {
        int i = Integer.parseInt(s, radix);
        if(i < MIN_VALUE || i > MAX_VALUE)
            throw new NumberFormatException("Value out of range. Value:\"" + s + "\" Radix:" + radix);
        return (byte)i;
    }

    public static byte parseByte(String s) throws NumberFormatException {
        return parseByte(s, 10);
    }

    public static Byte valueOf(String s, int radix)
        throws NumberFormatException {
        return valueOf(parseByte(s, radix));
    }

    public static Byte valueOf(String s) throws NumberFormatException {
        return valueOf(s, 10);
    }

    public static Byte decode(String nm) throws NumberFormatException {
        int i = Integer.decode(nm);
        if(i < MIN_VALUE || i > MAX_VALUE)
            throw new NumberFormatException("Value " + i + " out of range from input " + nm);
        return valueOf((byte)i);
    }

    public Byte(byte value) {
        this.value = value;
    }

    public Byte(String s) throws NumberFormatException {
        this.value = parseByte(s, 10);
    }

    @Override
    public byte byteValue() {
        return value;
    }

    @Override
    public short shortValue() {
        return (short)value;
    }

    @Override
    public int intValue() {
        return (int)value;
    }

    @Override
    public long longValue() {
        return (long)value;
    }

    @Override
    public float floatValue() {
        return (float)value;
    }

    @Override
    public double doubleValue() {
        return (double)value;
    }

    @Override
    public String toString() {
        return Integer.toString(value);
    }

    @Override
    public int hashCode() {
        return Byte.hashCode(value);
    }

    public static int hashCode(byte value) {
        return (int)value;
    }

    @Override
    public boolean equals(Object obj) {
        if(obj instanceof Byte) {
            return value == ((Byte)obj).byteValue();
        }
        return false;
    }

    @Override
    public int compareTo(Byte anotherByte) {
        return compare(this.value, anotherByte.value);
    }

    public static int compare(byte x, byte y) {
        return x - y;
    }

    public static int compareUnsigned(byte x, byte y) {
        return Byte.toUnsignedInt(x) - Byte.toUnsignedInt(y);
    }

    public static int toUnsignedInt(byte x) {
        return ((int) x) & 0xff;
    }

    public static long toUnsignedLong(byte x) {
        return ((long) x) & 0xffL;
    }
}
