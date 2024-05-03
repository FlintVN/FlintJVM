package java.lang;

public final class String implements Comparable<String>, CharSequence {
	private final byte[] value;
	private final byte coder;

	private static boolean isLatin1(byte[] value, int offset, int count) {
		count += offset;
		for(; offset < count; offset++)
			if(value[offset] < 0)
				return false;
		return true;
	}
	
	private static boolean isLatin1(char[] value, int offset, int count) {
		count += offset;
		for(; offset < count; offset++)
			if(value[offset] > 127)
				return false;
		return true;
	}
	
	private static int getUtf8ByteCount(byte b) {
		if(b > 0)
			return 1;
		if((b & 0xE0) == 0xC0)
			return 2;
		else if((b & 0xF0) == 0xE0)
			return 3;
		else if((b & 0xF8) == 0xF0)
			return 4;
		else if((b & 0xFC) == 0xF8)
			return 5;
		else
			return 6;
	}
	
	private static int getUtf8StrLength(byte[] utf8, int offset, int count) {
		int len = 0;
		count += offset;
		while(offset < count) {
			offset += getUtf8ByteCount(utf8[offset]);
			len++;
		}
		return len;
	}
	
	static String newString(byte[] value, byte coder) {
		return new String(value, 0, value.length, coder);
	}
	
	static String newString(byte[] value, int offset, int count, byte coder) {
		return new String(value, offset, count, coder);
	}

    public String() {
        value = "".value;
        coder = "".coder;
    }

    public String(String original) {
        value = original.value;
        coder = original.coder;
    }
    
    public String(char[] value) {
    	this(value, 0, value.length);
    }
    
    public String(char[] value, int offset, int count) {
    	if(isLatin1(value, offset, count)) {
            byte[] buff = new byte[count];
            for(int i = 0; i < count; i++)
                buff[i] = (byte)value[i];
            this.value = buff;
            this.coder = 0;
    	}
    	else {
            byte[] buff = new byte[count << 1];
	    	for(int i = 0; i < count; i++) {
                char c = value[i + offset];
                buff[i] = (byte)c;
                buff[i] = (byte)(c >>> 8);
            }
            this.value = buff;
            this.coder = 1;
    	}
    }
    
    String(byte[] value, int offset, int count, byte coder) {
    	if(offset == 0 && count == value.length)
    		this.value = value;
    	else {
	    	this.value = new byte[count];
	        System.arraycopy(value, offset, this.value, 0, count);
    	}
    	this.coder = coder;
    }

    public String(byte[] utf8Value) {
    	this(utf8Value, 0, utf8Value.length);
    }

    public String(byte[] utf8Value, int offset, int count) {
    	if(isLatin1(utf8Value, offset, count)) {
    		this.value = new byte[count];
    		System.arraycopy(utf8Value, offset, this.value, 0, count);
            this.coder = 0;
    	}
    	else {
    		int len = getUtf8StrLength(utf8Value, offset, count);
            byte[] buff = new byte[len << 1];
            int utf8Index = 0;
            for(int i = 0; i < len; i++) {
            	int index = i << 1;
            	byte b = utf8Value[utf8Index];
            	if(b < 0) {
		            int byteCount = getUtf8ByteCount(b);
		            int code = b & (0xFF >> (byteCount + 1));
		            while(--byteCount > 0) {
		            	utf8Index++;
		                code <<= 6;
		                code |= utf8Value[utf8Index] & 0x3F;
		            }
		            if(code < 65536) {
			            buff[index] = (byte)code;
			            buff[index + 1] = (byte)(code >>> 8);
		            }
		            else
		            	throw new Error("Characters are not supported");
            	}
            	else
            		buff[index] = b;
            	utf8Index++;
            }
            this.value = buff;
            this.coder = 1;
    	}
    }
    
    public String(StringBuffer buffer) {
        this(buffer.toString());
    }
    
    public String(StringBuilder builder) {
        this(builder.toString());
    }
    
    byte coder() {
        return coder;
    }

    byte[] value() {
        return value;
    }
    
    public int length() {
    	return value.length >>> coder;
    }

