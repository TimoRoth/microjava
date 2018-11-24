public class RIOT {
    public static final int EVT_NONE = 0x00;
    public static final int EVT_GPIO_INT = 0x01;
    public static final int EVT_COAP_REQ = 0x02;
    public static final int EVT_COAP_REPLY = 0x03;
    public static final int EVT_TIMER = 0x04;
    public static final int EVT_UPD_RDY = 0xFD;
    public static final int EVT_GENERIC = 0xFE;
    public static final int EVT_EXIT = 0xFF;

    public static int printString(String s) { return 0; }

    public static int waitEvent(int timeout_us) { return 0; }
    public static int getEventParam(int idx) { return 0; };
    public static String getEventParamStr(int idx) { return null; };
    public static byte[] getEventParamBytes(int idx) { return null; };
    public static boolean replyEvent(int reply_type, int param1, int param2, byte[] data) { return true; }
    public static boolean replyEvent(int reply_type, int param1, int param2, String data) { return true; }

    public static void setTimeoutUS(int timeout_us, int userdata) {}
    public static void setTimeoutS(int timeout_s, int userdata) {}

    public static void usleep(int usec) {}
}