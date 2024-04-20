package java.lang;

public class NumberFormatException extends IllegalArgumentException {
    public NumberFormatException () {
        super();
    }

    public NumberFormatException (String s) {
        super(s);
    }

    static NumberFormatException forInputString(String s, int radix) {
        return new NumberFormatException("For input string: \"" + s + "\"" + (radix == 10 ? "" : " under radix " + radix));
    }

    static NumberFormatException forCharSequence(CharSequence s, int beginIndex, int endIndex, int errorIndex) {
        return new NumberFormatException("Error at index " + (errorIndex - beginIndex) + " in: \"" + s.subSequence(beginIndex, endIndex) + "\"");
    }
}
