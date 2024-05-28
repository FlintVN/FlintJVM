package java.lang;

public final class Long extends Number implements Comparable<Long> {
    public static final long MIN_VALUE = 0x8000000000000000L;
    public static final long MAX_VALUE = 0x7fffffffffffffffL;
    public static final int SIZE = 64;
    public static final int BYTES = SIZE / Byte.SIZE;
    @SuppressWarnings("unchecked")
    public static final Class<Long> TYPE = (Class<Long>)Class.getPrimitiveClass("long");

    private final long value;

    public static String toString(long i, int radix) {
        if(radix < Character.MIN_RADIX || radix > Character.MAX_RADIX)
            radix = 10;

        if(radix == 10)
            return toString(i);

        byte[] buf = new byte[65];
        boolean negative = (i < 0);
        int charPos = buf.length - 1;

        if(!negative)
            i = -i;

        while(i <= -radix) {
            buf[charPos--] = (byte)Integer.digitToChar((int)-(i % radix));
            i = i / radix;
        }
        buf[charPos] = (byte)Integer.digitToChar((int)-i);

        if(negative)
            buf[--charPos] = '-';

        return String.newString(buf, charPos, (33 - charPos), (byte)0);
    }


    public static String toUnsignedString(long i, int radix) {
        if(i >= 0)
            return toString(i, radix);
        else {
            return switch(radix) {
                case 2  -> toBinaryString(i);
                case 4  -> toUnsignedString0(i, 2);
                case 8  -> toOctalString(i);
                case 10 -> {
                    long quot = (i >>> 1) / 5;
                    long rem = i - quot * 10;
                    yield toString(quot) + rem;
                }
                case 16 -> toHexString(i);
                case 32 -> toUnsignedString0(i, 5);
                default -> toUnsignedBigInteger(i).toString(radix);
            };
        }
    }

    private static BigInteger toUnsignedBigInteger(long i) {
        if(i >= 0L)
            return BigInteger.valueOf(i);
        else {
            int upper = (int) (i >>> 32);
            int lower = (int) i;

            BigInteger bint = BigInteger.valueOf(Integer.toUnsignedLong(upper));
            bint = bint.shiftLeft(32);
            bint = bint.add(BigInteger.valueOf(Integer.toUnsignedLong(lower)));
            return bint;
        }
    }

    public static String toHexString(long i) {
        return toUnsignedString0(i, 4);
    }

    public static String toOctalString(long i) {
        return toUnsignedString0(i, 3);
    }

    public static String toBinaryString(long i) {
        return toUnsignedString0(i, 1);
    }

    static String toUnsignedString0(long val, int shift) {
        int mag = Long.SIZE - Long.numberOfLeadingZeros(val);
        int chars = Math.max(((mag + (shift - 1)) / shift), 1);
        byte[] buf = new byte[chars];
        formatUnsignedLong0(val, shift, buf, 0, chars);
        return String.newString(buf, (byte)0);
    }

    private static void formatUnsignedLong0(long val, int shift, byte[] buf, int offset, int len) {
        int charPos = offset + len;
        int radix = 1 << shift;
        int mask = radix - 1;
        do {
            buf[--charPos] = (byte)Integer.digitToChar(((int) val) & mask);
            val >>>= shift;
        } while(charPos > offset);
    }

    static String fastUUID(long lsb, long msb) {
        byte[] buf = new byte[Character.MAX_RADIX];
        formatUnsignedLong0(lsb,        4, buf, 24, 12);
        formatUnsignedLong0(lsb >>> 48, 4, buf, 19, 4);
        formatUnsignedLong0(msb,        4, buf, 14, 4);
        formatUnsignedLong0(msb >>> 16, 4, buf, 9,  4);
        formatUnsignedLong0(msb >>> 32, 4, buf, 0,  8);

        buf[23] = '-';
        buf[18] = '-';
        buf[13] = '-';
        buf[8]  = '-';

        return String.newString(buf, (byte)0);
    }

