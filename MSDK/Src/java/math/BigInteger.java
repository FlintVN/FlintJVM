package java.math;

public class BigInteger extends Number implements Comparable<BigInteger> {
    public static BigInteger valueOf(long val) {
        return new BigInteger(val);
    }

    public BigInteger(byte[] val, int off, int len) {
        // TODO
    }

    public BigInteger(byte[] val) {
        this(val, 0, val.length);
    }

    public BigInteger(int signum, byte[] magnitude, int off, int len) {
        // TODO
    }

    public BigInteger(int signum, byte[] magnitude) {
        this(signum, magnitude, 0, magnitude.length);
    }

    public BigInteger(String val, int radix) {
        // TODO
    }

    public BigInteger(String val) {
        this(val, 10);
    }

    private BigInteger(long val) {
        // TODO
    }

    public BigInteger add(BigInteger val) {
        // TODO
    }

    public BigInteger subtract(BigInteger val) {
        // TODO
    }

    public BigInteger multiply(BigInteger val) {
        // TODO
    }

    public BigInteger divide(BigInteger val) {
        // TODO
    }

    public BigInteger remainder(BigInteger val) {
        // TODO
    }
    public BigInteger pow(int exponent) {
        // TODO
    }

    public BigInteger sqrt() {
        // TODO
    }

    public BigInteger gcd(BigInteger val) {
        // TODO
    }

    public BigInteger abs() {
        // TODO
    }

    public BigInteger negate() {
        // TODO
    }

    public BigInteger mod(BigInteger m) {
        // TODO
    }

    public BigInteger modPow(BigInteger exponent, BigInteger m) {
        // TODO
    }

    public BigInteger modInverse(BigInteger m) {
        // TODO
    }

    public BigInteger shiftLeft(int n) {
        // TODO
    }

    public BigInteger shiftRight(int n) {
        // TODO
    }

    public BigInteger and(BigInteger val) {
        // TODO
    }

    public BigInteger or(BigInteger val) {
        // TODO
    }

    public BigInteger xor(BigInteger val) {
        // TODO
    }

    public BigInteger not() {
        // TODO
    }

    public BigInteger andNot(BigInteger val) {
        // TODO
    }

    public BigInteger setBit(int n) {
        // TODO
    }

    public BigInteger clearBit(int n) {
        // TODO
    }

    public BigInteger flipBit(int n) {
        // TODO
    }

    public BigInteger min(BigInteger val) {
        // TODO
    }

    public BigInteger max(BigInteger val) {
        // TODO
    }

    public BigInteger nextProbablePrime() {
        // TODO
    }

    public int intValue() {
        // TODO
        return 0;
    }

    public long longValue() {
        // TODO
    }

    public float floatValue() {
        // TODO
    }

    public double doubleValue() {
        // TODO
    }

    public byte byteValue() {
        return (byte)intValue();
    }

    public short shortValue() {
        return (short)intValue();
    }

    public int compareTo(BigInteger val) {
        // TODO
        return 0;
    }
}
