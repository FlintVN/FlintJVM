package java.lang;

final class StringUTF16 {
	public static char charAt(byte[] value, int index) {
		index <<= 1;
		return (char)(((char)value[index + 1] << 8) | value[index]);
	}
	
	public static String newString(byte[] val, int index, int len) {
        if (len == 0)
            return "";
        index <<= 1;
        len <<= 1;
        byte[] buff = new byte[len];
        System.arraycopy(val, index, buff, 0, len);
        return String.newString(buff, 0, len, (byte)1);
    }
	
	public static int compareTo(byte[] value, byte[] other) {
		int len1 = value.length >>> 1;
        int len2 = other.length >>> 1;
		int lim = Math.min(len1, len2);
        for (int k = 0; k < lim; k++) {
        	char c1 = charAt(value, k);
        	char c2 = charAt(value, k);
            if (c1 != c2)
                return c1 - c2;
        }
        return len1 - len2;
    }
	
	public static int  compareToLatin1(byte[] value, byte[] other) {
		return -StringLatin1.compareToUTF16(other, value);
	}
}
