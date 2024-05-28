package java.lang;

import jdk.internal.math.FloatToDecimal;

import static java.lang.Math.FLOAT_EXP_BIAS;
import static java.lang.Math.FLOAT_SIGNIFICAND_WIDTH;

public final class Float extends Number implements Comparable<Float> {
    public static final float POSITIVE_INFINITY = 1.0f / 0.0f;
    public static final float NEGATIVE_INFINITY = -1.0f / 0.0f;
    public static final float NaN = 0.0f / 0.0f;
    public static final float MAX_VALUE = 0x1.fffffeP+127f;
    public static final float MIN_NORMAL = 0x1.0p-126f;
    public static final float MIN_VALUE = 0x0.000002P-126f;
    public static final int SIZE = 32;
    public static final int PRECISION = 24;
    public static final int MAX_EXPONENT = (1 << (SIZE - PRECISION - 1)) - 1;
    public static final int MIN_EXPONENT = 1 - MAX_EXPONENT;
    public static final int BYTES = SIZE / Byte.SIZE;
    @SuppressWarnings("unchecked")
    public static final Class<Float> TYPE = (Class<Float>)Class.getPrimitiveClass("float");

    private final float value;

    public static String toString(float f) {
        return FloatToDecimal.toString(f);
    }

    public static String toHexString(float f) {
        if(Math.abs(f) < Float.MIN_NORMAL &&  f != 0.0f) {
            String s = Double.toHexString(Math.scalb((double)f, Double.MIN_EXPONENT - Float.MIN_EXPONENT));
            int index = s.lastIndexOf("p-1022");
            StringBuilder sb = new StringBuilder(s.substring(0, index));
            sb.append("p-126");
            return sb.toString();
        }
        else
            return Double.toHexString(f);
    }

    public static Float valueOf(String s) throws NumberFormatException {
        return new Float(parseFloat(s));
    }

    public static Float valueOf(float f) {
        return new Float(f);
    }

    public static float parseFloat(String s) throws NumberFormatException {
        // TODO
        // return FloatingDecimal.parseFloat(s);
        throw new NumberFormatException("TODO");
    }

    public static boolean isNaN(float v) {
        return (v != v);
    }

    public static boolean isInfinite(float v) {
        return Math.abs(v) > MAX_VALUE;
    }

    public static boolean isFinite(float f) {
        return Math.abs(f) <= Float.MAX_VALUE;
    }

    public Float(float value) {
        this.value = value;
    }

    public Float(double value) {
        this.value = (float)value;
    }

    public Float(String s) throws NumberFormatException {
        value = parseFloat(s);
    }

    public boolean isNaN() {
        return isNaN(value);
    }

    public boolean isInfinite() {
        return isInfinite(value);
    }

    @Override
    public String toString() {
        return Float.toString(value);
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
        return value;
    }

    @Override
    public double doubleValue() {
        return (double)value;
    }

    @Override
    public int hashCode() {
        return Float.hashCode(value);
    }

    public static int hashCode(float value) {
        return floatToIntBits(value);
    }

    @Override
    public boolean equals(Object obj) {
        return (obj instanceof Float) && (floatToIntBits(((Float)obj).value) == floatToIntBits(value));
    }

    public static int floatToIntBits(float value) {
        if(!isNaN(value))
            return floatToRawIntBits(value);
        return 0x7fc00000;
    }

    public static native int floatToRawIntBits(float value);

    public static native float intBitsToFloat(int bits);

    public static float float16ToFloat(short floatBinary16) {
        int bin16arg = (int)floatBinary16;
        int bin16SignBit = 0x8000 & bin16arg;
        int bin16ExpBits = 0x7c00 & bin16arg;
        int bin16SignifBits = 0x03FF & bin16arg;

        float sign = (bin16SignBit != 0) ? -1.0f : 1.0f;

        int bin16Exp = (bin16ExpBits >> 10) - 15;
        if(bin16Exp == -15)
            return sign * (0x1p-24f * bin16SignifBits);
        else if(bin16Exp == 16) {
            if(bin16SignifBits == 0)
                return sign * Float.POSITIVE_INFINITY;
            else
                Float.intBitsToFloat((bin16SignBit << 16) | 0x7f80_0000 | (bin16SignifBits << (FLOAT_SIGNIFICAND_WIDTH - 11)));
        }

        // assert -15 < bin16Exp  && bin16Exp < 16;

        int floatExpBits = (bin16Exp + FLOAT_EXP_BIAS) << (FLOAT_SIGNIFICAND_WIDTH - 1);

        return Float.intBitsToFloat((bin16SignBit << 16) | floatExpBits | (bin16SignifBits << (FLOAT_SIGNIFICAND_WIDTH - 11)));
    }

    public static short floatToFloat16(float f) {
        int doppel = Float.floatToRawIntBits(f);
        short sign_bit = (short)((doppel & 0x8000_0000) >> 16);

        if(Float.isNaN(f))
            return (short)(sign_bit | 0x7c00 | (doppel & 0x007f_e000) >> 13 | (doppel & 0x0000_1ff0) >> 4 | (doppel & 0x0000_000f));

        float abs_f = Math.abs(f);

        if(abs_f >= (0x1.ffcp15f + 0x0.002p15f))
            return (short)(sign_bit | 0x7c00);

        if(abs_f <= 0x1.0p-24f * 0.5f)
            return sign_bit;

        int exp = Math.getExponent(f);
        // assert -25 <= exp && exp <= 15;

        int expdelta = 0;
        int msb = 0x0000_0000;
        if(exp < -14) {
            expdelta = -14 - exp;
            exp = -15;
            msb = 0x0080_0000;
        }
        int f_signif_bits = doppel & 0x007f_ffff | msb;

        short signif_bits = (short)(f_signif_bits >> (13 + expdelta));

        int lsb    = f_signif_bits & (1 << 13 + expdelta);
        int round  = f_signif_bits & (1 << 12 + expdelta);
        int sticky = f_signif_bits & ((1 << 12 + expdelta) - 1);

        if(round != 0 && ((lsb | sticky) != 0 ))
            signif_bits++;

        // assert (0xf800 & signif_bits) == 0x0;

        return (short)(sign_bit | ( ((exp + 15) << 10) + signif_bits ) );
    }

    @Override
    public int compareTo(Float anotherFloat) {
        return Float.compare(value, anotherFloat.value);
    }

    public static int compare(float f1, float f2) {
        if(f1 < f2)
            return -1;
        if(f1 > f2)
            return 1;

        int thisBits    = Float.floatToIntBits(f1);
        int anotherBits = Float.floatToIntBits(f2);

        return (thisBits == anotherBits ?  0 : (thisBits < anotherBits ? -1 : 1));
    }

    public static float sum(float a, float b) {
        return a + b;
    }

    public static float max(float a, float b) {
        return Math.max(a, b);
    }

    public static float min(float a, float b) {
        return Math.min(a, b);
    }
}
