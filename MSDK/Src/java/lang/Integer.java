package java.lang;

public final class Integer extends Number implements Comparable<Integer> {
    public static final int SIZE = 32;
    public static final int BYTES = SIZE / Byte.SIZE;
    public static final int MIN_VALUE = 0x80000000;
    public static final int MAX_VALUE = 0x7fffffff;
    public static final Class<Integer> TYPE = (Class<Integer>)Class.getPrimitiveClass("int");

    private final int value;

    static char digitToChar(int i) {
        if(0 <= i && i < 10)
            return (char)(i + '0');
        else if(10 <= i && i < 36)
            return (char)(i + 'a');
        throw new NumberFormatException("Cannot convert number " + i + " to char");
    }

    public static String toString(int i, int radix) {
        if(radix < 2 || radix > 36)
            radix = 10;

        if(radix == 10)
            return toString(i);

        byte[] buf = new byte[33];
        boolean negative = (i < 0);
        int charPos = buf.length - 1;

        if(!negative)
            i = -i;

        while(i <= -radix) {
            buf[charPos--] = (byte)digitToChar(-(i % radix));
            i = i / radix;
        }
        buf[charPos] = (byte)digitToChar(-i);

        if(negative)
            buf[--charPos] = '-';

        return String.newString(buf, charPos, (33 - charPos), (byte)0);
    }

    public static String toUnsignedString(int i, int radix) {
        return Long.toUnsignedString(toUnsignedLong(i), radix);
    }

    public static String toHexString(int i) {
        return toUnsignedString0(i, 4);
    }

    public static String toOctalString(int i) {
        return toUnsignedString0(i, 3);
    }

    public static String toBinaryString(int i) {
        return toUnsignedString0(i, 1);
    }

    private static String toUnsignedString0(int val, int shift) {
        int mag = Integer.SIZE - Integer.numberOfLeadingZeros(val);
        int chars = Math.max(((mag + (shift - 1)) / shift), 1);
        byte[] buf = new byte[chars];
        formatUnsignedInt(val, shift, buf, chars);
        return String.newString(buf, (byte)0);
    }

    private static void formatUnsignedInt(int val, int shift, byte[] buf, int len) {
        int charPos = len;
        int radix = 1 << shift;
        int mask = radix - 1;
        do {
            buf[--charPos] = (byte)digitToChar(val & mask);
            val >>>= shift;
        } while(charPos > 0);
    }

    public static String toString(int i) {
        byte[] buffer = new byte[11];
        int index = buffer.length - 1;
        boolean negative;
        if(i < 0) {
            negative = true;
            i = -i;
        }
        else
            negative = false;

        do {
            buffer[--index] = (byte)((i % 10) + '0');
            i /= 10;
        } while(i > 0);

        if(negative)
            buffer[--index] = '-';

        return String.newString(buffer, index, buffer.length - index, (byte)0);
    }

    public static String toUnsignedString(int i) {
        return Long.toString(toUnsignedLong(i));
    }

    public static int parseInt(String s, int radix) throws NumberFormatException {
        return parseInt(s, 0, s.length(), radix);
    }

