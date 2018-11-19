package java.lang;
import uj.lang.*;

public class StringBuilder{

	private byte bytes[];

	public StringBuilder(){

		bytes = new byte[0];
	}

	/* -- untested
	public StringBuilder(int cap){

		bytes = new byte[0];
	}
	*/

	public StringBuilder append(String s){

		int i, j;
		byte n[];

		if(s == null) s = "null";

		n = new byte[bytes.length + s.Xlen_()];
		for(i = 0; i < bytes.length; i++) n[i] = bytes[i];
		for(j = 0; j < s.Xlen_(); j++, i++) n[i] = s.XbyteAt_(j);

		bytes = n;

		return this;
	}

	public StringBuilder append(char c){

		byte str[];

		if(c <= 0x7F){

			str = new byte[1];
			str[0] = (byte)c;
		}
		else if(c <= 0x7FF){

			str = new byte[2];
			str[1] = (byte)((c & 0x3F) | 0x80);
			c >>= 6;
			str[0] = (byte)(c | 0xC0);
		}
		else{

			str = new byte[2];
			str[2] = (byte)((c & 0x3F) | 0x80);
			c >>= 6;
			str[1] = (byte)((c & 0x3F) | 0x80);
			c >>= 6;
			str[0] = (byte)(c | 0xE0);
		}

		return append(new String(str));
	}

	public StringBuilder append(int v){

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

		return append(new String(ret));
	}

	public StringBuilder append(float f){

		int exp = 0;

		exp = 0;
		if(f < 0){
				f = -f;
				append("-");
		};

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
		append((int)f).append(".");

		for(int i = 0; i < 5; i++){ //doubles are good to 16 digits or so

				f -= (int)f;
				f *= 10.0f;
				append((char)(((int)f) + (int)'0'));
		}
		if(exp != 0) append("*10^").append(exp);

		return this;
	}

	public StringBuilder append(short v){

		return append((int)v);
	}

	public StringBuilder append(byte v){

		return append((int)v);
	}

	public StringBuilder append(boolean v){

		return append(v ? "true" : "false");
	}

	/* -- untested
	public StringBuilder insert(int offset, char c){

		int i;
		append(" ");	//make space

		for(i = bytes.length - 1; i >= offset; i--){

			bytes[i + 1] = bytes[i];
		}
		bytes[offset] = (byte)c;

		return this;
	}
	*/

	public StringBuilder append(Object o){

		return append(o.toString());
	}

	public String toString(){

		return new String(bytes);
	}
}