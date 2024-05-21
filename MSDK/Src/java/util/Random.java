package java.util;

public class Random {
    private Random() {

    }

    public static boolean nextBoolean() {
        return (nextInt() & 0x01) == 1;
    }

    public static void nextBytes(byte[] bytes) {
        for(int i = 0, len = bytes.length; i < len;) {
            int rnd = nextInt();
            int n = Math.min(len - i, Integer.SIZE / Byte.SIZE);
            while(n-- > 0) {
                bytes[i++] = (byte)rnd;
                rnd >>= Byte.SIZE;
            }
        }
    }

    public static native int nextInt();

    public static long nextLong() {
        long ret = (long)nextInt() << 32;
        ret |= nextInt();
        return ret;
    }

    public static float nextFloat() {
        return nextInt() * 0x1.0p-24f;
    }

    public static double nextDouble() {
        return (nextLong() >>> (Double.SIZE - Double.PRECISION)) * 0x1.0p-53;
    }
}
