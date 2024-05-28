package java.lang;

import java.io.IOException;
import jdk.internal.math.DoubleToDecimal;
import jdk.internal.math.FloatToDecimal;

abstract sealed class AbstractStringBuilder implements Appendable, CharSequence permits StringBuilder, StringBuffer {
    byte[] value;
    boolean maybeLatin1;
    byte coder;
    int count;

    AbstractStringBuilder() {
        value = new byte[16];
        coder = 0;
        count = 0;
        maybeLatin1 = false;
    }

    AbstractStringBuilder(int capacity) {
        value = new byte[capacity];
        coder = 0;
        count = 0;
        maybeLatin1 = false;
    }

    AbstractStringBuilder(String str) {
        int len = str.length();
        coder = str.coder();
        value = new byte[(len + 16) << coder];
        count = 0;
        maybeLatin1 = false;
        append(str);
    }

    AbstractStringBuilder(CharSequence seq) {
        final byte initCoder;
        int length = seq.length();
        if(seq instanceof AbstractStringBuilder asb) {
            initCoder = asb.coder;
            maybeLatin1 = asb.maybeLatin1;
        }
        else if(seq instanceof String s)
            initCoder = s.coder();
        else
            initCoder = 0;
        value = new byte[(length + 16) << initCoder];
        coder = initCoder;
        append(seq);
    }

    public void setCharAt(int index, char ch) {
        if(index >= count)
            throw new StringIndexOutOfBoundsException("Index " + index + " out of bounds for length " + count);
        if((coder == 0) && (ch < 256))
            value[index] = (byte)ch;
        else {
            inflate();
            index <<= 1;
            value[index] = (byte)ch;
            value[index + 1] = (byte)(ch >>> 8);
            if(ch < 256)
                maybeLatin1 = true;
        }
    }

    public AbstractStringBuilder clear() {
        maybeLatin1 = false;
        coder = 0;
        count = 0;
        return this;
    }

    public AbstractStringBuilder append(Object obj) {
        return append(String.valueOf(obj));
    }

    public AbstractStringBuilder append(String str) {
        return append(str, 0, str.length());
    }

    public AbstractStringBuilder append(StringBuffer sb) {
        return append(sb, 0, sb.length());
    }

    @Override
    public AbstractStringBuilder append(CharSequence s) {
        if(s == null)
            return appendNull();
        if(s instanceof String)
            return append((String)s);
        if(s instanceof AbstractStringBuilder)
            return append((AbstractStringBuilder)s);
        return append(s, 0, s.length());
    }

    @Override
    public AbstractStringBuilder append(CharSequence s, int start, int end) {
        if(s == null)
            s = "null";
        byte[] val;
        int len = end - start;
        int count = this.count;
        int i = 0;
        if(coder == 0) {
            if(s instanceof String str) {
                if(str.coder() == 1) {
                    if(count != 0)
                        inflate();
                    else
                        coder = 1;
                }
            }
            else if(s instanceof AbstractStringBuilder asb) {
                if(asb.coder == 1) {
                    if(count != 0)
                        inflate();
                    else
                        coder = 1;
                }
            }
            ensureCapacityInternal(count + len);
            if(coder == 0) {
                val = this.value;
                for(; i < len; i++) {
                    char c = s.charAt(i + start);
                    if(c < 256)
                        val[count++] = (byte)c;
                    else if(count != 0) {
                        inflate();
                        break;
                    }
                    else {
                        coder = 1;
                        ensureCapacityInternal(count + len);
                        break;
                    }
                }
            }
        }
        else
            ensureCapacityInternal(count + len);
        if(i < len) {
            val = this.value;
            for(; i < len; i++) {
                char c = s.charAt(i + start);
                int index = count << 1;
                val[index] = (byte)c;
                val[index + 1] = (byte)(c >>> 8);
                count++;
            }
        }
        this.count = count;
        return this;
    }

    public AbstractStringBuilder append(char[] str) {
        return append(str, 0, str.length);
    }

    public AbstractStringBuilder append(char[] str, int offset, int len) {
        int count = this.count;
        byte[] val;
        int i = 0;
        if(coder == 0) {
            ensureCapacityInternal(count + len);
            val = this.value;
            for(; i < len; i++) {
                char c = str[i + offset];
                if(c < 256)
                    val[count++] = (byte)c;
                else if(count != 0) {
                    inflate();
                    break;
                }
                else {
                    coder = 1;
                    ensureCapacityInternal(count + len);
                    break;
                }
            }
        }
        else
            ensureCapacityInternal(count + len);
        if(i < len) {
            val = this.value;
            for(; i < len; i++) {
                char c = str[i + offset];
                int index = count << 1;
                val[index] = (byte)c;
                val[index + 1] = (byte)(c >>> 8);
                count++;
            }
        }
        this.count = count;
        return this;
    }

