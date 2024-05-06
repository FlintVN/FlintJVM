package jdk.internal.math;

import java.io.IOException;

import static java.lang.Integer.*;
import static java.lang.Float.*;
import static java.lang.Math.multiplyHigh;
import static jdk.internal.math.MathUtils.*;

final public class FloatToDecimal {
    private final byte[] bytes = new byte[9 + 6];
    private int index;

    private FloatToDecimal() {

    }

    public static String toString(float v) {
        return new FloatToDecimal().toDecimalString(v);
    }

    public static Appendable appendTo(float v, Appendable app) throws IOException {
        return new FloatToDecimal().appendDecimalTo(v, app);
    }

    private String toDecimalString(float v) {
        return switch (toDecimal(v)) {
            case 0 -> charsToString();
            case 1 -> "0.0";
            case 2 -> "-0.0";
            case 3 -> "Infinity";
            case 4 -> "-Infinity";
            default -> "NaN";
        };
    }

    private Appendable appendDecimalTo(float v, Appendable app) throws IOException {
        switch (toDecimal(v)) {
            case 0:
                char[] chars = new char[index + 1];
                for (int i = 0; i < chars.length; ++i)
                    chars[i] = (char) bytes[i];
                if (app instanceof StringBuilder builder)
                    return builder.append(chars);
                if (app instanceof StringBuffer buffer)
                    return buffer.append(chars);
                for (char c : chars)
                    app.append(c);
                return app;
            case 1: return app.append("0.0");
            case 2: return app.append("-0.0");
            case 3: return app.append("Infinity");
            case 4: return app.append("-Infinity");
            default: return app.append("NaN");
        }
    }

    private int toDecimal(float v) {
        int bits = floatToRawIntBits(v);
        int t = bits & 0x7FFFFF;
        int bq = (bits >>> 24 - 1) & 0xFF;
        if (bq < 0xFF) {
            index = -1;
            if (bits < 0)
                append('-');
            if (bq != 0) {
                int mq = -(-149) + 1 - bq;
                int c = 0x800000 | t;
                if (0 < mq & mq < 24) {
                    int f = c >> mq;
                    if (f << mq == c)
                        return toChars(f, 0);
                }
                return toDecimal(-mq, c, 0);
            }
            if (t != 0)
                return t < 8 ? toDecimal((-149), 10 * t, -1) : toDecimal((-149), t, 0);
            return bits == 0 ? 1 : 2;
        }
        if (t != 0)
            return 5;
        return bits > 0 ? 3 : 4;
    }

    private int toDecimal(int q, int c, int dk) {
        int out = c & 0x1;
        long cb = c << 2;
        long cbr = cb + 2;
        long cbl;
        int k;

        if (c != 0x800000 | q == (-149)) {
            cbl = cb - 2;
            k = flog10pow2(q);
        }
        else {
            cbl = cb - 1;
            k = flog10threeQuartersPow2(q);
        }
        int h = q + flog2pow10(-k) + 33;

        long g = g1(k) + 1;

        int vb = rop(g, cb << h);
        int vbl = rop(g, cbl << h);
        int vbr = rop(g, cbr << h);

        int s = vb >> 2;
        if (s >= 100) {
            int sp10 = 10 * (int) (s * 1_717_986_919L >>> 34);
            int tp10 = sp10 + 10;
            boolean upin = vbl + out <= sp10 << 2;
            boolean wpin = (tp10 << 2) + out <= vbr;
            if (upin != wpin)
                return toChars(upin ? sp10 : tp10, k);
        }

        int t = s + 1;
        boolean uin = vbl + out <= s << 2;
        boolean win = (t << 2) + out <= vbr;
        if (uin != win)
            return toChars(uin ? s : t, k + dk);
        int cmp = vb - (s + t << 1);
        return toChars(cmp < 0 || cmp == 0 && (s & 0x1) == 0 ? s : t, k + dk);
    }

    private static int rop(long g, long cp) {
        long x1 = multiplyHigh(g, cp);
        long vbp = x1 >>> 31;
        return (int) (vbp | (x1 & 0xFFFFFFFFL) + 0xFFFFFFFFL >>> 32);
    }

    private int toChars(int f, int e) {
        int len = flog10pow2(32 - numberOfLeadingZeros(f));
        if (f >= pow10(len))
            len += 1;

        f *= (int)pow10(9 - len);
        e += len;

        int h = (int) (f * 1_441_151_881L >>> 57);
        int l = f - 100_000_000 * h;

        if (0 < e && e <= 7)
            return toChars1(h, l, e);
        if (-3 < e && e <= 0)
            return toChars2(h, l, e);
        return toChars3(h, l, e);
    }

    private int toChars1(int h, int l, int e) {
        appendDigit(h);
        int y = y(l);
        int t;
        int i = 1;
        for (; i < e; ++i) {
            t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
        append('.');
        for (; i <= 8; ++i) {
            t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
        removeTrailingZeroes();
        return 0;
    }

    private int toChars2(int h, int l, int e) {
        appendDigit(0);
        append('.');
        for (; e < 0; ++e)
          appendDigit(0);
        appendDigit(h);
        append8Digits(l);
        removeTrailingZeroes();
        return 0;
    }

    private int toChars3(int h, int l, int e) {
        appendDigit(h);
        append('.');
        append8Digits(l);
        removeTrailingZeroes();
        exponent(e - 1);
        return 0;
    }

    private void append8Digits(int m) {
        int y = y(m);
        for (int i = 0; i < 8; ++i) {
            int t = 10 * y;
            appendDigit(t >>> 28);
            y = t & 0x0FFFFFFF;
        }
    }

    private void removeTrailingZeroes() {
        while (bytes[index] == '0')
            --index;
        if (bytes[index] == '.')
            ++index;
    }

    private int y(int a) {
        return (int) (multiplyHigh((long) (a + 1) << 28, 193_428_131_138_340_668L) >>> 20) - 1;
    }

    private void exponent(int e) {
        append('E');
        if (e < 0) {
            append('-');
            e = -e;
        }
        if (e < 10) {
            appendDigit(e);
            return;
        }
        int d = e * 103 >>> 10;
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
