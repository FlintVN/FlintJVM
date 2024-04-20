package java.lang;

public final class String implements Comparable<String>, CharSequence {
	private final byte[] value;
	private final byte coder;

	private static boolean isLatin1(byte[] value, int offset, int count) {
		for(; offset < count; offset++)
			if(value[offset] < 0)
				return false;
		return true;
	}
	
	private static boolean isLatin1(char[] value, int offset, int count) {
		for(; offset < count; offset++)
			if(value[offset] > 127)
				return false;
		return true;
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
                char c = value[i];
                buff[i] = (byte)c;
                buff[i] = (byte)(c >>> 8);
            }
            this.value = buff;
            this.coder = 1;
    	}
    }
    
    String(byte[] value, int offset, int count, byte coder) {
    	if(coder == 0)
            this.value = value.clone();
    	else {
            byte[] buff = new byte[count << 1];
	    	for(int i = 0; i < count; i++)
                buff[i << 1] = value[i + offset];
            this.value = buff;
    	}
    	this.coder = coder;
    }

    public String(byte[] value, int offset, int count) {
    	if(isLatin1(value, offset, count)) {
    		this.value = new byte[count];
    		System.arraycopy(value, offset, this.value, 0, count);
            this.coder = 0;
    	}
    	else {
            byte[] buff = new byte[count << 1];
	    	for(int i = 0; i < count; i++)
                buff[i << 1] = value[i + offset];
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
    
    public String substring(int beginIndex) {
        return substring(beginIndex, length());
    }
    
    public String substring(int beginIndex, int endIndex) {
        int length = length();
        if (beginIndex == 0 && endIndex == length)
            return this;
        int subLen = endIndex - beginIndex;
        return coder == 0 ? StringLatin1.newString(value, beginIndex, subLen) : StringUTF16.newString(value, beginIndex, subLen);
    }

    public CharSequence subSequence(int beginIndex, int endIndex) {
    	return this.substring(beginIndex, endIndex);
    }

    public String toString() {
    	return this;
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