    public static String toString(long i) {
        byte[] buffer = new byte[21];
        int index = buffer.length - 1;
        boolean negative;
        if(i < 0) {
            negative = true;
            i = -i;
        }
        else
            negative = false;

        do {
            buffer[--index] = (byte)((i % 10) + 48);
            i /= 10;
        } while(i > 0);

        if(negative)
            buffer[--index] = '-';

        return String.newString(buffer, index, buffer.length - index, (byte)0);
    }

    public static String toUnsignedString(long i) {
        return toUnsignedString(i, 10);
    }

    public static long parseLong(String s, int radix) throws NumberFormatException {
        return parseLong(s, 0, s.length(), radix);
    }

    public static long parseLong(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
        if(s == null)
            throw new NumberFormatException("Cannot parse null string");
        if(radix < Character.MIN_RADIX)
            throw new NumberFormatException("radix " + radix + " less than 2");
        if(radix > Character.MAX_RADIX)
            throw new NumberFormatException("radix " + radix + " greater than 36");

        boolean negative = false;
        int i = beginIndex;
        long limit = -Long.MAX_VALUE;

        if(i < endIndex) {
            char firstChar = s.charAt(i);
            if(firstChar < '0') {
                if(firstChar == '-') {
                    negative = true;
                    limit = Long.MIN_VALUE;
                }
                else if(firstChar != '+')
                    throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
                i++;
            }
            if(i >= endIndex)
                throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
            long multmin = limit / radix;
            long result = 0;
            while(i < endIndex) {
                int digit = Character.digit(s.charAt(i), radix);
                if(digit < 0 || result < multmin)
                    throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
                result *= radix;
                if(result < limit + digit)
                    throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
                i++;
                result -= digit;
            }
            return negative ? result : -result;
        }
        else
            throw new NumberFormatException("");
    }

    public static long parseLong(String s) throws NumberFormatException {
        return parseLong(s, 10);
    }

    public static long parseUnsignedLong(String s, int radix) throws NumberFormatException {
        if(s == null)
            throw new NumberFormatException("Cannot parse null string");

        int len = s.length();
        if(len > 0) {
            char firstChar = s.charAt(0);
            if(firstChar == '-')
                throw new NumberFormatException("Illegal leading minus sign on unsigned string " + s);
            else {
                if(len <= 12 || (radix == 10 && len <= 18) )
                    return parseLong(s, radix);

                long first = parseLong(s, 0, len - 1, radix);
                int second = Character.digit(s.charAt(len - 1), radix);
                if(second < 0)
                    throw new NumberFormatException("Bad digit at end of " + s);
                long result = first * radix + second;

                int guard = radix * (int) (first >>> 57);
                if(guard >= 128 || (result >= 0 && guard >= 128 - 36))
                    throw new NumberFormatException("String value " + s + " exceeds range of unsigned long.");
                return result;
            }
        }
        else
            throw NumberFormatException.forInputString(s, radix);
    }

    public static long parseUnsignedLong(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
        int start = beginIndex, len = endIndex - beginIndex;

        if(len > 0) {
            char firstChar = s.charAt(start);
            if(firstChar == '-')
                throw new NumberFormatException("Illegal leading minus sign on unsigned string " + s.subSequence(start, start + len));
            else {
                if(len <= 12 || (radix == 10 && len <= 18) )
                    return parseLong(s, start, start + len, radix);

                long first = parseLong(s, start, start + len - 1, radix);
                int second = Character.digit(s.charAt(start + len - 1), radix);
                if(second < 0)
                    throw new NumberFormatException("Bad digit at end of " + s.subSequence(start, start + len));
                long result = first * radix + second;

                int guard = radix * (int) (first >>> 57);
                if(guard >= 128 || (result >= 0 && guard >= 128 - 36))
                    throw new NumberFormatException("String value " + s.subSequence(start, start + len) + " exceeds range of unsigned long.");
                return result;
            }
        }
        else
            throw NumberFormatException.forInputString("", radix);
    }

    public static long parseUnsignedLong(String s) throws NumberFormatException {
        return parseUnsignedLong(s, 10);
    }

