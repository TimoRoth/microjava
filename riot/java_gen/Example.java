public class Example implements Runnable {
	private int del;

	public Example(int delaynum) {

		del = delaynum * 234;	//approx in centiseconds
	}

	public void run(){

		int v = del;
		printString(toString(v));
		printString("\nDas ist ein Test\n");
	}

	public static void printString(String s) {

		int i, L = s.Xlen_();			//we cheat by not claling string methods - they are slow

		for(i = 0; i < L; i++) uj.lang.RT.consolePut((char)s.XbyteAt_(i));
	}

	public static String toString(int v) {

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
		String s = "aefsdfsdfg12423542435\n";

		Example ct = new Example(RIOT.printString("Hallo Print String!"));
		ct.run();

		printString(s);

		s = RIOT.getSomeString();

		RIOT.sayHello();

		RIOT.printString(s);

		RIOT.printString(toString(RIOT.getSomeString().Xlen_()));

		RIOT.printString("Umlaut test: öäüß");
	}
}
