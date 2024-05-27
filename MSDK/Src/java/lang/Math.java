package java.lang;

import java.util.Random;

public final class Math {
    public static final double E = 2.718281828459045;
    public static final double PI = 3.141592653589793;
    public static final double TAU = 2.0 * PI;

    static final int FLOAT_SIGNIFICAND_WIDTH = Float.PRECISION;
    static final int FLOAT_MIN_SUB_EXPONENT = Float.MIN_EXPONENT - (FLOAT_SIGNIFICAND_WIDTH - 1);
    static final int FLOAT_EXP_BIAS = (1 << (Float.SIZE - FLOAT_SIGNIFICAND_WIDTH - 1)) - 1;
    static final int FLOAT_SIGN_BIT_MASK = 1 << (Float.SIZE - 1);
    static final int FLOAT_EXP_BIT_MASK = ((1 << (Float.SIZE - FLOAT_SIGNIFICAND_WIDTH)) - 1) << (FLOAT_SIGNIFICAND_WIDTH - 1);
    static final int FLOAT_SIGNIF_BIT_MASK = (1 << (FLOAT_SIGNIFICAND_WIDTH - 1)) - 1;
    static final int FLOAT_MAG_BIT_MASK = FLOAT_EXP_BIT_MASK | FLOAT_SIGNIF_BIT_MASK;

    static final int DOUBLE_SIGNIFICAND_WIDTH = Double.PRECISION;
    static final int DOUBLE_MIN_SUB_EXPONENT = Double.MIN_EXPONENT - (DOUBLE_SIGNIFICAND_WIDTH - 1);
    static final int DOUBLE_EXP_BIAS = (1 << (Double.SIZE - DOUBLE_SIGNIFICAND_WIDTH - 1)) - 1;
    static final long DOUBLE_SIGN_BIT_MASK = 1L << (Double.SIZE - 1);
    static final long DOUBLE_EXP_BIT_MASK = ((1L << (Double.SIZE - DOUBLE_SIGNIFICAND_WIDTH)) - 1) << (DOUBLE_SIGNIFICAND_WIDTH - 1);
    static final long DOUBLE_SIGNIF_BIT_MASK = (1L << (DOUBLE_SIGNIFICAND_WIDTH - 1)) - 1;
    static final long DOUBLE_MAG_BIT_MASK = DOUBLE_EXP_BIT_MASK | DOUBLE_SIGNIF_BIT_MASK;

    private Math() {

    }

    public static native double sin(double a);

    public static native double cos(double a);

    public static native double tan(double a);

    public static native double asin(double a);

    public static native double acos(double a);

    public static native double atan(double a);

    public static double toRadians(double angdeg) {
        return angdeg * 0.017453292519943295d;
    }

    public static double toDegrees(double angrad) {
        return angrad * 57.29577951308232d;
    }

    public static double exp(double a) {
        return pow(Math.E, a);
    }

    public static native double log(double a);

    public static native double log10(double a);

    public static native double sqrt(double a);

    public static native double cbrt(double a);

    public static double IEEEremainder(double f1, double f2) {
        return f1 - (f2 * Math.rint(f1 / f2));
    }

    public static double ceil(double a) {
        return floorOrCeil(a, -0.0, 1.0, 1.0);
    }

    public static double floor(double a) {
        return floorOrCeil(a, -1.0, 0.0, -1.0);
    }

    private static double floorOrCeil(double a, double negativeBoundary, double positiveBoundary, double sign) {
        int exponent = Math.getExponent(a);

        if(exponent < 0)
            return ((a == 0.0) ? a : ((a < 0.0) ?  negativeBoundary : positiveBoundary));
        else if(exponent >= 52)
            return a;
        // assert exponent >= 0 && exponent <= 51;

        long doppel = Double.doubleToRawLongBits(a);
        long mask = DOUBLE_SIGNIF_BIT_MASK >> exponent;

        if( (mask & doppel) == 0L )
            return a;
        else {
            double result = Double.longBitsToDouble(doppel & (~mask));
            if(sign*a > 0.0)
                result = result + sign;
            return result;
        }
    }

    public static double rint(double a) {
        double twoToThe52 = (double)(1L << 52);
        double sign = Math.copySign(1.0, a);
        a = Math.abs(a);

        if(a < twoToThe52)
            a = ((twoToThe52 + a ) - twoToThe52);

        return sign * a;
    }