    public char charAt(int index) {
    	if (coder == 0)
            return StringLatin1.charAt(value, index);
        else
            return StringUTF16.charAt(value, index);
    }

    public boolean isEmpty() {
    	return value.length == 0;
    }
    
    public boolean startsWith(String prefix, int toffset) {
        if (toffset < 0 || toffset > length() - prefix.length())
            return false;
        byte ta[] = value;
        byte pa[] = prefix.value;
        int po = 0;
        int pc = pa.length;
        byte coder = this.coder;
        if (coder == prefix.coder) {
            int to = (coder == 0) ? toffset : toffset << 1;
            while (po < pc) {
                if (ta[to++] != pa[po++])
                    return false;
            }
        }
        else {
            if (coder == 0)
                return false;
            while (po < pc) {
                if (StringUTF16.charAt(ta, toffset++) != (pa[po++] & 0xff))
                    return false;
            }
        }
        return true;
    }
    
    public boolean startsWith(String prefix) {
        return startsWith(prefix, 0);
    }
    
    public boolean endsWith(String suffix) {
        return startsWith(suffix, length() - suffix.length());
    }
    
    public boolean contains(CharSequence s) {
        return indexOf(s.toString()) >= 0;
    }
    
    public int indexOf(int ch) {
    	return (coder == 0) ? StringLatin1.indexOf(value, ch) : StringUTF16.indexOf(value, ch);
    }
    
    public int indexOf(String str) {
        byte coder = this.coder;
        if (coder == str.coder)
            return (coder == 0) ? StringLatin1.indexOf(value, str.value) : StringUTF16.indexOf(value, str.value);
        if (coder == 0)
            return -1;
        return StringUTF16.indexOfLatin1(value, str.value);
    }
    
    public int lastIndexOf(int ch) {
    	return (coder == 0) ? StringLatin1.lastIndexOf(value, ch) : StringUTF16.lastIndexOf(value, ch);
    }
    
    public int lastIndexOf(String str) {
    	byte coder = this.coder;
    	if (coder == str.coder)
            return (coder == 0) ? StringLatin1.lastIndexOf(value, str.value) : StringUTF16.lastIndexOf(value, str.value);
        if (coder == 0)
            return -1;
        return StringUTF16.lastIndexOfLatin1(value, str.value);
    }
    
    public String replace(char oldChar, char newChar) {
        if (oldChar != newChar) {
            String ret = (coder == 0) ? StringLatin1.replace(value, oldChar, newChar) : StringUTF16.replace(value, oldChar, newChar);
            if (ret != null)
                return ret;
        }
        return this;
    }
    
    public String substring(int beginIndex) {
        return substring(beginIndex, length());
    }
    
    public String substring(int beginIndex, int endIndex) {
        int length = length();
        if (beginIndex == 0 && endIndex == length)
            return this;
        int subLen = endIndex - beginIndex;
        if(coder == 0)
        	return newString(value, beginIndex, subLen, (byte)0);
        boolean isLatin1 = true;
        byte[] val = value;
        for(int i = beginIndex; i < endIndex; i++) {
        	int index = i << 1;
        	if(val[index + 1] != 0) {
        		isLatin1 = false;
        		break;
        	}
        }
        if(isLatin1) {
        	byte[] buff = new byte[subLen];
        	for(int i = beginIndex; i < endIndex; i++)
        		buff[i] = val[i << 1];
        	return String.newString(buff, (byte)0);
        }
        return newString(value, beginIndex, subLen, (byte)1);
    }

    public CharSequence subSequence(int beginIndex, int endIndex) {
    	return this.substring(beginIndex, endIndex);
    }
    
    public String[] split() {
    	return coder == 0 ? StringLatin1.split(value) : StringUTF16.split(value);
    }
    
    public String[] split(char ch) {
    	String[] ret = coder == 0 ? StringLatin1.split(value, ch) : StringUTF16.split(value, ch);
    	if(ret == null)
    		ret = new String[] {this};
    	return ret;
    }
    
    public String toLower() {
    	String ret = (coder == 0) ? StringLatin1.toLower(value) : StringUTF16.toLower(value);
    	if(ret == null)
    		return this;
    	return ret;
    }
    
