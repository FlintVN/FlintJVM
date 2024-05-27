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

    public synchronized StringBuffer clear() {
        super.clear();
        return this;
    }

    public synchronized StringBuffer append(Object obj) {
        super.append(String.valueOf(obj));
        return this;
    }

    public synchronized StringBuffer append(String str) {
        super.append(str, 0, str.length());
        return this;
    }

    public synchronized StringBuffer append(StringBuffer sb) {
        super.append(sb, 0, sb.length());
        return this;
    }

    public synchronized StringBuffer append(CharSequence s) {
        super.append(s);
        return this;
    }

    public synchronized StringBuffer append(CharSequence s, int start, int end) {
        super.append(s, start, end);
        return this;
    }

    public synchronized StringBuffer append(char[] str) {
        super.append(str, 0, str.length);
        return this;
    }

    public synchronized StringBuffer append(char[] str, int offset, int len) {
        super.append(str, offset, len);
        return this;
    }

    public synchronized StringBuffer append(boolean b) {
        super.append(String.valueOf(b));
        return this;
    }

    public synchronized StringBuffer append(char c) {
        super.append(c);
        return this;
    }

    public synchronized StringBuffer append(int i) {
        super.append(String.valueOf(i));
        return this;
    }

    public synchronized StringBuffer append(long l) {
        super.append(String.valueOf(l));
        return this;
    }

    public synchronized StringBuffer append(float f) {
        super.append(f);
        return this;
    }

    public synchronized StringBuffer append(double d) {
        super.append(d);
        return this;
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
