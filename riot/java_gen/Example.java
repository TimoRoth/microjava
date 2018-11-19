public class Example
{
	public static String toString(int v)
	{
		int t = v;
		int len = 0;
		byte [] ret;
		boolean neg = false;

		if(v < 0){
			neg = true;
			len++;
			v = -v;
		}

		do{
			len++;
			t /= 10;
		}while(t != 0);

		ret = new byte[len];

		do{
			ret[--len] = (byte)((v % 10) + (int)'0');
			v /= 10;
		}while(v != 0);

		if(neg) ret[--len] = '-';

		return new String(ret);
	}

	public static void main() {
		RIOT.printString("Hi :)");

		while (true) {
			int eventId = RIOT.waitEvent(1 * 1000000);
			RIOT.printString("Hmm: " + toString(eventId));
			switch (eventId)
			{
			case RIOT.EVT_GPIO_INT:
				int gpio_pin = RIOT.getEventParam(0);
				RIOT.printString("GPIO Event on " + toString(gpio_pin));
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