    public static int parseInt(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
        if(s == null)
            throw new NumberFormatException("Cannot parse null string");
        if(radix < 2)
            throw new NumberFormatException("radix " + radix + " less than 2");
        if(radix > 36)
            throw new NumberFormatException("radix " + radix + " greater than 36");

        boolean negative = false;
        int i = beginIndex;
        int limit = -Integer.MAX_VALUE;

        if(i < endIndex) {
            char firstChar = s.charAt(i);
            if(firstChar < '0') {
                if(firstChar == '-') {
                    negative = true;
                    limit = Integer.MIN_VALUE;
                }
                else if(firstChar != '+')
                    throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
                i++;
                if(i == endIndex)
                    throw NumberFormatException.forCharSequence(s, beginIndex, endIndex, i);
            }
            int multmin = limit / radix;
            int result = 0;
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
            throw NumberFormatException.forInputString("", radix);
    }

    public static int parseInt(String s) throws NumberFormatException {
        return parseInt(s, 10);
    }

    public static int parseUnsignedInt(String s, int radix) throws NumberFormatException {
        if(s == null)
            throw new NumberFormatException("Cannot parse null string");

        int len = s.length();
        if(len > 0) {
            char firstChar = s.charAt(0);
            if(firstChar == '-')
                throw new NumberFormatException("Illegal leading minus sign on unsigned string " + s);
            else if(len <= 5 || (radix == 10 && len <= 9))
                return parseInt(s, radix);
            else {
                long ell = Long.parseLong(s, radix);
                if((ell & 0xffff_ffff_0000_0000L) == 0)
                    return (int) ell;
                else
                    throw new NumberFormatException("String value " + s + " exceeds range of unsigned int.");
            }
        }
        else
            throw NumberFormatException.forInputString(s, radix);
    }

    public static int parseUnsignedInt(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
        int start = beginIndex, len = endIndex - beginIndex;

        if(len > 0) {
            char firstChar = s.charAt(start);
            if(firstChar == '-')
                throw new NumberFormatException("Illegal leading minus sign on unsigned string " + s);
            else if(len <= 5 || (radix == 10 && len <= 9))
                return parseInt(s, start, start + len, radix);
            else {
                long ell = Long.parseLong(s, start, start + len, radix);
                if((ell & 0xffff_ffff_0000_0000L) == 0)
                    return (int)ell;
                else
                    throw new NumberFormatException("String value " + s + " exceeds range of unsigned int.");
            }
        }
        else
            throw new NumberFormatException("");
    }

    public static int parseUnsignedInt(String s) throws NumberFormatException {
        return parseUnsignedInt(s, 10);
    }

    public static Integer valueOf(String s, int radix) throws NumberFormatException {
        return Integer.valueOf(parseInt(s, radix));
    }

    public static Integer valueOf(String s) throws NumberFormatException {
        return Integer.valueOf(parseInt(s, 10));
    }

    public static Integer valueOf(int i) {
        return new Integer(i);
    }

    public Integer(int value) {
        this.value = value;
    }

    public Integer(String s) throws NumberFormatException {
        this.value = parseInt(s, 10);
    }

    public byte byteValue() {
        return (byte)value;
    }

    public short shortValue() {
        return (short)value;
    }

    public int intValue() {
        return value;
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
        return toString(value);
    }

    public int hashCode() {
        return Integer.hashCode(value);
    }

    public static int hashCode(int value) {
        return value;
    }

    public boolean equals(Object obj) {
        if(obj instanceof Integer)
            return value == ((Integer)obj).intValue();
        return false;
    }

    public static Integer decode(String nm) throws NumberFormatException {
        int radix = 10;
        int index = 0;
        boolean negative = false;
        int result;

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
            result = parseInt(nm, index, nm.length(), radix);
            result = negative ? -result : result;
        }
        catch(NumberFormatException e) {
            String constant = negative ? ("-" + nm.substring(index)) : nm.substring(index);
            result = parseInt(constant, radix);
        }
        return result;
    }

    public int compareTo(Integer anotherInteger) {
        return compare(this.value, anotherInteger.value);
    }

    public static int compare(int x, int y) {
        return (x < y) ? -1 : ((x == y) ? 0 : 1);
    }

    public static int compareUnsigned(int x, int y) {
        return compare(x + MIN_VALUE, y + MIN_VALUE);
    }

    public static long toUnsignedLong(int x) {
        return ((long) x) & 0xffffffffL;
    }

    public static int divideUnsigned(int dividend, int divisor) {
        return (int)(toUnsignedLong(dividend) / toUnsignedLong(divisor));
    }

    public static int remainderUnsigned(int dividend, int divisor) {
        return (int)(toUnsignedLong(dividend) % toUnsignedLong(divisor));
    }

    public static int highestOneBit(int i) {
        return i & (MIN_VALUE >>> numberOfLeadingZeros(i));
    }

    public static int lowestOneBit(int i) {
        return i & -i;
    }

