package java.lang;
import uj.lang.*;

public class Thread implements Runnable{

	public Thread(Runnable what){
	
		uj.lang.RT.threadCreate(what);	
	}
	
	public void run(){
	
			
	}
}