public class COAP
{
    public static final int GET = 1;
    public static final int POST = 2;
    public static final int PUT = 4;
    public static final int DELETE = 8;

    public static final int CLASS_SUCCESS = 2;
    public static final int CODE_CREATED = 65;
    public static final int CODE_DELETED = 66;
    public static final int CODE_VALID = 67;
    public static final int CODE_CHANGED = 68;
    public static final int CODE_204 = 68;
    public static final int CODE_CONTENT = 69;
    public static final int CODE_205 = 69;
    public static final int CODE_231 = 95;

    public static final int CLASS_CLIENT_FAILURE = 4;
    public static final int CODE_BAD_REQUEST = 128;
    public static final int CODE_UNAUTHORIZED = 129;
    public static final int CODE_BAD_OPTION = 130;
    public static final int CODE_FORBIDDEN = 131;
    public static final int CODE_PATH_NOT_FOUND = 132;
    public static final int CODE_404 = 132;
    public static final int CODE_METHOD_NOT_ALLOWED = 133;
    public static final int CODE_NOT_ACCEPTABLE = 134;
    public static final int CODE_REQUEST_ENTITY_INCOMPLETE = 136;
    public static final int CODE_PRECONDITION_FAILED = 140;
    public static final int CODE_REQUEST_ENTITY_TOO_LARGE = 141;
    public static final int CODE_UNSUPPORTED_CONTENT_FORMAT = 143;

    public static final int CLASS_SERVER_FAILURE = 5;
    public static final int CODE_INTERNAL_SERVER_ERROR = 160;
    public static final int CODE_NOT_IMPLEMENTED = 161;
    public static final int CODE_BAD_GATEWAY = 162;
    public static final int CODE_SERVICE_UNAVAILABLE = 163;
    public static final int CODE_GATEWAY_TIMEOUT = 164;
    public static final int CODE_PROXYING_NOT_SUPPORTED = 165;

    public static final int FORMAT_TEXT = 0;
    public static final int FORMAT_LINK = 40;
    public static final int FORMAT_XML = 41;
    public static final int FORMAT_OCTET = 42;
    public static final int FORMAT_EXI = 47;
    public static final int FORMAT_JSON = 50;
    public static final int FORMAT_CBOR = 60;

    public static final int METHOD_GET = 1;
    public static final int METHOD_POST = 2;
    public static final int METHOD_PUT = 3;
    public static final int METHOD_DELETE = 4;

    public static final int MEMO_UNUSED = 0;
    public static final int MEMO_WAIT = 1;
    public static final int MEMO_RESP = 2;
    public static final int MEMO_TIMEOUT = 3;
    public static final int MEMO_ERR = 4;

    // SERVER

    // Important: Resources have to be registered in alphabetical order!
    public static int registerResource(String path, int methods) { return -1; }
    public static void finishRegistration() {}

    // CLIENT

    public static int sendRequest(int method, String addr, int port, String path) { return -1; }
    public static int sendRequest(int method, String addr, int port, String path, int format, String payload) { return -1; }
    public static int sendRequest(int method, String addr, int port, String path, int format, byte[] payload) { return -1; }
}