public class CoolTest implements Runnable{

	private int del;

	public CoolTest(int delaynum){
	
		del = delaynum * 234;	//approx in centiseconds
	}
	
	public void run(){
	
		int v = del;
		
		while(true){
		
			UC.gpioSetHi(0, 7, !UC.gpioIsHi(0,7));
			
			int x = UC.clkGet();
			while(UC.clkGet() - x < v);
		}	
	}

	public static void printString(String s){
	
		int i, L = s.Xlen_();			//we cheat by not claling string methods - they are slow
		
		for(i = 0; i < L; i++) uj.lang.RT.consolePut((char)s.XbyteAt_(i));	
	}
	
	public static String toString(int v){
	
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
		
		return uj.lang.MiniString.Xnew_(ret);
	}

	public static String toString(float f){
	
		String ret;
		int exp, man;

		man = 0;
		exp = 0;
		if(f < 0){
			f = -f;
			ret = "-";
		} else ret = "";
		
		if(f < 0.001f || f >= 100000.f){
		
			while(f >= 10.0f){
				exp++;
				f /= 10.0f;
			}
			while(f < 1.0f){
				exp--;
				f *= 10.0f;
			}
		}
		ret = ret + toString((int)f) + ".";

		while(ret.length() < 10){	//doubles are good to 16 digits or so
		
			f -= (int)f;
			f *= 10.0f;
			ret = ret + (char)(((int)f) + (int)'0');
			man++;
		}
		if(exp != 0) ret = ret + "*10^" + toString(exp);
		
		return ret;
	}

	public static void main(){
		
		int clk = UC.clkGet();
		
		UC.gpioSetOutput(0, 7, true);
		
		int i = 200, j;
		int needThreads = 10;
		Thread[] threads = new Thread[needThreads];
		int haveThreads = 0;
		
		while(haveThreads < needThreads){
		
			for(j = 2; j < i; j++) if(i % j == 0) break;
			if(i == j){
				threads[haveThreads++] = new Thread(new CoolTest(i));
				printString("Started thread " + toString(haveThreads) + " with period " + toString(i) + "\n");
			}
			i++;
		}
		
		clk = UC.clkGet() - clk;
		int instrs = UC.instrsGet();
		
		float c = ((float)clk)/23437.5f;
		
		printString(toString(instrs) + " instrs in " + toString(clk) + " ticks (" + toString(c) + "s) -> " + toString((float)instrs / c) + " instrs/sec (" + toString((float)clk * 1024f / (float)instrs) + " instrs/instr).\n");
		
		while(true);
	}
}