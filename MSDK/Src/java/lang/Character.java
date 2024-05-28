package java.lang;

public final class Character implements Comparable<Character> {
    public static final int MIN_RADIX = 2;
    public static final int MAX_RADIX = 36;
    public static final char MIN_VALUE = 0x0000;
    public static final char MAX_VALUE = 0xFFFF;
    @SuppressWarnings("unchecked")
    public static final Class<Character> TYPE = (Class<Character>) Class.getPrimitiveClass("char");

    private final char value;

    public static native char toLower(char c);

    public static native char toUpper(char c);

    public static boolean isLowerCase(char ch) {
        return toUpper(ch) != ch;
    }

    public static boolean isUpperCase(char ch) {
        return toLower(ch) != ch;
    }

    public static boolean isDigit(char ch) {
        return (('0' <= ch) && (ch <= '9'));
    }

    public static int digit(char ch, int radix) {
        if('A' <= ch && ch <= 'Z')
            ch += 32;
        int ret = -1;
        if('0' <= ch && ch <= '9')
            ret = ch - '0';
        else if('a' <= ch && ch <= 'z')
            ret = ch - 'a' + 10;
        if(ret >= radix)
            return -1;
        return ret;
    }

    public Character(char value) {
        this.value = value;
    }

    public static Character valueOf(char c) {
        return new Character(c);
    }

    public char charValue() {
        return value;
    }

    @Override
    public int hashCode() {
        return Character.hashCode(value);
    }

    public static int hashCode(char value) {
        return (int)value;
    }

    @Override
    public boolean equals(Object obj) {
        if(obj instanceof Character)
            return value == ((Character)obj).charValue();
        return false;
    }

    @Override
    public String toString() {
        return String.valueOf(value);
    }

    public static String toString(char c) {
        return String.valueOf(c);
    }

    @Override
    public int compareTo(Character anotherCharacter) {
        return value - anotherCharacter.value;
    }

    public static int compare(char x, char y) {
        return x - y;
    }
}
