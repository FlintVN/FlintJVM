package java.lang;

public final class StringBuilder extends AbstractStringBuilder implements Comparable<StringBuilder>, CharSequence {
	public StringBuilder() {
        super(16);
    }

    public StringBuilder(int capacity) {
        super(capacity);
    }

    public StringBuilder(String str) {
        super(str);
    }

    public StringBuilder(CharSequence seq) {
        super(seq);
    }
    
    public int compareTo(StringBuilder another) {
        return super.compareTo(another);
    }
    
    public int length() {
        return count;
    }
    
    public int capacity() {
        return value.length >> coder;
    }
    
    public String toString() {
    	return substring(0, count);
    }
}
