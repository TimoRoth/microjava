public class Default
{
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

		while (true) {
			int eventId = RIOT.waitEvent(30 * 1000000);
			switch (eventId)
			{
			case RIOT.EVT_GPIO_INT:
				int gpio_pin = RIOT.getEventParam(0);
				RIOT.printString("GPIO Event on " + gpio_pin);
				break;
			case RIOT.EVT_EXIT:
				RIOT.printString("bye");
				return;
			case RIOT.EVT_NONE:
			case -1:
				// No event occured, either timeout or spurious wakeup.
				RIOT.printString("cycling");
				break;
			}
		}
	}
}
