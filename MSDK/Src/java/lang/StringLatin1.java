package java.lang;

final class StringLatin1 {
    public static char charAt(byte[] value, int index) {
        return (char)(value[index] & 0xFF);
    }

    public static void putCharAt(byte[] value, int index, char c) {
        if(c > 255)
            throw new Error("Character is not latin1");
        value[index] = (byte)c;
    }

    public static int indexOf(byte[] value, int ch, int fromIndex) {
        if(ch > 255)
            return -1;
        byte c = (byte)ch;
        for(int i = fromIndex; i < value.length; i++) {
            if(c == value[i])
                return i;
        }
        return -1;
    }

    public static int indexOf(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return 0;
        if(value.length == 0)
            return -1;
        byte first = str[0];
        int max = value.length - str.length;
        for(int i = fromIndex; i <= max; i++) {
            if(value[i] != first)
                while(++i <= max && value[i] != first);
            if(i <= max) {
                int j = i + 1;
                int end = j + str.length - 1;
                for(int k = 1; j < end && value[j] == str[k]; j++, k++);
                if(j == end)
                    return i;
            }
        }
        return -1;
    }

    public static int lastIndexOf(byte[] value, int ch, int fromIndex) {
        if(ch > 255)
            return -1;
        byte c = (byte)ch;
        for(int i = fromIndex; i >= 0; i--) {
            if(c == value[i])
                return i;
        }
        return -1;
    }