    public static native double atan2(double y, double x);

    public static native double pow(double a, double b);

    public static int round(float a) {
        int intBits = Float.floatToRawIntBits(a);
        int biasedExp = (intBits & FLOAT_EXP_BIT_MASK) >> (FLOAT_SIGNIFICAND_WIDTH - 1);
        int shift = (FLOAT_SIGNIFICAND_WIDTH - 2 + FLOAT_EXP_BIAS) - biasedExp;
        if((shift & -32) == 0) {
            int r = ((intBits & FLOAT_SIGNIF_BIT_MASK) | (FLOAT_SIGNIF_BIT_MASK + 1));
            if(intBits < 0)
                r = -r;
            return ((r >> shift) + 1) >> 1;
        }
        else
            return (int) a;
    }

    public static long round(double a) {
        long longBits = Double.doubleToRawLongBits(a);
        long biasedExp = (longBits & DOUBLE_EXP_BIT_MASK) >> (DOUBLE_SIGNIFICAND_WIDTH - 1);
        long shift = (DOUBLE_SIGNIFICAND_WIDTH - 2 + DOUBLE_EXP_BIAS) - biasedExp;
        if((shift & -64) == 0) {
            long r = ((longBits & DOUBLE_SIGNIF_BIT_MASK) | (DOUBLE_SIGNIF_BIT_MASK + 1));
            if(longBits < 0)
                r = -r;
            return ((r >> shift) + 1) >> 1;
        }
        else
            return (long) a;
    }

    public static double random() {
        return Random.nextDouble();
    }

    public static long multiplyFull(int x, int y) {
        return (long)x * (long)y;
    }

    public static long multiplyHigh(long x, long y) {
        long x1 = x >> 32;
        long x2 = x & 0xFFFFFFFFL;
        long y1 = y >> 32;
        long y2 = y & 0xFFFFFFFFL;

        long z2 = x2 * y2;
        long t = x1 * y2 + (z2 >>> 32);
        long z1 = t & 0xFFFFFFFFL;
        long z0 = t >> 32;
        z1 += x2 * y1;

        return x1 * y1 + z0 + (z1 >> 32);
    }

    public static long unsignedMultiplyHigh(long x, long y) {
        long result = Math.multiplyHigh(x, y);
        result += (y & (x >> 63)); // equivalent to `if(x < 0) result += y;`
        result += (x & (y >> 63)); // equivalent to `if(y < 0) result += x;`
        return result;
    }

    public static int floorDiv(int x, int y) {
        final int q = x / y;
        if((x ^ y) < 0 && (q * y != x))
            return q - 1;
        return q;
    }

    public static long floorDiv(long x, int y) {
        return floorDiv(x, (long)y);
    }

    public static long floorDiv(long x, long y) {
        final long q = x / y;
        if((x ^ y) < 0 && (q * y != x))
            return q - 1;
        return q;
    }

    public static int floorMod(int x, int y) {
        final int r = x % y;
        if((x ^ y) < 0 && r != 0)
            return r + y;
        return r;
    }

    public static int floorMod(long x, int y) {
        return (int)floorMod(x, (long)y);
    }

    public static long floorMod(long x, long y) {
        final long r = x % y;
        if((x ^ y) < 0 && r != 0)
            return r + y;
        return r;
    }

    public static int ceilDiv(int x, int y) {
        final int q = x / y;
        if((x ^ y) >= 0 && (q * y != x))
            return q + 1;
        return q;
    }

    public static long ceilDiv(long x, int y) {
        return ceilDiv(x, (long)y);
    }

    public static long ceilDiv(long x, long y) {
        final long q = x / y;
        if((x ^ y) >= 0 && (q * y != x))
            return q + 1;
        return q;
    }

    public static int ceilMod(int x, int y) {
        final int r = x % y;
        if((x ^ y) >= 0 && r != 0)
            return r - y;
        return r;
    }

    public static int ceilMod(long x, int y) {
        return (int)ceilMod(x, (long)y);
    }

    public static long ceilMod(long x, long y) {
        final long r = x % y;
        if((x ^ y) >= 0 && r != 0)
            return r - y;
        return r;
    }

