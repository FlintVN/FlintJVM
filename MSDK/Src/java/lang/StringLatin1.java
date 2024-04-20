package java.lang;

final class StringLatin1 {
	public static char charAt(byte[] value, int index) {
		return (char)value[index];
	}
	
	public static String newString(byte[] val, int index, int len) {
        if (len == 0)
            return "";
        byte[] buff = new byte[len];
        System.arraycopy(val, index, buff, 0, len);
        return String.newString(buff, 0, len, (byte)0);
    }
	
	public static int compareTo(byte[] value, byte[] other) {
		int lim = Math.min(value.length, other.length);
        for (int k = 0; k < lim; k++) {
            if (value[k] != other[k])
                return value[k] - other[k];
        }
        return value.length - other.length;
    }
    
    public static int compareToUTF16(byte[] value, byte[] other) {
        int lim = Math.min(value.length, other.length >> 1);
        for (int k = 0; k < lim; k++) {
            char c1 = (char)value[k];
            char c2 = StringUTF16.charAt(other, k);
            if (c1 != c2)
                return c1 - c2;
        }
        return value.length - (other.length >> 1);
    }
}
