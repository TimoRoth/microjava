public class Default
{
	static int a_req;
	static int b_req;
	static int c_req;

	private static void handleCoapReq()
	{
		int req_id = RIOT.getEventParam(0);
		int method = RIOT.getEventParam(1);
		String data = RIOT.getEventParamStr(2);

		if (req_id == c_req)
			RIOT.printString("In /java/c request handler!");

		RIOT.printString("Got CoAP request id " + req_id + " with method " + method);
		if (data != null)
			RIOT.printString("Data: " + data);
		else
			RIOT.printString("no data");

		RIOT.replyEvent(RIOT.EVT_COAP_REPLY, COAP.CODE_CONTENT, COAP.FORMAT_TEXT, "Test Reply\n");
	}

	public static void main()
	{
		RIOT.printString("Hi :)");

		int pin = GPIO.pin(0, 12); // Port A12 on bluepill, GPIO12 on ESP32

		if (!GPIO.init_int(pin, GPIO.IN_PD, GPIO.FALLING))
			RIOT.printString("GPIO init failed");

		int button_pin = GPIO.pin(0, 0); // Boot button on ESP32

		if (!GPIO.init_int(button_pin, GPIO.IN_PD, GPIO.FALLING))
			RIOT.printString("GPIO button init failed");

		RIOT.printString("Waiting for GPIO events from pin " + pin + " and " + button_pin);

		// These have to be in alphabetical order!
		a_req = COAP.registerResource("/java/a", COAP.GET);
		b_req = COAP.registerResource("/java/b", COAP.PUT);
		c_req = COAP.registerResource("/java/c", COAP.GET | COAP.PUT);
		COAP.finishRegistration();

		while (true) {
			int eventId = RIOT.waitEvent(30 * 1000000);
			switch (eventId)
			{
			case RIOT.EVT_GPIO_INT:
				int gpio_pin = RIOT.getEventParam(0);
				RIOT.printString("GPIO Event on " + gpio_pin);
				break;
			case RIOT.EVT_UPD_RDY:
				RIOT.printString("Rebooting for update!");
				return;
			case RIOT.EVT_EXIT:
				RIOT.printString("bye");
				return;
			case RIOT.EVT_COAP_REQ: // We HAVE to reply to this, otherwise the CoAP thread hangs forever.
				handleCoapReq();
				break;
			case RIOT.EVT_NONE:
			case -1:
				// No event occured, either timeout or spurious wakeup.
				break;
			default:
				RIOT.printString("Unhandeld event ID: " + eventId);
				break;
			}
		}
	}
}
