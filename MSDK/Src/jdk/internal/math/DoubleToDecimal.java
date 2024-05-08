package jdk.internal.math;

import java.io.IOException;

import static java.lang.Long.*;
import static java.lang.Double.*;
import static java.lang.Math.multiplyHigh;
import static jdk.internal.math.MathUtils.*;

final public class DoubleToDecimal {
    private final byte[] bytes = new byte[17 + 7];

    private int index;

    private DoubleToDecimal() {

    }

    public static String toString(double v) {
        return new DoubleToDecimal().toDecimalString(v);
    }

    public static Appendable appendTo(double v, Appendable app) throws IOException {
        return new DoubleToDecimal().appendDecimalTo(v, app);
    }

    private String toDecimalString(double v) {
        return switch(toDecimal(v)) {
            case 0 -> charsToString();
            case 1 -> "0.0";
            case 2 -> "-0.0";
            case 3 -> "Infinity";
            case 4 -> "-Infinity";
            default -> "NaN";
        };
    }

    private Appendable appendDecimalTo(double v, Appendable app) throws IOException {
        switch(toDecimal(v)) {
            case 0:
                char[] chars = new char[index + 1];
                for(int i = 0; i < chars.length; ++i)
                    chars[i] = (char) bytes[i];
                if(app instanceof StringBuilder builder)
                    return builder.append(chars);
                if(app instanceof StringBuffer buffer)
                    return buffer.append(chars);
                for(char c : chars)
                    app.append(c);
                return app;
            case 1: return app.append("0.0");
            case 2: return app.append("-0.0");
            case 3: return app.append("Infinity");
            case 4: return app.append("-Infinity");
            default: return app.append("NaN");
        }
    }

    private int toDecimal(double v) {
        long bits = doubleToRawLongBits(v);
        long t = bits & 0x0FFFFFFFFFFFFFL;
        int bq = (int) (bits >>> 53 - 1) & 0x07FF;
        if(bq < 0x07FF) {
            index = -1;
            if(bits < 0)
                append('-');
            if(bq != 0) {
                int mq = -(-1074) + 1 - bq;
                long c = 0x10000000000000L | t;
                if(0 < mq & mq < 53) {
                    long f = c >> mq;
                    if(f << mq == c)
                        return toChars(f, 0);
                }
                return toDecimal(-mq, c, 0);
            }
            if(t != 0)
                return t < 3 ? toDecimal((-1074), 10 * t, -1) : toDecimal((-1074), t, 0);
            return bits == 0 ? 1 : 2;
        }
        if(t != 0)
            return 5;
        return bits > 0 ? 3 : 4;
    }

    private int toDecimal(int q, long c, int dk) {
        int out = (int) c & 0x1;
        long cb = c << 2;
        long cbr = cb + 2;
        long cbl;
        int k;

        if(c != 0x10000000000000L | q == (-1074)) {
            cbl = cb - 2;
            k = flog10pow2(q);
        }
        else {
            cbl = cb - 1;
            k = flog10threeQuartersPow2(q);
        }
        int h = q + flog2pow10(-k) + 2;

        long g1 = g1(k);
        long g0 = g0(k);

        long vb = rop(g1, g0, cb << h);
        long vbl = rop(g1, g0, cbl << h);
        long vbr = rop(g1, g0, cbr << h);

        long s = vb >> 2;
        if(s >= 100) {
            long sp10 = 10 * multiplyHigh(s, 115_292_150_460_684_698L << 4);
            long tp10 = sp10 + 10;
            boolean upin = vbl + out <= sp10 << 2;
            boolean wpin = (tp10 << 2) + out <= vbr;
            if(upin != wpin)
                return toChars(upin ? sp10 : tp10, k);
        }

        long t = s + 1;
        boolean uin = vbl + out <= s << 2;
        boolean win = (t << 2) + out <= vbr;
        if(uin != win)
            return toChars(uin ? s : t, k + dk);
        long cmp = vb - (s + t << 1);
        return toChars(cmp < 0 || cmp == 0 && (s & 0x1) == 0 ? s : t, k + dk);
    }

    private static long rop(long g1, long g0, long cp) {
        long x1 = multiplyHigh(g0, cp);
        long y0 = g1 * cp;
        long y1 = multiplyHigh(g1, cp);
        long z = (y0 >>> 1) + x1;
        long vbp = y1 + (z >>> 63);
        return vbp | (z & 0x7FFFFFFFFFFFFFFFL) + 0x7FFFFFFFFFFFFFFFL >>> 63;
    }

    private int toChars(long f, int e) {
        int len = flog10pow2(64 - numberOfLeadingZeros(f));
        if(f >= pow10(len))
            len += 1;

        f *= pow10(17 - len);
        e += len;

        long hm = multiplyHigh(f, 193_428_131_138_340_668L) >>> 20;
        int l = (int) (f - 100_000_000L * hm);
        int h = (int) (hm * 1_441_151_881L >>> 57);
        int m = (int) (hm - 100_000_000 * h);

        if(0 < e && e <= 7)
            return toChars1(h, m, l, e);
        if(-3 < e && e <= 0)
            return toChars2(h, m, l, e);
        return toChars3(h, m, l, e);
    }

    private int toChars1(int h, int m, int l, int e) {
        appendDigit(h);
        int y = y(m);
        int t;
        int i = 1;
        for(; i < e; ++i) {
            t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
        append('.');
        for(; i <= 8; ++i) {
            t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
        lowDigits(l);
        return 0;
    }

    private int toChars2(int h, int m, int l, int e) {
        appendDigit(0);
        append('.');
        for(; e < 0; ++e)
            appendDigit(0);
        appendDigit(h);
        append8Digits(m);
        lowDigits(l);
        return 0;
    }

    private int toChars3(int h, int m, int l, int e) {
        appendDigit(h);
        append('.');
        append8Digits(m);
        lowDigits(l);
        exponent(e - 1);
        return 0;
    }

    private void lowDigits(int l) {
        if(l != 0)
            append8Digits(l);
        removeTrailingZeroes();
    }

    private void append8Digits(int m) {
        int y = y(m);
        for(int i = 0; i < 8; ++i) {
            int t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
    }

    private void removeTrailingZeroes() {
        while(bytes[index] == '0')
            --index;
        if(bytes[index] == '.')
            ++index;
    }

    private int y(int a) {
        return (int) (multiplyHigh((long) (a + 1) << 28, 193_428_131_138_340_668L) >>> 20) - 1;
    }

    private void exponent(int e) {
        append('E');
        if(e < 0) {
            append('-');
            e = -e;
        }
        if(e < 10) {
            appendDigit(e);
            return;
        }
        int d;
        if(e >= 100) {
            d = e * 1_311 >>> 17;
            appendDigit(d);
            e -= 100 * d;
        }
        d = e * 103 >>> 10;
        appendDigit(d);
        appendDigit(e - 10 * d);
    }

    private void append(int c) {
        bytes[++index] = (byte) c;
    }

    private void appendDigit(int d) {
        bytes[++index] = (byte) ('0' + d);
    }

    private String charsToString() {
        return new String(bytes, 0, index + 1);
    }
}
