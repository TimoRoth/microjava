class LedRunner implements Runnable{

	public void run(){
	
		int led = 0;
		int dir = 1;
		int t;
	
		while(true){
		
			UC.gpioSetHi(3, led, true);
			t = UC.clkGet();
			while(UC.clkGet() - t < 1953);	//10 LED switches per second
			UC.gpioSetHi(3, led, false);
			led += dir;
			if(led < 0 || led > 7){
				dir = -dir;
				led += 2 * dir;
			}
		}
	}
}

public class DianaTest implements Runnable
{

	private static synchronized void lcdString(int row, int col, String s){
	
		int i, L = s.length();
		
		UC.lcdGotoXY(col, row);
		for(i = 0; i < L; i++) UC.lcdChar(s.charAt(i));
	}

	private static void lcdCenteredString(int row, String s){
	
		int L = s.length();
		
		if(L > 16) return;
	
		lcdString(row, (16 - s.length()) / 2, s);
	}

	public void run(){
	
		int cand, t;
		final String rotChars[] = {"\0", "\1", "\2", "\3"};
		int rotIdx = 0;
		int animCtr = 0;
		
		//if we already ran, we'd have a candidate in the eeprom - read it
		cand = 0;
		for(t = 0; t < 4; t++) cand = (cand << 8) + UC.eepromRead(t);
		
		//if cand is 0xFFFFFFFF (that is, negative 1), then we had no candidate and strat from scratch
		if(cand < 0) cand = 1;
		
		while(true){
		
			cand++;
			for(t = 2; t < cand; t++){
				if((cand % t) == 0) break;
				
				if(++animCtr == 100){
					animCtr = 0;
					lcdString(1, 15, rotChars[rotIdx]);
					if(++rotIdx == rotChars.length) rotIdx = 0;
				}
			}
			if(t == cand){
				
				lcdCenteredString(1, intToString(cand));
				
				//save the value to EEPROM
				for(t = 0; t < 4; t++) UC.eepromWrite(3 - t, (byte)(cand >> (t << 3)));
			}
		}
	}

	private static void initCustomChars(){
	
		//see here for help: http://www.quinapalus.com/hd44780udg.html
		int i, j;
		final byte customChars[][] =	{
							{0x00, 0x0E, 0x15, 0x15, 0x11, 0x0E, 0x00, 0x00},
							{0x00, 0x0E, 0x11, 0x17, 0x11, 0x0E, 0x00, 0x00},
							{0x00, 0x0E, 0x11, 0x15, 0x15, 0x0E, 0x00, 0x00},
							{0x00, 0x0E, 0x11, 0x1D, 0x11, 0x0E, 0x00, 0x00}
						};
		
		//init custom LCD characters 1..4
		
		UC.lcdRawWrite(0x40);	//enter configuration mode
		
		for(i = 0; i < customChars.length; i++){
		
			for(j = 0; j < customChars[i].length; j++){
				
				UC.lcdChar((char)customChars[i][j]);
			}
		}
		
		UC.lcdRawWrite(0x80);	//exit configuration mode
	}

	public static void main(){
	
		int h = 0, m = 0, s = 0, d, t, L;
		char hc1, hc2;
		Thread primeThread, ledThread;
		String time;
		
		//init things
		initCustomChars();
		UC.lcdClear();
		for(t = 0; t < 8; t++){
		
			UC.gpioSetOutput(3, t, false);	//use pull-ups to light leds
			UC.gpioSetHi(3, t, false);
		}
		UC.pwmSetBri(1, 64);
		UC.pwmSetBri(0, 1);
		
		//start the other threads
		primeThread = new Thread(new DianaTest());
		ledThread =  new Thread(new LedRunner());
		
		
		//adc test
		{
		
			UC.rawMemWrite(0x7C,(byte)0x5E);	//ADMUX:  measure 1.1V with ref at Vcc
			UC.rawMemWrite(0x7A,(byte)0xC7);	//ADCSRA: ADC on, single conversion, no int, 1:128 prescaler
			
			while((UC.rawMemRead(0x7A) & 0x40) != 0);	//wait
			
			t = UC.rawMemRead(0x79);	//read result.hi
			t <<= 8;
			t += UC.rawMemRead(0x78);	//read result.lo
			
			t = (1024 * 1100 + (t >> 1)) / t;
			
			lcdCenteredString(0, "Batt: " + intToString(t / 1000) + "." + intToString(t % 1000) + "V");
		
			t = UC.clkGet();
			while(UC.clkGet() - t < 39063);	//wait 2 sec
		}
		
		//begin clock work
		d = UC.clkGet();
		while(true){
			
			t = h % 12;
			if(t == 0) t = 12;
			
			time = intToString(t / 10) + intToString(t % 10);
			time += ":" + intToString(m / 10) + intToString(m % 10);
			time += ":" + intToString(s / 10) + intToString(s % 10);
			time += "  " + (h >= 12 ? "PM" : "AM");
			
			lcdCenteredString(0, time);
			
			if(60 == ++s){
				
				s = 0;
				if(60 == ++m){
					
					m = 0;
					if(24 == ++h) h = 0;
				}
			}
			while((t = UC.clkGet()) - d < 19531);
			d = d + 19531;
		}
	}
	
	public static String intToString(int v){
	
		int t;
		int len = 0;
		byte [] ret;
		boolean neg = false;
		
		if(v < 0){
			neg = true;
			len++;
			v = -v;
		}
		
		t = v;
		
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
}