public class Animals{

	private final static int KEY_S2 = 1;
	private final static int KEY_S3 = 2;
	private final static int KEY_S4 = 4;
	

	private static byte gKbdChars[] =	{
							'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T',
							'U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9','?','_','.',','
						};

	private static int getKeys(){
	
		int ret = 0;
		
		if(!UC.gpioIsHi(2, 2)) ret += KEY_S2;
		if(!UC.gpioIsHi(2, 3)) ret += KEY_S3;
		if(!UC.gpioIsHi(2, 4)) ret += KEY_S4;
		
		return ret;
	}

	private static int waitKey(){
	
		int i;
		
		while((i = getKeys()) == 0);	//wait for press
		while(getKeys() != 0);		//wait for release
		
		return i;
	}

	private static byte kbd(){
	
		int first = 0;
		int len = gKbdChars.length;
		int mid, i;
		
		while(len > 1){
		
			UC.lcdClear();
			mid = len / 2;
			
			UC.lcdGotoXY(0, 0);
			for(i = 0; i < mid; i++) UC.lcdChar((char)gKbdChars[i + first]);
			
			UC.lcdGotoXY(0, 1);
			for(i = mid; i < len; i++) UC.lcdChar((char)gKbdChars[i + first]);
			
			i = waitKey();
			
			if(i == KEY_S2) len = mid;
			else if(i == KEY_S3) {
			
				len -= mid;
				first += mid;	
			}
			else if(i == KEY_S4){
			
				if(len == gKbdChars.length) return 0;		//from tpo level we abort
				else{
				
					first = 0;				//prom any other level we restart the character
					len = gKbdChars.length;
				}
			}
		}
		
		i = gKbdChars[first];
		if(i == '_') i = ' ';
		
		return (byte)i;
	}
	
	private static byte[] input(){
	
		int i;
		byte c;
		byte str[] = new byte[0];
		
		while((c = kbd()) != 0){
		
			byte[] tmp = new byte[str.length + 1];
			for(i = 0; i < str.length; i++) tmp[i] = str[i];
			tmp[str.length] = c;
			str = tmp;
		}
		
		return str;
	}

	private static void init(){
	
		UC.gpioSetOutput(2, 2, false);
		UC.gpioSetOutput(2, 3, false);
		UC.gpioSetOutput(2, 4, false);
		
		UC.gpioSetHi(2, 2, true);
		UC.gpioSetHi(2, 3, true);
		UC.gpioSetHi(2, 4, true);
		
		if(eeRead16(5) == 0xFFFF){	//no data
		
			eeWrite16(5, 7);	//root pointer
			eeWrite16(7, 0xFFFF);
			eeWrite16(9, 0xFFFF);
			eeWriteString(11, new byte[] {'D','O','G'});
		}	
	}

	private static void showStr(byte[] str){
		
		int col = 0;
		int i;
		
		UC.lcdClear();
		UC.lcdGotoXY(0, 0);
		for(i = 0; i < str.length; i++) {
			
			UC.lcdChar((char)str[i]);
			col++;
			if(col == 20) UC.lcdGotoXY(0, 1);
		}
	}

	private static int eeRead16(int addr){
	
		int ret;
		
		ret = ((int)UC.eepromRead(addr + 0)) & 0xFF;
		ret <<= 8;
		ret += ((int)UC.eepromRead(addr + 1)) & 0xFF;
		
		return ret;
	}
	
	private static void eeWrite16(int addr, int val){
	
		UC.eepromWrite(addr + 0, (byte)(val >> 8));
		UC.eepromWrite(addr + 1, (byte)(val));
	}
	
	private static int eeFindEnd(){
	
		int pos = UC.eepromSize() - 1;
		
		while(pos >= 5 && (((int)UC.eepromRead(pos)) & 0xFF) == 0xFF) pos--;
		
		return pos + 1;
	}
	
	private static byte[] eeReadString(int addr){
	
		int i, len = ((int)UC.eepromRead(addr++)) & 0xFF;
		byte[] ret = new byte[len];
		
		for(i = 0; i < len; i++) ret[i] = UC.eepromRead(addr++);
		
		return ret;
	}
	
	private static void eeWriteString(int addr, byte[] str){
	
		int i;
		
		UC.eepromWrite(addr++, (byte)str.length);
		
		for(i = 0; i < str.length; i++) UC.eepromWrite(addr++,str[i]);
	}

	public static void main(){
	
		int ptr, pptr = 5;
		byte[] str;
		int i;
		
		init();
		
		while(true){
		
			ptr = eeRead16(pptr);
			str = eeReadString(ptr + 4);
			
			showStr(str);
			i = waitKey();
			
			if(eeRead16(ptr + 0) == 0xFFFF && eeRead16(ptr + 2) == 0xFFFF){	//leaf
			
				if(i == KEY_S2) pptr = 5;	//all good - restart
				else if(i == KEY_S3){		//process new entry
				
					showStr(new byte[]{'E','n','t','e','r',' ','n','e','w',' ','Q',' ','s','t',' ','y','o','u','r',' ','a','n','s','w','=','Y','/','N',' ','m','i','n','e','=','N','/','Y'});
					i = waitKey();
					if(i == KEY_S2){	//proceed
						
						str = input();
						
						showStr(new byte[]{'E','n','t','e','r',' ','y','o','u','r',' ','a','n','s','w','e','r'});
						i = waitKey();
						if(i == KEY_S2){	//proceed
							
							byte[] answ = input();
							int newOfst;
							
							
							showStr(new byte[]{'Y','E','S','=','m','y',' ','a','n','s','w','e','r','?'});
							i = waitKey();
							
							if(i != KEY_S4){
								
								newOfst = (i == KEY_S2) ? 2 : 0;
								
								i = eeFindEnd();
								if(UC.eepromSize() - i >= 4 + answ.length + 1 + 4 + str.length + 1){	//we have space
								
									int newLeaf;
									
									//write new leaf node
									newLeaf = i;
									eeWrite16(i, 0xFFFF);
									eeWrite16(i + 2, 0xFFFF);
									eeWriteString(i + 4, answ);
									i += 4 + 1+ answ.length;
									
									//write new branch node
									eeWrite16(i + newOfst, newLeaf);
									eeWrite16(i + 2 - newOfst, ptr);
									eeWriteString(i + 4, str);
									
									//re-point parent node to new branch node
									eeWrite16(pptr, i);
								}
								else{
									showStr(new byte[]{'O','u','t',' ','o','f',' ','E','E','P','R','O','M'});
									waitKey();
								}
							}
						}
					}
					pptr = 5;		//restart
				}
			}
			else{	//branch
			
				if(i == KEY_S2) pptr = ptr;
				else if(i == KEY_S3) pptr = ptr + 2;
			}
		}
	}
}