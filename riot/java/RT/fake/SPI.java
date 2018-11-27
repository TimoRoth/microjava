public class SPI
{
    public static final int CS_UNDEF = -1;

    public static final int MODE_0 = 0;
    public static final int MODE_1 = 1;
    public static final int MODE_2 = 2;
    public static final int MODE_3 = 3;

    public static final int CLK_100KHZ = 0;
    public static final int CLK_400KHZ = 1;
    public static final int CLK_1MHZ = 2;
    public static final int CLK_5MHZ = 3;
    public static final int CLK_10MHZ = 4;

    public static int dev(int num) { return 0; }
    public static int hwcs(int num) { return 0; }

    public static void init(int bus) {}
    public static void init_pins(int bus) {}
    public static boolean init_cs(int bus, int cs) { return false; }

    public static boolean acquire(int bus, int cs, int mode, int clk) { return false; }
    public static void release(int bus) {}

    public static byte transfer_byte(int bus, int cs, boolean cont, byte out) { return 0; }
    public static byte[] transfer_bytes(int bus, int cs, boolean cont, byte[] out) { return null; }

    public static byte transfer_reg(int bus, int cs, int reg, byte out) { return 0; }
    public static byte[] transfer_regs(int bus, int cs, int reg, byte[] out) { return null; }
}