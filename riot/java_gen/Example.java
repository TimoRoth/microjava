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
		if (GPIO.init(GPIO.pin(0, 1), GPIO.OUT))
			RIOT.printString("init OK");
		else
			RIOT.printString("init not ok");
	}
}