    public static Long valueOf(String s, int radix) throws NumberFormatException {
        return Long.valueOf(parseLong(s, radix));
    }

    public static Long valueOf(String s) throws NumberFormatException {
        return Long.valueOf(parseLong(s, 10));
    }

    public static Long valueOf(long l) {
        return new Long(l);
    }

    public static Long decode(String nm) throws NumberFormatException {
        int radix = 10;
        int index = 0;
        boolean negative = false;
        long result;

        if(nm.isEmpty())
            throw new NumberFormatException("Zero length string");
        char firstChar = nm.charAt(0);
        if(firstChar == '-') {
            negative = true;
            index++;
        }
        else if(firstChar == '+')
            index++;

        if(nm.startsWith("0x", index) || nm.startsWith("0X", index)) {
            index += 2;
            radix = 16;
        }
        else if(nm.startsWith("#", index)) {
            index ++;
            radix = 16;
        }
        else if(nm.startsWith("0", index) && nm.length() > 1 + index) {
            index ++;
            radix = 8;
        }

        if(nm.startsWith("-", index) || nm.startsWith("+", index))
            throw new NumberFormatException("Sign character in wrong position");

        try {
            result = parseLong(nm, index, nm.length(), radix);
            result = negative ? -result : result;
        }
        catch(NumberFormatException e) {
            String constant = negative ? ("-" + nm.substring(index)) : nm.substring(index);
            result = parseLong(constant, radix);
        }
        return result;
    }

    public Long(long value) {
        this.value = value;
    }

    public Long(String s) throws NumberFormatException {
        this.value = parseLong(s, 10);
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
        return value;
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
        return toString(value);
    }

    @Override
    public int hashCode() {
        return Long.hashCode(value);
    }

    public static int hashCode(long value) {
        return (int)(value ^ (value >>> 32));
    }

    @Override
    public boolean equals(Object obj) {
        if(obj instanceof Long)
            return value == ((Long)obj).longValue();
        return false;
    }

    @Override
    public int compareTo(Long anotherLong) {
        return compare(this.value, anotherLong.value);
    }

    public static int compare(long x, long y) {
        return (x < y) ? -1 : ((x == y) ? 0 : 1);
    }

    public static int compareUnsigned(long x, long y) {
        return compare(x + MIN_VALUE, y + MIN_VALUE);
    }

    public static long divideUnsigned(long dividend, long divisor) {
        if(divisor >= 0) {
            final long q = (dividend >>> 1) / divisor << 1;
            final long r = dividend - q * divisor;
            return q + ((r | ~(r - divisor)) >>> (Long.SIZE - 1));
        }
        return (dividend & ~(dividend - divisor)) >>> (Long.SIZE - 1);
    }

    public static long remainderUnsigned(long dividend, long divisor) {
        if(divisor >= 0) {
            final long q = (dividend >>> 1) / divisor << 1;
            final long r = dividend - q * divisor;
            return r - ((~(r - divisor) >> (Long.SIZE - 1)) & divisor);
        }
        return dividend - (((dividend & ~(dividend - divisor)) >> (Long.SIZE - 1)) & divisor);
    }

    public static long highestOneBit(long i) {
        return i & (MIN_VALUE >>> numberOfLeadingZeros(i));
    }

    public static long lowestOneBit(long i) {
        return i & -i;
    }

    public static int numberOfLeadingZeros(long i) {
        int x = (int)(i >>> 32);
        return x == 0 ? 32 + Integer.numberOfLeadingZeros((int)i)
                : Integer.numberOfLeadingZeros(x);
    }

    public static int numberOfTrailingZeros(long i) {
        int x = (int)i;
        return x == 0 ? 32 + Integer.numberOfTrailingZeros((int)(i >>> 32)) : Integer.numberOfTrailingZeros(x);
    }

     public static int bitCount(long i) {
        i = i - ((i >>> 1) & 0x5555555555555555L);
        i = (i & 0x3333333333333333L) + ((i >>> 2) & 0x3333333333333333L);
        i = (i + (i >>> 4)) & 0x0f0f0f0f0f0f0f0fL;
        i = i + (i >>> 8);
        i = i + (i >>> 16);
        i = i + (i >>> 32);
        return (int)i & 0x7f;
     }

