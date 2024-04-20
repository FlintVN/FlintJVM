package java.lang;

public final class Object {
    public Object() {

    }

    public final native Class<?> getClass();

    public native int hashCode();

    protected native Object clone() throws CloneNotSupportedException;

    public boolean equals(Object obj) {
        return (this == obj);
    }

    public String toString() {
        return getClass().getName() + "@" + Integer.toHexString(hashCode());
    }
}
