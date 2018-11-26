public class I2C
{
    public static final int ADDR10 = 1;
    public static final int REG16 = 2;
    public static final int NOSTOP = 4;
    public static final int NOSTART = 8;

    public static int dev(int num) { return 0; }

    public static void init(int dev) {}
    public static boolean acquire(int dev) { return false; }
    public static boolean release(int dev) { return false; }

    public static byte read_reg(int dev, int addr, int reg, int flags) { return 0; }
    public static byte[] read_regs(int dev, int addr, int reg, int len, int flags) { return null; }
    public static boolean write_reg(int dev, int addr, int reg, byte data, int flags) { return false; }
    public static boolean write_regs(int dev, int addr, int reg, byte[] data, int flags) { return false; }

    public static byte read_byte(int dev, int addr, int flags) { return 0; }
    public static byte[] read_bytes(int dev, int addr, int len, int flags) { return null; }
    public static boolean write_byte(int dev, int addr, byte data, int flags) { return false; }
    public static boolean write_bytes(int dev, int addr, byte[] data, int flags) { return false; }
}