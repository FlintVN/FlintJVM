package java.lang;

import java.lang.annotation.*;

@Retention(RetentionPolicy.SOURCE)
public @interface SuppressWarnings {
    String[] value();
}