    public String toUpper() {
    	String ret = (coder == 0) ? StringLatin1.toUpper(value) : StringUTF16.toUpper(value);
    	if(ret == null)
    		return this;
    	return ret;
    }
    
    public String trim() {
    	String ret = (coder == 0) ? StringLatin1.trim(value) : StringUTF16.trim(value);
    	if(ret == null)
    		return this;
    	return ret;
    }
    
    public String translateEscapes() {
        if (isEmpty())
            return "";
        char[] chars = toCharArray();
        int length = chars.length;
        int from = 0;
        int to = 0;
        while (from < length) {
            char ch = chars[from++];
            if (ch == '\\') {
                ch = from < length ? chars[from++] : '\0';
                switch (ch) {
	                case 'b':
	                    ch = '\b';
	                    break;
	                case 'f':
	                    ch = '\f';
	                    break;
	                case 'n':
	                    ch = '\n';
	                    break;
	                case 'r':
	                    ch = '\r';
	                    break;
	                case 's':
	                    ch = ' ';
	                    break;
	                case 't':
	                    ch = '\t';
	                    break;
	                case '\'':
	                case '\"':
	                case '\\':
	                    break;
	                case '0': case '1': case '2': case '3':
	                case '4': case '5': case '6': case '7':
	                    int limit = Integer.min(from + (ch <= '3' ? 2 : 1), length);
	                    int code = ch - '0';
	                    while (from < limit) {
	                        ch = chars[from];
	                        if (ch < '0' || '7' < ch)
	                            break;
	                        from++;
	                        code = (code << 3) | (ch - '0');
	                    }
	                    ch = (char)code;
	                    break;
	                case '\n':
	                    continue;
	                case '\r':
	                    if (from < length && chars[from] == '\n')
	                        from++;
	                    continue;
	                default: {
	                    String msg = "Invalid escape sequence: " + ch + " " + Integer.toHexString((int)ch);
	                    throw new IllegalArgumentException(msg);
	                }
                }
            }
            chars[to++] = ch;
        }
        return new String(chars, 0, to);
    }
    
    public char[] toCharArray() {
        return (coder == 0) ? StringLatin1.toChars(value) : StringUTF16.toChars(value);
    }
    
    public static String valueOf(Object obj) {
        return (obj == null) ? "null" : obj.toString();
    }

    public static String valueOf(char[] data) {
        return new String(data);
    }

    public static String valueOf(char[] data, int offset, int count) {
        return new String(data, offset, count);
    }

    public static String copyValueOf(char[] data, int offset, int count) {
        return new String(data, offset, count);
    }

    public static String copyValueOf(char[] data) {
        return new String(data);
    }


    public static String valueOf(boolean b) {
        return b ? "true" : "false";
    }

    public static String valueOf(char c) {
    	if(c < 128) {
    		byte[] buff = new byte[1];
    		buff[0] = (byte)c;
    		return newString(buff, (byte)0);
    	}
    	else {
    		byte[] buff = new byte[2];
    		buff[0] = (byte)c;
    		buff[1] = (byte)(c >>> 8);
    		return newString(buff, (byte)0);
    	}
    }

    public static String valueOf(int i) {
        return Integer.toString(i);
    }

    public static String valueOf(long l) {
        return Long.toString(l);
    }

    public static String valueOf(float f) {
        return Float.toString(f);
    }

    public static String valueOf(double d) {
        return Double.toString(d);
    }

    public String toString() {
    	return this;
    }
    
    public boolean equals(Object anObject) {
        if (this == anObject)
            return true;
        if((anObject instanceof String aString) && (this.coder == aString.coder))
        	return StringLatin1.equals(value, aString.value);
        return false;
    }
    
    public int compareTo(String anotherString) {
        byte[] v1 = value;
        byte[] v2 = anotherString.value;
        byte coder = this.coder;
        if (coder == anotherString.coder)
            return coder == 0 ? StringLatin1.compareTo(v1, v2) : StringUTF16.compareTo(v1, v2);
        return coder == 0 ? StringLatin1.compareToUTF16(v1, v2) : StringUTF16.compareToLatin1(v1, v2);
     }
}