    public static int abs(int a) {
        return (a < 0) ? -a : a;
    }

    public static long abs(long a) {
        return (a < 0) ? -a : a;
    }

    public static float abs(float a) {
        return Float.intBitsToFloat(Float.floatToRawIntBits(a) & FLOAT_MAG_BIT_MASK);
    }

    public static double abs(double a) {
        return Double.longBitsToDouble(Double.doubleToRawLongBits(a) & DOUBLE_MAG_BIT_MASK);
    }

    public static int max(int a, int b) {
        return (a >= b) ? a : b;
    }

    public static long max(long a, long b) {
        return (a >= b) ? a : b;
    }

    public static float max(float a, float b) {
        if(a != a)
            return a;
        if((a == 0.0f) && (b == 0.0f) && (Float.floatToRawIntBits(a) == Float.floatToRawIntBits(-0.0f)))
            return b;
        return (a >= b) ? a : b;
    }

    public static double max(double a, double b) {
        if(a != a)
            return a;
        if((a == 0.0d) && (b == 0.0d) && (Double.doubleToRawLongBits(a) == Double.doubleToRawLongBits(-0.0d)))
            return b;
        return (a >= b) ? a : b;
    }

    public static int min(int a, int b) {
        return (a <= b) ? a : b;
    }

    public static long min(long a, long b) {
        return (a <= b) ? a : b;
    }

    public static float min(float a, float b) {
        if(a != a)
            return a;
        if((a == 0.0f) && (b == 0.0f) && (Float.floatToRawIntBits(b) == Float.floatToRawIntBits(-0.0f)))
            return b;
        return (a <= b) ? a : b;
    }

    public static double min(double a, double b) {
        if(a != a)
            return a;
        if((a == 0.0d) && (b == 0.0d) && (Double.doubleToRawLongBits(b) == Double.doubleToRawLongBits(-0.0d)))
            return b;
        return (a <= b) ? a : b;
    }

    public static double signum(double d) {
        return (d == 0.0 || Double.isNaN(d)) ? d : copySign(1.0, d);
    }

    public static float signum(float f) {
        return (f == 0.0f || Float.isNaN(f)) ? f : copySign(1.0f, f);
    }

    public static native double sinh(double x);

    public static native double cosh(double x);

    public static native double tanh(double x);

    public static double hypot(double x, double y) {
        return sqrt((x * x) + (y * y));
    }

    public static double expm1(double x) {
        return exp(x) - 1;
    }

    public static double log1p(double x) {
        return Math.log(x + 1);
    }

    public static float copySign(float magnitude, float sign) {
        return Float.intBitsToFloat((Float.floatToRawIntBits(sign) & (FLOAT_SIGN_BIT_MASK)) |
                                    (Float.floatToRawIntBits(magnitude) & (FLOAT_EXP_BIT_MASK | FLOAT_SIGNIF_BIT_MASK)));
    }

    public static double copySign(double magnitude, double sign) {
        return Double.longBitsToDouble((Double.doubleToRawLongBits(sign) & (DOUBLE_SIGN_BIT_MASK)) |
                                       (Double.doubleToRawLongBits(magnitude) & (DOUBLE_EXP_BIT_MASK | DOUBLE_SIGNIF_BIT_MASK)));
    }

    public static int getExponent(float f) {
        return ((Float.floatToRawIntBits(f) & FLOAT_EXP_BIT_MASK) >> (FLOAT_SIGNIFICAND_WIDTH - 1)) - FLOAT_EXP_BIAS;
    }

    public static int getExponent(double d) {
        return (int)(((Double.doubleToRawLongBits(d) & DOUBLE_EXP_BIT_MASK) >> (DOUBLE_SIGNIFICAND_WIDTH - 1)) - DOUBLE_EXP_BIAS);
    }

    public static double nextAfter(double start, double direction) {
        if(start > direction) {
            if(start != 0.0d) {
                final long transducer = Double.doubleToRawLongBits(start);
                return Double.longBitsToDouble(transducer + ((transducer > 0L) ? -1L : 1L));
            }
            else
                return -Double.MIN_VALUE;
        }
        else if(start < direction) {
            final long transducer = Double.doubleToRawLongBits(start + 0.0d);
            return Double.longBitsToDouble(transducer + ((transducer >= 0L) ? 1L : -1L));
        }
        else if(start == direction)
            return direction;
        else
            return start + direction;
    }