    public static long rotateLeft(long i, int distance) {
        return (i << distance) | (i >>> -distance);
    }

    public static long rotateRight(long i, int distance) {
        return (i >>> distance) | (i << -distance);
    }

    public static long reverse(long i) {
        i = (i & 0x5555555555555555L) << 1 | (i >>> 1) & 0x5555555555555555L;
        i = (i & 0x3333333333333333L) << 2 | (i >>> 2) & 0x3333333333333333L;
        i = (i & 0x0f0f0f0f0f0f0f0fL) << 4 | (i >>> 4) & 0x0f0f0f0f0f0f0f0fL;

        return reverseBytes(i);
    }

    public static long compress(long i, long mask) {
        i = i & mask;
        long maskCount = ~mask << 1;

        for(int j = 0; j < 6; j++) {
            long maskPrefix = parallelSuffix(maskCount);
            long maskMove = maskPrefix & mask;
            mask = (mask ^ maskMove) | (maskMove >>> (1 << j));
            long t = i & maskMove;
            i = (i ^ t) | (t >>> (1 << j));
            maskCount = maskCount & ~maskPrefix;
        }
        return i;
    }

    public static long expand(long i, long mask) {
        long originalMask = mask;
        long maskCount = ~mask << 1;
        long maskPrefix = parallelSuffix(maskCount);
        long maskMove1 = maskPrefix & mask;
        mask = (mask ^ maskMove1) | (maskMove1 >>> (1 << 0));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        long maskMove2 = maskPrefix & mask;
        mask = (mask ^ maskMove2) | (maskMove2 >>> (1 << 1));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        long maskMove3 = maskPrefix & mask;
        mask = (mask ^ maskMove3) | (maskMove3 >>> (1 << 2));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        long maskMove4 = maskPrefix & mask;
        mask = (mask ^ maskMove4) | (maskMove4 >>> (1 << 3));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        long maskMove5 = maskPrefix & mask;
        mask = (mask ^ maskMove5) | (maskMove5 >>> (1 << 4));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        long maskMove6 = maskPrefix & mask;

        long t = i << (1 << 5);
        i = (i & ~maskMove6) | (t & maskMove6);
        t = i << (1 << 4);
        i = (i & ~maskMove5) | (t & maskMove5);
        t = i << (1 << 3);
        i = (i & ~maskMove4) | (t & maskMove4);
        t = i << (1 << 2);
        i = (i & ~maskMove3) | (t & maskMove3);
        t = i << (1 << 1);
        i = (i & ~maskMove2) | (t & maskMove2);
        t = i << (1 << 0);
        i = (i & ~maskMove1) | (t & maskMove1);

        return i & originalMask;
    }

    private static long parallelSuffix(long maskCount) {
        long maskPrefix = maskCount ^ (maskCount << 1);
        maskPrefix = maskPrefix ^ (maskPrefix << 2);
        maskPrefix = maskPrefix ^ (maskPrefix << 4);
        maskPrefix = maskPrefix ^ (maskPrefix << 8);
        maskPrefix = maskPrefix ^ (maskPrefix << 16);
        maskPrefix = maskPrefix ^ (maskPrefix << 32);
        return maskPrefix;
    }

    public static int signum(long i) {
        return (int) ((i >> 63) | (-i >>> 63));
    }

    public static long reverseBytes(long i) {
        i = (i & 0x00ff00ff00ff00ffL) << 8 | (i >>> 8) & 0x00ff00ff00ff00ffL;
        return (i << 48) | ((i & 0xffff0000L) << 16) | ((i >>> 16) & 0xffff0000L) | (i >>> 48);
    }

    public static long sum(long a, long b) {
        return a + b;
    }

    public static long max(long a, long b) {
        return Math.max(a, b);
    }

    public static long min(long a, long b) {
        return Math.min(a, b);
    }
}