    public static int numberOfLeadingZeros(int i) {
        if(i <= 0)
            return i == 0 ? 32 : 0;
        int n = 31;
        if(i >= 1 << 16) { n -= 16; i >>>= 16; }
        if(i >= 1 <<  8) { n -=  8; i >>>=  8; }
        if(i >= 1 <<  4) { n -=  4; i >>>=  4; }
        if(i >= 1 <<  2) { n -=  2; i >>>=  2; }
        return n - (i >>> 1);
    }

    public static int numberOfTrailingZeros(int i) {
        i = ~i & (i - 1);
        if(i <= 0) return i & 32;
        int n = 1;
        if(i > 1 << 16) { n += 16; i >>>= 16; }
        if(i > 1 <<  8) { n +=  8; i >>>=  8; }
        if(i > 1 <<  4) { n +=  4; i >>>=  4; }
        if(i > 1 <<  2) { n +=  2; i >>>=  2; }
        return n + (i >>> 1);
    }

    public static int bitCount(int i) {
        i = i - ((i >>> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >>> 2) & 0x33333333);
        i = (i + (i >>> 4)) & 0x0f0f0f0f;
        i = i + (i >>> 8);
        i = i + (i >>> 16);
        return i & 0x3f;
    }

    public static int rotateLeft(int i, int distance) {
        return (i << distance) | (i >>> -distance);
    }

    public static int rotateRight(int i, int distance) {
        return (i >>> distance) | (i << -distance);
    }

    public static int reverse(int i) {
        i = (i & 0x55555555) << 1 | (i >>> 1) & 0x55555555;
        i = (i & 0x33333333) << 2 | (i >>> 2) & 0x33333333;
        i = (i & 0x0f0f0f0f) << 4 | (i >>> 4) & 0x0f0f0f0f;

        return reverseBytes(i);
    }

    public static int compress(int i, int mask) {
        i = i & mask;
        int maskCount = ~mask << 1;

        for(int j = 0; j < 5; j++) {
            int maskPrefix = parallelSuffix(maskCount);
            int maskMove = maskPrefix & mask;
            mask = (mask ^ maskMove) | (maskMove >>> (1 << j));
            int t = i & maskMove;
            i = (i ^ t) | (t >>> (1 << j));
            maskCount = maskCount & ~maskPrefix;
        }
        return i;
    }

    public static int expand(int i, int mask) {
        int originalMask = mask;
        int maskCount = ~mask << 1;
        int maskPrefix = parallelSuffix(maskCount);
        int maskMove1 = maskPrefix & mask;
        mask = (mask ^ maskMove1) | (maskMove1 >>> (1 << 0));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        int maskMove2 = maskPrefix & mask;
        mask = (mask ^ maskMove2) | (maskMove2 >>> (1 << 1));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        int maskMove3 = maskPrefix & mask;
        mask = (mask ^ maskMove3) | (maskMove3 >>> (1 << 2));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        int maskMove4 = maskPrefix & mask;
        mask = (mask ^ maskMove4) | (maskMove4 >>> (1 << 3));
        maskCount = maskCount & ~maskPrefix;

        maskPrefix = parallelSuffix(maskCount);
        int maskMove5 = maskPrefix & mask;

        int t = i << (1 << 4);
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

    private static int parallelSuffix(int maskCount) {
        int maskPrefix = maskCount ^ (maskCount << 1);
        maskPrefix = maskPrefix ^ (maskPrefix << 2);
        maskPrefix = maskPrefix ^ (maskPrefix << 4);
        maskPrefix = maskPrefix ^ (maskPrefix << 8);
        maskPrefix = maskPrefix ^ (maskPrefix << 16);
        return maskPrefix;
    }

    public static int signum(int i) {
        return (i >> 31) | (-i >>> 31);
    }

    public static int reverseBytes(int i) {
        return (i << 24) | ((i & 0xff00) << 8) | ((i >>> 8) & 0xff00) | (i >>> 24);
    }

    public static int sum(int a, int b) {
        return a + b;
    }

    public static int max(int a, int b) {
        return Math.max(a, b);
    }

    public static int min(int a, int b) {
        return Math.min(a, b);
    }
}
