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

    public StringBuilder clear() {
        super.clear();
        return this;
    }

    public StringBuilder append(Object obj) {
        super.append(String.valueOf(obj));
        return this;
    }

    public StringBuilder append(String str) {
        super.append(str, 0, str.length());
        return this;
    }

    public StringBuilder append(StringBuffer sb) {
        super.append(sb, 0, sb.length());
        return this;
    }

    public StringBuilder append(CharSequence s) {
        super.append(s);
        return this;
    }

    public StringBuilder append(CharSequence s, int start, int end) {
        super.append(s, start, end);
        return this;
    }

    public StringBuilder append(char[] str) {
        super.append(str, 0, str.length);
        return this;
    }

    public StringBuilder append(char[] str, int offset, int len) {
        super.append(str, offset, len);
        return this;
    }

    public StringBuilder append(boolean b) {
        super.append(String.valueOf(b));
        return this;
    }

    public StringBuilder append(char c) {
        super.append(c);
        return this;
    }

    public StringBuilder append(int i) {
        super.append(String.valueOf(i));
        return this;
    }

    public StringBuilder append(long l) {
        super.append(String.valueOf(l));
        return this;
    }

    public StringBuilder append(float f) {
        super.append(f);
        return this;
    }

    public StringBuilder append(double d) {
        super.append(d);
        return this;
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