    public static int lastIndexOf(byte[] value, byte[] str, int fromIndex) {
        if(str.length == 0)
            return value.length - 1;
        if(value.length < str.length)
            return -1;

        for(int i = fromIndex - (str.length - 1); i >= 0; i--) {
            if(value[i] == str[0]) {
                boolean found = true;
                for(int j = 1; j < str.length; j++) {
                    if(value[i + j] != str[j]) {
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
        if(oldChar > 255)
            return null;
        int i = 0;
        int len = value.length;
        byte oldCh = (byte)oldChar;
        if(newChar < 256) {
            byte[] ret = null;
            for(; i < len; i++) {
                if(value[i] == oldCh) {
                    ret = new byte[len];
                    System.arraycopy(value, 0, ret, 0, i);
                    ret[i] = (byte)newChar;
                    break;
                }
            }
            for(; i < len; i++) {
                byte c = value[i];
                ret[i] = (c == oldCh) ? (byte)newChar : c;
            }
            return (ret == null) ? null : String.newString(ret, (byte)0);
        }
        else {
            byte[] ret = null;
            for(; i < len; i++) {
                if(value[i] == oldCh) {
                    ret = new byte[len << 1];
                    for(int j = 0; j < i; j++)
                        ret[j << 1] = value[j];
                    int index = i << 1;
                    ret[index] = (byte)newChar;
                    ret[index + 1] = (byte)(newChar >>> 8);
                    break;
                }
            }
            for(; i < len; i++) {
                byte c = value[i];
                int index = i << 1;
                if(c == oldCh) {
                    ret[index] = (byte)newChar;
                    ret[index + 1] = (byte)(newChar >>> 8);
                }
                else
                    ret[index] = c;
            }
            return (ret == null) ? null : String.newString(ret, (byte)1);
        }
    }

    public static String[] split(byte[] value) {
        int len = value.length;
        String[] ret = new String[len];
        for(int i = 0; i < len; i++)
            ret[i] = String.newString(value, i, 1, (byte)0);
        return ret;
    }

    public static String[] split(byte[] value, char ch) {
        if(ch > 255)
            return null;
        int len = value.length;
        int arrayCount = 1;
        byte c = (byte)ch;
        for(int i = 0; i < len; i++) {
            if(c == value[i])
                arrayCount++;
        }
        if(arrayCount == 1)
            return null;
        String[] ret = new String[arrayCount];
        int index = 0;
        int start = 0;
        for(int i = 0; i < len; i++) {
            if(c == value[i]) {
                ret[index] = String.newString(value, start, i - start, (byte)0);
                start = i + 1;
                index++;
            }
        }
        if(start < len)
            ret[index] = String.newString(value, start, len - start, (byte)0);
        return ret;
    }

    private static byte toLower(byte ch) {
        int c = ch & 0xFF;
        if((('A' <= c) && (c <= 'Z')) || (('À' <= c) && (c <= 'Ö')) || (c == 'Ø'))
            return (byte)(c + 32);
        return ch;
    }

    public static String toLower(byte[] value) {
        int i;
        int length = value.length;
        byte[] ret = null;
        for(i = 0; i < length; i++) {
            int c = value[i] & 0xFF;
            if((('A' <= c) && (c <= 'Z')) || (('À' <= c) && (c <= 'Ö')) || (c == 'Ø')) {
                ret = new byte[length];
                System.arraycopy(value, 0, ret, 0, i);
                ret[i] = (byte)(c + 32);
                i++;
                break;
            }
        }
        for(; i < length; i++) {
            int c = value[i] & 0xFF;
            if((('A' <= c) && (c <= 'Z')) || (('À' <= c) && (c <= 'Ö')) || (c == 'Ø'))
                ret[i] = (byte)(c + 32);
            else
                ret[i] = (byte)c;
        }
        return (ret == null) ? null : String.newString(ret,  (byte)0);
    }

    public static String toUpper(byte[] value) {
        int i;
        int length = value.length;
        byte[] ret = null;
        for(i = 0; i < length; i++) {
            int c = value[i] & 0xFF;
            if((('a' <= c) && (c <= 'z')) || (('à' <= c) && (c <= 'ö')) || (c == 'ø')) {
                ret = new byte[length];
                System.arraycopy(value, 0, ret, 0, i);
                ret[i] = (byte)(c - 32);
                i++;
                break;
            }
        }
        for(; i < length; i++) {
            int c = value[i] & 0xFF;
            if((('a' <= c) && (c <= 'z')) || (('à' <= c) && (c <= 'ö')) || (c == 'ø'))
                ret[i] = (byte)(c - 32);
            else
                ret[i] = (byte)c;
        }
        return (ret == null) ? null : String.newString(ret,  (byte)0);
    }

    public static String trim(byte[] value) {
        int len = value.length;
        int st = 0;
        while((st < len) && ((value[st] & 0xFF) <= ' '))
            st++;
        while((st < len) && ((value[len - 1] & 0xFF) <= ' '))
            len--;
        return ((st > 0) || (len < value.length)) ? String.newString(value, st, len - st, (byte)0) : null;
    }

    public static char[] toChars(byte[] value) {
        int len = value.length;
        char[] ret = new char[len];
        for(int i = 0; i < len; i++)
            ret[i] = (char)(value[i] & 0xFF);
        return ret;
    }

    public static int compareTo(byte[] value, byte[] other) {
        int lim = Math.min(value.length, other.length);
        for(int i = 0; i < lim; i++) {
            if(value[i] != other[i])
                return value[i] - other[i];
        }
        return value.length - other.length;
    }

    public static int compareToUTF16(byte[] value, byte[] other) {
        int lim = Math.min(value.length, other.length >> 1);
        for(int i = 0; i < lim; i++) {
            char c1 = (char)(value[i] & 0xFF);
            char c2 = StringUTF16.charAt(other, i);
            if(c1 != c2)
                return c1 - c2;
        }
        return value.length - (other.length >> 1);
    }

    public static boolean equals(byte[] value, byte[] other) {
        if(value.length != other.length)
            return false;
        for(int i = 0; i < value.length; i++) {
            if(value[i] != other[i])
                return false;
        }
        return true;
    }

    public static boolean equalsIgnoreCase(byte[] value, byte[] other) {
        int len = value.length;
        for(int i = 0; i < len; i++)
            if(toLower(value[i]) != toLower(other[i]))
                return false;
        return true;
    }
}
