package homework_prima_implementazione;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

public class Master
{
	// hashes - password da trovare
	private HashSet<String> hashes;

    // passwordMap - hash -> password trovata
	private Map<String, String> passwordMap;

	private AllStringGenerator asg;
	private WorkerThread[] workerThreads;

	// lunghezza delle password che si lavora il master
	private int length;

	private GeneratorFactory gf;


	/**
	 * Parametrizzare: numero di thread; lunghezza password da dare ai
	 * thread.
	 */
	public Master(HashSet<String> hashes, int n_thread, int length)
	{
		this.hashes = hashes;
		this.workerThreads = new WorkerThread[n_thread];
		this.length = length;

		this.gf = new GeneratorFactory("abcdefghijklmnopqrstuvwxyz0123456789",this.length);

		this.asg = this.gf.getASG();
		this.length = length;
        this.passwordMap = new HashMap<String, String>();

	}

    public void work() throws InterruptedException
	{
        MD5 md5 = new MD5();

		// generare le prime parole e controllarle
		for (int i = 1; i <= this.length; ++i) {
			StringGenerator sg = gf.getSG(i);
			while (sg.hasWords()) {
                String w = sg.getNextWord();
                String hash = md5.hash(w);
                if (this.hashes.contains(hash)) {
                    this.foundPassword(hash,w);
				}
            }
		}
		for (int i = 0; i < this.workerThreads.length; i++) {
			workerThreads[i] = new WorkerThread(this, this.asg, gf.getSG());
			workerThreads[i].start();
		}
		for (int i=0; i < workerThreads.length; i++) {
			workerThreads[i].join();
		}

		System.out.println(this.passwordMap);
	}

    synchronized public void foundPassword(String hash, String pass)
	{
		this.hashes.remove(hash);
		this.passwordMap.put(hash, pass);
		System.out.println("Found password: " + pass);
	}

	synchronized public HashSet<String> getPasswords()
	{
		// ritorna un hash set di password ancora da trovare
		HashSet<String> copy = new HashSet<String>();
        copy.addAll(this.hashes);
		return copy;
	}
}
