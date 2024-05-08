package java.lang;

public interface CharSequence {
    int length();

    char charAt(int index);

    default boolean isEmpty() {
        return this.length() == 0;
    }

    CharSequence subSequence(int start, int end);

    public String toString();

    public static int compare(CharSequence cs1, CharSequence cs2) {
        if(cs1 != null && cs1 == cs2)
            return 0;

        if(cs1.getClass() == cs2.getClass() && cs1 instanceof Comparable)
            return ((Comparable<Object>)cs1).compareTo(cs2);

        for(int i = 0, len = Math.min(cs1.length(), cs2.length()); i < len; i++) {
            char a = cs1.charAt(i);
            char b = cs2.charAt(i);
            if(a != b)
                return a - b;
        }

        return cs1.length() - cs2.length();
    }
}
