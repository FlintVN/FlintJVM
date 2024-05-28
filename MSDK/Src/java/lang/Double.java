package java.lang;

import jdk.internal.math.DoubleToDecimal;

import static java.lang.Math.DOUBLE_SIGNIF_BIT_MASK;

public final class Double extends Number implements Comparable<Double> {
    public static final double POSITIVE_INFINITY = 1.0 / 0.0;
    public static final double NEGATIVE_INFINITY = -1.0 / 0.0;
    public static final double NaN = 0.0d / 0.0;
    public static final double MAX_VALUE = 0x1.fffffffffffffP+1023;
    public static final double MIN_NORMAL = 0x1.0p-1022;
    public static final double MIN_VALUE = 0x0.0000000000001P-1022;
    public static final int SIZE = 64;
    public static final int PRECISION = 53;
    public static final int MAX_EXPONENT = (1 << (SIZE - PRECISION - 1)) - 1; // 1023
    public static final int MIN_EXPONENT = 1 - MAX_EXPONENT; // -1022
    public static final int BYTES = SIZE / Byte.SIZE;
    @SuppressWarnings("unchecked")
    public static final Class<Double> TYPE = (Class<Double>)Class.getPrimitiveClass("double");

    private final double value;

    public static String toString(double d) {
        return DoubleToDecimal.toString(d);
    }

    public static String toHexString(double d) {
        if(!isFinite(d))
            return Double.toString(d);
        else {
            StringBuilder answer = new StringBuilder(24);

            if(Math.copySign(1.0, d) == -1.0)
                answer.append("-");

            answer.append("0x");

            d = Math.abs(d);

            if(d == 0.0)
                answer.append("0.0p0");
            else {
                boolean subnormal = (d < Double.MIN_NORMAL);

                long signifBits = (Double.doubleToLongBits(d) & DOUBLE_SIGNIF_BIT_MASK) | 0x1000000000000000L;

                answer.append(subnormal ? "0." : "1.");

                String signif = Long.toHexString(signifBits).substring(3, 16);
                if(signif.equals("0000000000000"))
                    signif = "0";
                else {
                    int length = signif.length();
                    while(signif.charAt(length - 1) == '0')
                        length--;
                    signif = signif.substring(0, length);
                }
                answer.append(signif);

                answer.append('p');
                answer.append(subnormal ? Double.MIN_EXPONENT: Math.getExponent(d));
            }
            return answer.toString();
        }
    }

    public static Double valueOf(String s) throws NumberFormatException {
        return new Double(parseDouble(s));
    }

    public static Double valueOf(double d) {
        return new Double(d);
    }

    public static double parseDouble(String s) throws NumberFormatException {
        // TODO
        // return FloatingDecimal.parseDouble(s);
        throw new NumberFormatException("TODO");
    }

    public static boolean isNaN(double v) {
        return (v != v);
    }

    public static boolean isInfinite(double v) {
        return Math.abs(v) > MAX_VALUE;
    }

    public static boolean isFinite(double d) {
        return Math.abs(d) <= Double.MAX_VALUE;
    }

    public Double(double value) {
        this.value = value;
    }

    public Double(String s) throws NumberFormatException {
        value = parseDouble(s);
    }

    public boolean isNaN() {
        return isNaN(value);
    }

    public boolean isInfinite() {
        return isInfinite(value);
    }

    @Override
    public String toString() {
        return toString(value);
    }

    @Override
    public byte byteValue() {
        return (byte)value;
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
        return value;
    }

    @Override
    public int hashCode() {
        return Double.hashCode(value);
    }

    public static int hashCode(double value) {
        return Long.hashCode(doubleToLongBits(value));
    }

    @Override
    public boolean equals(Object obj) {
        return (obj instanceof Double) && (doubleToLongBits(((Double)obj).value) == doubleToLongBits(value));
    }

    public static long doubleToLongBits(double value) {
        if(!isNaN(value))
            return doubleToRawLongBits(value);
        return 0x7ff8000000000000L;
    }

    public static native long doubleToRawLongBits(double value);

    public static native double longBitsToDouble(long bits);

    @Override
    public int compareTo(Double anotherDouble) {
        return Double.compare(value, anotherDouble.value);
    }

    public static int compare(double d1, double d2) {
        if(d1 < d2)
            return -1;
        if(d1 > d2)
            return 1;

        long thisBits    = Double.doubleToLongBits(d1);
        long anotherBits = Double.doubleToLongBits(d2);

        return (thisBits == anotherBits ?  0 : (thisBits < anotherBits ? -1 : 1));
    }

    public static double sum(double a, double b) {
        return a + b;
    }

    public static double max(double a, double b) {
        return Math.max(a, b);
    }

    public static double min(double a, double b) {
        return Math.min(a, b);
    }
}
