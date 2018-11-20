public class RIOT {
    public static final int EVT_NONE = 0x00;
    public static final int EVT_GPIO_INT = 0x01;
    public static final int EVT_EXIT = 0xFF;

    public static int printString(String s) { return 0; }

    public static int waitEvent(int timeout_us) { return 0; }
    public static int getEventParam(int idx) { return 0; };
    public static String getEventParamStr(int idx) { return ""; };

    public static void usleep(int usec) {}
}