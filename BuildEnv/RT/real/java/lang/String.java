package java.lang;
import uj.lang.*;

public class String extends MiniString{

	public String(byte[] bytes){	//our constructor for raw bytes
	
		super(bytes);
	}

	/*
		this class is FORBIDDEN from having any instance variables dues to how it inherits from
			uj.lang.miniString and how we construct these in the VM
	*/
	

	
	/* -- untested
	public String substring(int beginIndex, int endIndex){
	
		//this is not the right way to do this - will only work for ascii!
		byte[] arr = new byte[endIndex - beginIndex];
		int i;
		
		for(i = beginIndex; i < endIndex; i++){
			
			arr[i - beginIndex] = XbyteAt_(i);
		}
		
		return MiniString.Xnew_(arr);
	}
	
	public String substring(int beginIndex){
	
		return substring(beginIndex, length());
	}
	*/
	
	
}