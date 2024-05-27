package java.lang;

final class StringUTF16 {
    public static char charAt(byte[] value, int index) {
        index <<= 1;
        return (char)((value[index + 1] << 8) | (value[index] & 0xFF));
    }

    public static void putCharAt(byte[] value, int index, char c) {
        index <<= 1;
        value[index] = (byte)c;
        value[index + 1] = (byte)(c >>> 8);
    }

    public static void putCharAt(byte[] value, int index, byte c) {
        index <<= 1;
        value[index] = c;
        value[index + 1] = 0;
    }

    public static int indexOf(byte[] value, int ch, int fromIndex) {
        if(ch > 65535)
            return -1;
        for(int i = fromIndex; i < value.length; i++) {
            if(ch == charAt(value, i))
                return i;
        }
        return -1;
    }

    public static int indexOf(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return 0;
        if(value.length == 0)
            return -1;
        int valueCount = value.length >>> 1;
        int strCount = str.length >>> 1;
        char first = charAt(str, 0);
        int max = (valueCount - strCount);
        for(int i = fromIndex; i <= max; i++) {
            if(charAt(value, i) != first)
                while(++i <= max && charAt(value, i) != first);
            if(i <= max) {
                int j = i + 1;
                int end = j + strCount - 1;
                for(int k = 1; j < end && charAt(value, j) == charAt(str, k); j++, k++);
                if(j == end)
                    return i;
            }
        }
        return -1;
    }

    public static int indexOfLatin1(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return 0;
        if(value.length == 0)
            return -1;
        int valueCount = value.length >>> 1;
        int strCount = str.length;
        char first = (char)str[0];
        int max = (valueCount - strCount);
        for(int i = fromIndex; i <= max; i++) {
            if(charAt(value, i) != first)
                while(++i <= max && charAt(value, i) != first);
            if(i <= max) {
                int j = i + 1;
                int end = j + strCount - 1;
                for(int k = 1; j < end && charAt(value, j) == (char)str[k]; j++, k++);
                if(j == end)
                    return i;
            }
        }
        return -1;
    }

    public static int lastIndexOf(byte[] value, int ch, int fromIndex) {
        if(ch > 65535)
            return -1;
        for(int i = fromIndex; i >= 0; i--) {
            if(ch == charAt(value, i))
                return i;
        }
        return -1;
    }

    public static int lastIndexOf(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return value.length - 1;
        if(value.length < str.length)
            return -1;
        int strCount = str.length >>> 1;
        for(int i = fromIndex - (strCount - 1); i >= 0; i--) {
            if(charAt(value, i) == charAt(str, 0)) {
                boolean found = true;
                for(int j = 1; j < strCount; j++) {
                    if(charAt(value, i + j) != charAt(str, j)) {
                        found = false;
                        break;
                    }
                }
                if(found)
                    return i;
            }
        }
        return -1;
    }

    public static int lastIndexOfLatin1(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return value.length - 1;
        if(value.length < str.length)
            return -1;
        int strCount = str.length >>> 1;
        for(int i = fromIndex - (strCount - 1); i >= 0; i--) {
            if(charAt(value, i) == str[0]) {
                boolean found = true;
                for(int j = 1; j < strCount; j++) {
                    if(charAt(value, i + j) != str[j]) {
                        found = false;
                        break;
                    }
                }
                if(found)
                    return i;
            }
        }
        return -1;
    }

    public static String replace(byte[] value, char oldChar, char newChar) {
        int i = 0;
        int len = value.length >>> 1;
        byte[] ret = null;
        for(; i < len; i++) {
            char c = charAt(value, i);
            if(c == oldChar) {
                ret = new byte[len << 1];
                System.arraycopy(value, 0, ret, 0, i << 1);
                int index = i << 1;
                ret[index] = (byte)newChar;
                ret[index + 1] = (byte)(newChar >>> 8);
                break;
            }
        }
        for(; i < len; i++) {
            char c = charAt(value, i);
            int index = i << 1;
            if(c == oldChar) {
                ret[index] = (byte)newChar;
                ret[index + 1] = (byte)(newChar >>> 8);
            }
            else {
                ret[index] = (byte)c;
                ret[index + 1] = (byte)(c >>> 8);
            }
        }
        if(ret != null) {
            for(i = 0; i < len; i++) {
                int index = i << 1;
                if(ret[index + 1] != 0)
                    return String.newString(ret, (byte)1);
            }
            byte[] buff = new byte[len];
            for(i = 0; i < len; i++)
                buff[i] = ret[i << 1];
            return String.newString(ret, (byte)0);
        }
        return null;
    }