    public AbstractStringBuilder append(boolean b) {
        return append(String.valueOf(b));
    }

    @Override
    public AbstractStringBuilder append(char c) {
        if((coder == 0) && (c < 256)) {
            ensureCapacityInternal(count + 1);
            value[count++] = (byte)c;
        }
        else {
            if(count != 0)
                inflate();
            else
                coder = 1;
            ensureCapacityInternal(count + 1);
            int index = count << 1;
            value[index] = (byte)c;
            value[index + 1] = (byte)(c >>> 8);
            count++;
        }
        return this;
    }

    public AbstractStringBuilder append(int i) {
        return append(String.valueOf(i));
    }

    public AbstractStringBuilder append(long l) {
        return append(String.valueOf(l));
    }

    public AbstractStringBuilder append(float f) {
        try {
            FloatToDecimal.appendTo(f, this);
        }
        catch(IOException e) {
            throw new AssertionError(e);
        }
        return this;
    }

    public AbstractStringBuilder append(double d) {
        try {
            DoubleToDecimal.appendTo(d, this);
        }
        catch(IOException e) {
            throw new AssertionError(e);
        }
        return this;
    }

    private AbstractStringBuilder appendNull() {
        int count = this.count;
        ensureCapacityInternal(count + 4);
        byte[] val = this.value;
        if(coder == 0) {
            val[count++] = 'n';
            val[count++] = 'u';
            val[count++] = 'l';
            val[count++] = 'l';
        }
        else {
            val[count << 1] = 'n';
            count++;
            val[count << 1] = 'u';
            count++;
            val[count << 1] = 'l';
            count++;
            val[count << 1] = 'l';
            count++;
        }
        this.count = count;
        return this;
    }

    int compareTo(AbstractStringBuilder another) {
        if(this == another)
            return 0;

        int lim = Math.min(count, another.count);
        for(int i = 0; i < lim; i++) {
            char c1 = charAt(i);
            char c2 = another.charAt(i);
            if(c1 != c2)
                return c1 - c2;
        }
        return count - another.count;
    }

    private void inflate() {
        if(this.coder == 1)
            return;
        int count = this.count;
        byte[] value = this.value;
        int length = value.length;
        byte[] buf = new byte[length << 1];
        for(int i = 0; i < count; i++)
            buf[i << 1] = value[i];
        this.value = buf;
        this.coder = 1;
    }

    public void trimToSize() {
        int length = count << coder;
        if(length < value.length) {
            byte[] buff = new byte[length];
            System.arraycopy(value, 0, buff, 0, length);
            value = buff;
        }
    }

    @Override
    public int length() {
        return count;
    }

    @Override
    public char charAt(int index) {
        if(coder == 0)
            return (char)value[index];
        return StringUTF16.charAt(value, index);
    }

    public String substring(int start) {
        return substring(start, count);
    }

    @Override
    public CharSequence subSequence(int start, int end) {
        return substring(start, end);
    }

    public String substring(int start, int end) {
        byte[] val = value;
        if(coder == 0) {
            if((start == 0) && (end == val.length))
                val = val.clone();
            return String.newString(val, start, end - start, (byte)0);
        }
        boolean isLatin1 = true;
        for(int i = start; i < end; i++) {
            int index = i << 1;
            if(val[index + 1] != 0) {
                isLatin1 = false;
                break;
            }
        }
        if(isLatin1) {
            byte[] buff = new byte[end - count];
            for(int i = start; i < end; i++)
                buff[i] = val[i << 1];
            return String.newString(buff, (byte)0);
        }
        if((start == 0) && (end == val.length))
            val = val.clone();
        return String.newString(val, start << 1, (end - start) << 1, (byte)1);
    }

    public int indexOf(String str) {
        if(coder == 0)
            return StringLatin1.indexOf(value, str.value(), 0);
        return StringUTF16.indexOf(value, str.value(), 0);
    }

    public int lastIndexOf(String str) {
        if(coder == 0)
            return StringLatin1.lastIndexOf(value, str.value(), count - 1);
        return StringUTF16.lastIndexOf(value, str.value(), count - 1);
    }

    @Override
    public abstract String toString();

    public int capacity() {
        return value.length >> coder;
    }

    private void ensureCapacityInternal(int minimumCapacity) {
        byte coder = this.coder;
        int oldCapacity = value.length >> coder;
        if(minimumCapacity > oldCapacity) {
            minimumCapacity += 16;
            byte[] buff = new byte[minimumCapacity << coder];
            System.arraycopy(value, 0, buff, 0, count << coder);
            value = buff;
        }
    }
}
