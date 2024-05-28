package java.lang;

public final class Class<T> {
    private transient String name;

    private Class() {

    }

    @Override
    public String toString() {
        String kind = isInterface() ? "interface " : isPrimitive() ? "" : "class ";
        return kind.concat(getName());
    }

    static native Class<?> getPrimitiveClass(String name);

    public native static Class<?> forName(String className) throws ClassNotFoundException;

    public native boolean isInstance(Object obj);

    public native boolean isAssignableFrom(Class<?> cls);

    public native boolean isInterface();

    public native boolean isArray();

    public native boolean isPrimitive();

    public String getName() {
        return name;
    }

    public native Class<? super T> getSuperclass();

    public String getPackageName() {
        Class<?> c = isArray() ? elementType() : this;
        if(c.isPrimitive())
            return "java.lang";
        else {
            String cn = c.getName();
            int dot = cn.lastIndexOf('.');
            return (dot != -1) ? cn.substring(0, dot).intern() : "";
        }
    }

    public native Class<?>[] getInterfaces();

    public native Class<?> getComponentType();

    private Class<?> elementType() {
        if(!isArray())
            return null;

        Class<?> c = this;
        while(c.isArray())
            c = c.getComponentType();
        return c;
    }

    public native int getModifiers();

    public String getSimpleName() {
        String simpleName = getName();
        int arrayCount = 0;
        int startIndex = simpleName.lastIndexOf('.');
        int endIndex = simpleName.length();
        while(simpleName.charAt(arrayCount) == '[')
            arrayCount++;
        startIndex = (startIndex < 0) ? arrayCount : (startIndex + 1);
        if((endIndex - startIndex) == 1) {
            char ch = simpleName.charAt(startIndex);
            simpleName = switch(ch) {
                case 'Z' -> "boolean";
                case 'C' -> "char";
                case 'F' -> "float";
                case 'D' -> "double";
                case 'B' -> "byte";
                case 'S' -> "short";
                case 'I' -> "int";
                case 'J' -> "long";
                default -> String.valueOf(ch);
            };
        }
        else {
            if(arrayCount > 0 && startIndex == arrayCount && simpleName.charAt(arrayCount) == 'L')
                startIndex++;
            if(simpleName.charAt(endIndex - 1) == ';')
                endIndex--;
            simpleName = simpleName.substring(startIndex, endIndex);
        }
        if(arrayCount > 0)
            simpleName = simpleName.concat("[]".repeat(arrayCount));
        return simpleName;
    }

    public String getTypeName() {
        if(isArray()) {
            try {
                Class<?> cl = this;
                int dimensions = 0;
                do {
                    dimensions++;
                    cl = cl.getComponentType();
                } while(cl.isArray());
                return cl.getName().concat("[]".repeat(dimensions));
            }
            catch(Throwable e) {

            }
        }
        return getName();
    }

    private static boolean arrayContentsEq(Object[] a1, Object[] a2) {
        if(a1 == null)
            return a2 == null || a2.length == 0;
        if(a2 == null)
            return a1.length == 0;
        if(a1.length != a2.length)
            return false;
        for(int i = 0; i < a1.length; i++) {
            if(a1[i] != a2[i])
                return false;
        }
        return true;
    }

    public boolean isEnum() {
        return (this.getModifiers() & 0x00004000) != 0;
    }

    public boolean isRecord() {
        return (this.getModifiers() & 0x00000010) != 0;
    }

    @SuppressWarnings("unchecked")
    public T cast(Object obj) {
        if(obj != null && !isInstance(obj))
            throw new ClassCastException(cannotCastMsg(obj));
        return (T)obj;
    }

    private String cannotCastMsg(Object obj) {
        return "Cannot cast " + obj.getClass().getName() + " to " + getName();
    }

    public native boolean isHidden();
}