    public static String[] split(byte[] value) {
        int len = value.length >> 1;
        String[] ret = new String[len];
        for(int i = 0; i < len; i++) {
            char c = charAt(value, i);
            if(c < 256)
                ret[i] = String.newString(new byte[] {(byte)c}, 0, 1, (byte)0);
            else {
                byte b1 = (byte)c;
                byte b2 = (byte)(c >>> 8);
                ret[i] = String.newString(new byte[] {b1, b2}, 0, 2, (byte)1);
            }
        }
        return ret;
    }

    public static String[] split(byte[] value, char ch) {
        if(ch > 65535)
            return null;
        int len = value.length >>> 1;
        int arrayCount = 1;
        for(int i = 0; i < len; i++) {
            if(ch == charAt(value, i))
                arrayCount++;
        }
        if(arrayCount == 1)
            return null;
        String[] ret = new String[arrayCount];
        int index = 0;
        int start = 0;
        boolean isLatin1 = true;
        for(int i = 0; i < len; i++) {
            if(ch == charAt(value, i)) {
                int count = i - start;
                if(!isLatin1)
                    ret[index] = String.newString(value, (start << 1), count << 1, (byte)1);
                else {
                    byte[] buff = new byte[count];
                    for(int j = 0; j < count; j++)
                        buff[j] = value[(j + start) << 1];
                    ret[index] = String.newString(buff, (byte)0);
                }
                start = i + 1;
                index++;
                isLatin1 = true;
            }
            if(isLatin1 && ch > 255)
                isLatin1 = false;
        }
        if(start < len) {
            int count = len - start;
            if(!isLatin1)
                ret[index] = String.newString(value, (start << 1), count << 1, (byte)1);
            else {
                byte[] buff = new byte[count];
                for(int j = 0; j < count; j++)
                    buff[j] = value[(j + start) << 1];
                ret[index] = String.newString(buff, (byte)0);
            }
        }
        return ret;
    }

    public static String toLower(byte[] value) {
        int i;
        int length = value.length >>> 1;
        byte[] ret = null;
        for(i = 0; i < length; i++) {
            char c = charAt(value, i);
            char lower = Character.toLower(c);
            if(lower != c) {
                ret = new byte[value.length];
                int index = i << 1;
                System.arraycopy(value, 0, ret, 0, index);
                ret[index] = (byte)lower;
                ret[index + 1] = (byte)(lower >>> 8);
                i++;
                break;
            }
        }
        for(; i < length; i++) {
            int index = i << 1;
            char lower = Character.toLower(charAt(value, i));
            ret[index] = (byte)lower;
            ret[index + 1] = (byte)(lower >>> 8);
        }
        return (ret == null) ? null : String.newString(ret,  (byte)1);
    }

    public static String toUpper(byte[] value) {
        int i;
        int length = value.length >>> 1;
        byte[] ret = null;
        for(i = 0; i < length; i++) {
            char c = charAt(value, i);
            char upper = Character.toUpper(c);
            if(upper != c) {
                ret = new byte[value.length];
                int index = i << 1;
                System.arraycopy(value, 0, ret, 0, index);
                ret[index] = (byte)upper;
                ret[index + 1] = (byte)(upper >>> 8);
                i++;
                break;
            }
        }
        for(; i < length; i++) {
            int index = i << 1;
            char upper = Character.toUpper(charAt(value, i));
            ret[index] = (byte)upper;
            ret[index + 1] = (byte)(upper >>> 8);
        }
        return (ret == null) ? null : String.newString(ret,  (byte)1);
    }

    public static String trim(byte[] value) {
        int len = value.length >>> 1;
        int st = 0;
        while((st < len) && (charAt(value, st) <= ' '))
            st++;
        while((st < len) && (charAt(value, len - 1) <= ' '))
            len--;
        return ((st > 0) || (len < value.length)) ? String.newString(value, (st << 1), (len - st) << 1, (byte)1) : null;
    }

    public static char[] toChars(byte[] value) {
        int len = value.length >> 1;
        char[] ret = new char[len];
        for(int i = 0; i < len; i++) {
            char c = charAt(value, i);
            ret[i] = c;
        }
        return ret;
    }

    public static int compareTo(byte[] value, byte[] other) {
        int len1 = value.length >>> 1;
        int len2 = other.length >>> 1;
        int lim = Math.min(len1, len2);
        for(int i = 0; i < lim; i++) {
            char c1 = charAt(value, i);
            char c2 = charAt(value, i);
            if(c1 != c2)
                return c1 - c2;
        }
        return len1 - len2;
    }

    public static int  compareToLatin1(byte[] value, byte[] other) {
        return -StringLatin1.compareToUTF16(other, value);
    }

    public static boolean equalsIgnoreCase(byte[] value, byte[] other) {
        int len = value.length >>> 1;
        for(int i = 0; i < len; i++)
            if(Character.toLower(charAt(value, i)) != Character.toLower(charAt(other, i)))
                return false;
        return true;
    }
}
