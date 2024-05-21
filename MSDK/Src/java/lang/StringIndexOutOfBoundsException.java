package java.lang;

public class StringIndexOutOfBoundsException extends IndexOutOfBoundsException {
    public StringIndexOutOfBoundsException() {
        super();
    }

    public StringIndexOutOfBoundsException(String s) {
        super(s);
    }

    public StringIndexOutOfBoundsException(int index) {
        super("String index out of range: " + index);
    }
}
