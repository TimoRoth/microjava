public class Default
{
	static int a_req;
	static int b_req;
	static int c_req;

	static final int i2c_timer_timeout_s = 60;
	static final int i2c_timer = 100;
	static int i2c_dev;

	static final int spi_timer_timeout_s = 30;
	static final int spi_timer = 200;
	static int spi_dev;
	static int spi_cs;

	private static void handleCoapReq()
	{
		int req_id = RIOT.getEventParam(0);
		int method = RIOT.getEventParam(1);

		if (req_id == c_req)
			RIOT.printString("In /java/c request handler!");

		RIOT.printString("Got CoAP request id " + req_id + " with method " + method);

		if (req_id == b_req) {
			String data = RIOT.getEventParamStr(2);
			if (data != null)
				RIOT.printString("Data: " + data);
			else
				RIOT.printString("no data");
		} else if(req_id == c_req) {
			byte[] data = RIOT.getEventParamBytes(2);
			if (data != null)
				RIOT.printString("Got " + data.length + " bytes data: " + new String(data));
			else
				RIOT.printString("no bytes data");
		}

		RIOT.replyEvent(RIOT.EVT_COAP_REPLY, COAP.CODE_CONTENT, COAP.FORMAT_TEXT, "Test Reply\n");
	}

    private static void handleCoapResp()
    {
        int req_id = RIOT.getEventParam(0);
		int req_state = RIOT.getEventParam(1);

        if (req_state == COAP.MEMO_TIMEOUT || req_state == COAP.MEMO_ERR) {
            RIOT.printString("Error sending COAP request " + req_id + ": " + req_state);
            return;
        }

        int resp_code = RIOT.getEventParam(2);
        int resp_format = RIOT.getEventParam(3);
        String payload = RIOT.getEventParamStr(4);

        RIOT.printString("Got reply to COAP request " + req_id + " with code " + resp_code + " and format " + resp_format);

        if (payload != null)
            RIOT.printString("Request payload: " + payload);
    }

	private static void handleI2CTimer()
	{
		RIOT.setTimeoutS(i2c_timer_timeout_s, i2c_timer);

		I2C.acquire(i2c_dev);
		byte[] data = { 0x01, 0x34, 0x00, (byte)0xff };
		I2C.write_regs(i2c_dev, 0, 2, data, 0);
		byte res = I2C.read_byte(i2c_dev, 4, 0);
		I2C.release(i2c_dev);

		RIOT.printString("I2C res on dev " + i2c_dev + ": " + res);
	}

	private static void handleSPITimer()
	{
		RIOT.setTimeoutS(spi_timer_timeout_s, spi_timer);

		SPI.acquire(spi_dev, spi_cs, SPI.MODE_0, SPI.CLK_1MHZ);
		byte[] data = { 0x01, 0x34, 0x00, (byte)0xff };
		byte[] res = SPI.transfer_bytes(spi_dev, spi_cs, false, data);
		SPI.release(spi_dev);

		RIOT.printString("SPI res on dev " + spi_dev + " cs " + spi_cs + ": " + new String(res));
	}

	private static void handleTimerEvent()
	{
		int timer_id = RIOT.getEventParam(0);
		RIOT.printString("Timer event: " + timer_id);

		switch(timer_id)
		{
			case i2c_timer:
				handleI2CTimer();
				break;
			case spi_timer:
				handleSPITimer();
				break;
		}
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

		RIOT.printString("Random int: " + RIOT.random());
		RIOT.printString("Random int range 55 to 555: " + RIOT.randomRange(55, 555));
		RIOT.printString("Random int range -555 to -55: " + RIOT.randomRange(-555, -55));

		// These have to be in alphabetical order!
		a_req = COAP.registerResource("/java/a", COAP.GET);
		b_req = COAP.registerResource("/java/b", COAP.PUT);
		c_req = COAP.registerResource("/java/c", COAP.GET | COAP.PUT);
		COAP.finishRegistration();

        int get_req_id = COAP.sendRequest(COAP.METHOD_GET, "fd20::1", 5683, "/test/get_request");
        RIOT.printString("Sending COAP GET request with ID " + get_req_id);

        int put_req_id = COAP.sendRequest(COAP.METHOD_PUT, "fd20::1", 5683, "/test/put_request", COAP.FORMAT_TEXT, "Test data to be put!");
        RIOT.printString("Sending COAP PUT request with ID " + put_req_id);

		// Not available on all boards, comment in if desired
		/* i2c_dev = I2C.dev(0);
		I2C.init(i2c_dev);
		RIOT.setTimeoutS(i2c_timer_timeout_s, i2c_timer); */

		/* spi_dev = SPI.dev(0);
		spi_cs = GPIO.pin(0, 14); // depending on hw cs channels: SPI.hwcs(0)
		if (SPI.init_cs(spi_dev, spi_cs))
			RIOT.setTimeoutS(spi_timer_timeout_s, spi_timer);
		else
			RIOT.printString("SPI init failed."); */

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
            case RIOT.EVT_COAP_RESP:
                handleCoapResp();
                break;
			case RIOT.EVT_TIMER:
				handleTimerEvent();
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