    public static float nextAfter(float start, double direction) {
        if(start > direction) {
            if(start != 0.0f) {
                final int transducer = Float.floatToRawIntBits(start);
                return Float.intBitsToFloat(transducer + ((transducer > 0) ? -1 : 1));
            }
            else
                return -Float.MIN_VALUE;
        }
        else if(start < direction) {
            final int transducer = Float.floatToRawIntBits(start + 0.0f);
            return Float.intBitsToFloat(transducer + ((transducer >= 0) ? 1 : -1));
        }
        else if(start == direction)
            return (float)direction;
        else
            return start + (float)direction;
    }

    public static double nextUp(double d) {
        if(d < Double.POSITIVE_INFINITY) {
            final long transducer = Double.doubleToRawLongBits(d + 0.0D);
            return Double.longBitsToDouble(transducer + ((transducer >= 0L) ? 1L : -1L));
        }
        else
            return d;
    }

    public static float nextUp(float f) {
        if(f < Float.POSITIVE_INFINITY) {
            final int transducer = Float.floatToRawIntBits(f + 0.0F);
            return Float.intBitsToFloat(transducer + ((transducer >= 0) ? 1 : -1));
        }
        else
            return f;
    }

    public static double nextDown(double d) {
        if(Double.isNaN(d) || d == Double.NEGATIVE_INFINITY)
            return d;
        else {
            if(d == 0.0)
                return -Double.MIN_VALUE;
            else
                return Double.longBitsToDouble(Double.doubleToRawLongBits(d) + ((d > 0.0d)?-1L:+1L));
        }
    }

    public static float nextDown(float f) {
        if(Float.isNaN(f) || f == Float.NEGATIVE_INFINITY)
            return f;
        else {
            if(f == 0.0f)
                return -Float.MIN_VALUE;
            else
                return Float.intBitsToFloat(Float.floatToRawIntBits(f) + ((f > 0.0f)?-1:+1));
        }
    }

    public static double scalb(double d, int scaleFactor) {
        final int MAX_SCALE = Double.MAX_EXPONENT + -Double.MIN_EXPONENT + DOUBLE_SIGNIFICAND_WIDTH + 1;
        int exp_adjust = 0;
        int scale_increment = 0;
        double exp_delta = Double.NaN;

        if(scaleFactor < 0) {
            scaleFactor = Math.max(scaleFactor, -MAX_SCALE);
            scale_increment = -512;
            exp_delta = powerOfTwoD(-512);
        }
        else {
            scaleFactor = Math.min(scaleFactor, MAX_SCALE);
            scale_increment = 512;
            exp_delta = powerOfTwoD(512);
        }

        int t = (scaleFactor >> 9-1) >>> 32 - 9;
        exp_adjust = ((scaleFactor + t) & (512 -1)) - t;

        d *= powerOfTwoD(exp_adjust);
        scaleFactor -= exp_adjust;

        while(scaleFactor != 0) {
            d *= exp_delta;
            scaleFactor -= scale_increment;
        }
        return d;
    }

    public static float scalb(float f, int scaleFactor) {
        final int MAX_SCALE = Float.MAX_EXPONENT + -Float.MIN_EXPONENT + FLOAT_SIGNIFICAND_WIDTH + 1;
        scaleFactor = Math.max(Math.min(scaleFactor, MAX_SCALE), -MAX_SCALE);
        return (float)((double)f*powerOfTwoD(scaleFactor));
    }

    static double powerOfTwoD(int n) {
        // assert(n >= Double.MIN_EXPONENT && n <= Double.MAX_EXPONENT);
        return Double.longBitsToDouble((((long)n + (long)DOUBLE_EXP_BIAS) << (DOUBLE_SIGNIFICAND_WIDTH-1)) & DOUBLE_EXP_BIT_MASK);
    }

    static float powerOfTwoF(int n) {
        // assert(n >= Float.MIN_EXPONENT && n <= Float.MAX_EXPONENT);
        return Float.intBitsToFloat(((n + FLOAT_EXP_BIAS) << (FLOAT_SIGNIFICAND_WIDTH-1)) & FLOAT_EXP_BIT_MASK);
    }
}
