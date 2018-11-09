public class UC{

//clocking
	public static int clkGet(){return 0;}		//get num clocks expired so far
	public static int clkSpeed(){return 0;}		//get num clock ticks per second
	public static int instrsGet(){return 0;}	//number of java instructions executed so far
	
//GPIO
	public static int gpioGetNumPorts(){return 0;}
	public static int gpioGetPortPins(){return 0;}
	public static void gpioSetOutput(int port, int pin, boolean isOutput){}
	public static boolean gpioIsHi(int port, int pin){return true;}
	public static void gpioSetHi(int port, int pin, boolean isOutput){}	//if input, make it a pullup
	
//EEPROM
	public static int eepromSize(){return 0;}
	public static void eepromWrite(int addr, byte val){}
	public static byte eepromRead(int addr){return 0;}
	
//Character LCD
	public static void lcdClear(){}
	public static void lcdGotoXY(int x, int y){}
	public static void lcdChar(char c){}
	public static void lcdRawWrite(int v){}					//issue a raw command to the LCD
	
//Graphical LCD
	public static void glcdClear(){}
	public static void glcdSetPixels(int rowOver8, int col, byte vals){}
	public static void glcdSetManyPixels(int rowOver8, int col, byte[] manyVals, int arrayOffset, int numCols){}
	
//PWMs
	public static final int PWM_TYPE_WHITE_LED	= 0;
	public static final int PWM_TYPE_LCD_BACKLITE	= 1;	
	public static void pwmSetBri(int type, int bri){}

//I2C
	public static void i2cStart(){}
	public static boolean i2cSend(byte b){return false;}
	public static byte i2cRecv(boolean ack){return 0;}
	public static void i2cStop(){}

//raw memory access
	public static byte rawMemRead(int addr){return 0;}
	public static void rawMemWrite(int addr, byte val){}
}