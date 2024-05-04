package java.lang;

public final class StringBuffer extends AbstractStringBuilder implements Comparable<StringBuilder>, CharSequence {
	public StringBuffer() {
        super(16);
    }

    public StringBuffer(int capacity) {
        super(capacity);
    }

    public StringBuffer(String str) {
        super(str);
    }

    public StringBuffer(CharSequence seq) {
        super(seq);
    }
    
    public synchronized void setCharAt(int index, char ch) {
    	super.setCharAt(index, ch);
    }
    
    public synchronized AbstractStringBuilder clear() {
    	return super.clear();
    }
    
    public synchronized AbstractStringBuilder append(Object obj) {
    	return super.append(String.valueOf(obj));
    }
    
    public synchronized AbstractStringBuilder append(String str) {
		return super.append(str, 0, str.length());
    }
    
    public synchronized AbstractStringBuilder append(StringBuffer sb) {
		return super.append(sb, 0, sb.length());
    }
    
    public synchronized AbstractStringBuilder append(CharSequence s) {
    	return super.append(s);
    }
    
    public synchronized AbstractStringBuilder append(CharSequence s, int start, int end) {
    	return super.append(s, start, end);
    }
    
    public synchronized AbstractStringBuilder append(char[] str) {
    	return super.append(str, 0, str.length);
    }
    
    public synchronized AbstractStringBuilder append(char[] str, int offset, int len) {
    	return super.append(str, offset, len);
    }
    
    public synchronized AbstractStringBuilder append(boolean b) {
		return super.append(String.valueOf(b));
	}
    
    public synchronized AbstractStringBuilder append(char c) {
    	return super.append(c);
    }
    
    public synchronized AbstractStringBuilder append(int i) {
		return super.append(String.valueOf(i));
	}
    
    public synchronized AbstractStringBuilder append(long l) {
		return super.append(String.valueOf(l));
	}
    
    public synchronized AbstractStringBuilder append(float f) {
    	return super.append(f);
    }
    
    public synchronized AbstractStringBuilder append(double d) {
    	return super.append(d);
    }
    
    public synchronized void trimToSize() {
        super.trimToSize();
    }
    
    public synchronized int compareTo(StringBuilder another) {
        return super.compareTo(another);
    }
    
    public synchronized char charAt(int index) {
    	return super.charAt(index);
    }
    
    public synchronized String substring(int start) {
    	return super.substring(start, count);
    }
    
    public synchronized CharSequence subSequence(int start, int end) {
		return super.substring(start, end);
	}
    
    public synchronized String substring(int start, int end) {
    	return super.substring(start, end);
    }
    
    public synchronized int indexOf(String str) {
    	return super.indexOf(str);
    }
    
    public synchronized int lastIndexOf(String str) {
    	return super.lastIndexOf(str);
    }
    
    public synchronized int length() {
        return count;
    }
    
    public synchronized int capacity() {
        return value.length >> coder;
    }
    
    public synchronized String toString() {
    	return super.substring(0, count);
    }
}
