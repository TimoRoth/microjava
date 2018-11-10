import uj.lang.*;

public class Wii{

	static byte[] gScreenData;

	private static void printString(String s){
	
		int i, L = s.Xlen_();			//we cheat by not claling string methods - they are slow
		
		for(i = 0; i < L; i++) uj.lang.RT.consolePut((char)s.XbyteAt_(i));
	}
	
	private static byte[] i2cOp(byte[] in, int numOut){
	
		byte[] out = null;
		int i, L;
		
		L = in.length;
		
		UC.i2cStart();
		UC.i2cSend((byte)0xA4);
		for(i = 0; i < L; i++) UC.i2cSend(in[i]);
		if(numOut != 0){
			out = new byte[numOut];
			UC.i2cStart();
			UC.i2cSend((byte)0xA5);
			for(i = 0; i < numOut; i++) out[i] = UC.i2cRecv(i != numOut - 1);
		}
		UC.i2cStop();
		
		return out;
	}

	private static void puts(String s){
	
		int i, L = s.Xlen_();			//we cheat by not claling string methods - they are slow
		
		for(i = 0; i < L; i++) UC.lcdChar((char)s.XbyteAt_(i));
	}

	private static void setPix(int r, int c, boolean on){
	
		//screen is as follows: 8 char rows, each col of which is 8 px high (one byte), 128 cols in screen
		byte mask = (byte)(1 << (r & 7));
		int rowOver8 = r >> 3;
		int idx = c + (rowOver8 << 7);
		
		if(on) mask = (byte)(gScreenData[idx] | mask);
		else mask = (byte)(gScreenData[idx] &~ mask);
		
		gScreenData[idx] = mask;

		UC.glcdSetPixels(rowOver8, c, gScreenData[idx]);
	}
	
	private static boolean getPix(int r, int c){
	
		byte mask = (byte)(1 << (r & 7));
		int rowOver8 = r >> 3;
		int idx = c + (rowOver8 << 7);
		
		return (gScreenData[idx] & mask) != 0;
	}

	private static void erasePix(){
	
		gScreenData = new byte [1024];		//cheap and easy - let garbage collector handle the rest :)	
	
		//now actually erase the screen
	
		byte row [] = new byte[128];
		
		for(int i = 0; i < 8; i++) UC.glcdSetManyPixels(i, 0, row, 0, 128);
	}
	
	private static void cube(int eyeX, int eyeY, int eyeZ, boolean on){
		
		UC.lcdGotoXY(0, 0);
		puts(toString(eyeX) + " _ " + toString(eyeY) + " _ " + toString(eyeZ) + "    ");
	}
	
	private static void line(int x0, int y0, int x1, int y1, boolean on){
		
		int dx = x1 - x0;
		int dy = y1 - y0;
		
		if(dx < 0) dx = -dx;
		if(dy < 0) dy = -dy;
		
		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;

		int err = dx - dy;
		while(true){
			
			setPix(y0, x0, on);
			if(x0 == x1 && y0 == y1) return;
			
			int e2 = err << 1;
			
			if(e2 > -dy){
				
				err -= dy;
				x0 += sx;
			}
			if(e2 < dx){
				
				err += dx;
				y0 += sy;
			}
		}
	}
	
	public static int sqrt(int x){
	
		int guess = 0x5A82;	//approx sqrt(0x7FFFFFFF)/2
		int step = (guess + 1) >> 1;
		int approx;
		
		while(step != 0){
		
			approx = guess * guess;
			if(approx == x) break;
			else if(approx > x) guess -= step;
			else guess += step;
			step = step >> 1;	
		}
		
		return guess;
	}

	public static void main(){
	
		//draw mode vals
			int posX = 64, posY = 32;
			boolean posV = false;
		boolean drawMode = true;
		
		erasePix();
		
		//init
		
		i2cOp(new byte[]{(byte)0xF0, (byte)0x55}, 0);
		i2cOp(new byte[]{(byte)0xFB, 0}, 0);
		i2cOp(new byte[]{(byte)0xFA}, 6);
		
		
		//send crypto key
		
		i2cOp(new byte[]{(byte)0xF0, (byte)0xAA}, 0);
		i2cOp(new byte[]{(byte)0x40, 0, 0, 0, 0, 0, 0}, 0);
		i2cOp(new byte[]{(byte)0x40, 0, 0, 0, 0, 0, 0}, 0);
		i2cOp(new byte[]{(byte)0x40, 0, 0, 0, 0}, 0);
		
		
		//loop
		while(true){
		
			byte[] vals = i2cOp(new byte[]{0}, 6);
			int joyX, joyY, accX, accY, accZ, t;
			boolean Z, C;
			
			joyX = ((int)vals[0]) & 0xFF;
			joyY = ((int)vals[1]) & 0xFF;
			
			accX = (((int)vals[2]) & 0xFF) << 2;
			accY = (((int)vals[3]) & 0xFF) << 2;
			accZ = (((int)vals[4]) & 0xFF) << 2;
			
			t = ((int)vals[5]) & 0xFF;
			
			Z = (t & 0x01) == 0;
			C = (t & 0x02) == 0;
			
			if((t & 0x04) != 0) accX += 2;
			if((t & 0x08) != 0) accX += 1;
			
			if((t & 0x10) != 0) accY += 2;
			if((t & 0x20) != 0) accY += 1;
			
			if((t & 0x40) != 0) accZ += 2;
			if((t & 0x80) != 0) accZ += 1;
			
			
			if(drawMode){
				if(C && Z){	//switch mode
				
					erasePix();
					posV = false;
					drawMode = false;
				}
				else{
					if(C) posV = true;		//C writes
					else if(Z) posV = false;	//Z erases
				
					setPix(posY, posX, posV);
					posX = joyX >> 1;
					posY = (255 - joyY) >> 2;
					posV = getPix(posY, posX);
					setPix(posY, posX, true);
				}
			}
			else{		//cube mode
			
				if(C && Z){	//switch mode
				
					erasePix();
					drawMode = true;
				}
				else{
					//if holding nunchuck as expected:
					//	X axis runs left to right (positives to the right)
					//	Y axis runs from user to joystick (positives at joystick)
					//	Z axis runs from top to bottom (positives on bottom of remote)
				
				
					accX -= 512;
					accY -= 512;
					accZ -= 512;
					UC.lcdGotoXY(0, 0);
					puts(toString(accX) + " _ " + toString(accY) + " _ " + toString(accZ) + "    ");
					UC.lcdGotoXY(0, 1);
					puts("MAG:" + toString(sqrt(accX * accX + accY * accY + accZ * accZ)) + "   ");
				}
			}
		}
	}

	private static String toString(int v){
	
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
}