package java.lang;

public class Throwable {
    private String detailMessage;

    public Throwable() {
        detailMessage = null;
    }

    public Throwable(String message) {
        detailMessage = message;
    }

    public String getMessage() {
        return detailMessage;
    }

    public String getLocalizedMessage() {
        return detailMessage;
    }

    public String toString() {
        return this.getClass().getName() + ": " + detailMessage;
    }
}
