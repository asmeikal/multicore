package homework_seconda_implementazione;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

public class Master
{
	private ArrayList<byte[]> hashes;
	private int length;
	private GeneratorFactory gf;
	private AllStringGenerator asg;
	private WorkerThread[] workerThreads;
	private HashMap<byte[], byte[]> passwordMap;
	
	/*private Comparator<byte[]> comp = new Comparator<byte[]>() {
        @Override
        public int compare(byte[] x, byte[] y) {

            if (x.length < y.length) {return -1;}
            if (x.length > x.length) {return 1;}

            for (int i=0; i < x.length; i++) {
                if (x[i] < y[i]) { return -1; }
                else if (x[i] > y[i]) {  return 1; }
                }
            
            return 0;
        }
        };*/

	private Comparator<byte[]> comp = (x,y) -> {
                if (x.length < y.length) {return -1;}
        	if (x.length > y.length) {return 1;}

        	for (int i=0; i < x.length; i++) {
            	   if      (x[i] < y[i]) { return -1; }
                   else if (x[i] > y[i])  {return 1;}
                }
                return 0;
        };


	public Master(ArrayList<String> hashes, int n_thread, int length)
	{	
		this.hashes = new TreeSet<byte[]>(comp);
        	this.hashes.addAll(MD5.StringToByteArray(hashes));
		this.workerThreads = new WorkerThread[n_thread];
		this.length = length;
		this.gf = new GeneratorFactory("abcdefghijklmnopqrstuvwxyz0123456789".getBytes(), this.length);
		this.asg = gf.getASG();
		this.passwordMap = new HashMap<byte[], byte[]>();
	}

	public void work() throws InterruptedException
	{
		MD5 md5 = new MD5();

		// controllare le prime parole in sequenziale
		byte[] word;
		for (int i = 1; i <= this.length; ++i) {
			word = new byte[i];
			StringGenerator sg = this.gf.getSG(i);
			while (sg.hasWords()) {
				sg.getNextWord(word, 0);
				byte[] hash = md5.hash(word);
				if (this.hashes.contains(hash)) {
                    		    foundPassword(hash,word);
				}
			}
		}

		// il master fa lavorare i thread

		for (int i = 0; i < this.workerThreads.length; i++) {
			workerThreads[i] = new WorkerThread(this, this.asg, gf.getSG());
			workerThreads[i].start();
		}
		for (int i = 0; i < workerThreads.length; i++) {
			workerThreads[i].join();
		}

		System.out.println(this.passwordMap);

	}

	synchronized public void foundPassword(byte[] hash, byte[] pass)
	{
	    TreeSet<byte[]> t = new TreeSet<>(comp);
            t.addAll(this.hashes);
            t.remove(hash);
            this.hashes = t;
	}

	synchronized public ArrayList<byte[]> getPasswords()
	{
		return this.hashes;
	}
}
