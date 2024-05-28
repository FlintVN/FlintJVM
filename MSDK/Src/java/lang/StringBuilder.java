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

    @Override
    public StringBuilder clear() {
        super.clear();
        return this;
    }

    @Override
    public StringBuilder append(Object obj) {
        super.append(String.valueOf(obj));
        return this;
    }

    @Override
    public StringBuilder append(String str) {
        super.append(str, 0, str.length());
        return this;
    }

    @Override
    public StringBuilder append(StringBuffer sb) {
        super.append(sb, 0, sb.length());
        return this;
    }

    @Override
    public StringBuilder append(CharSequence s) {
        super.append(s);
        return this;
    }

    @Override
    public StringBuilder append(CharSequence s, int start, int end) {
        super.append(s, start, end);
        return this;
    }

    @Override
    public StringBuilder append(char[] str) {
        super.append(str, 0, str.length);
        return this;
    }

    @Override
    public StringBuilder append(char[] str, int offset, int len) {
        super.append(str, offset, len);
        return this;
    }

    @Override
    public StringBuilder append(boolean b) {
        super.append(String.valueOf(b));
        return this;
    }

    @Override
    public StringBuilder append(char c) {
        super.append(c);
        return this;
    }

    @Override
    public StringBuilder append(int i) {
        super.append(String.valueOf(i));
        return this;
    }

    @Override
    public StringBuilder append(long l) {
        super.append(String.valueOf(l));
        return this;
    }

    @Override
    public StringBuilder append(float f) {
        super.append(f);
        return this;
    }

    @Override
    public StringBuilder append(double d) {
        super.append(d);
        return this;
    }

    @Override
    public int compareTo(StringBuilder another) {
        return super.compareTo(another);
    }

    @Override
    public int length() {
        return count;
    }

    @Override
    public int capacity() {
        return value.length >> coder;
    }

    @Override
    public String toString() {
        return substring(0, count);
    }
}
