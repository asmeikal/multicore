package homework_seconda_implementazione;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.TreeSet;

public class Master
{
	private TreeSet<byte[]> hashes;
	private int length;
	private GeneratorFactory gf;
	private AllStringGenerator asg;
	private WorkerThread[] workerThreads;
	private Map<String, String> passwordMap;

	private Comparator<byte[]> comp = new Comparator<byte[]>()
	{
		@Override
		public int compare(byte[] x, byte[] y)
		{

			if (x.length < y.length) {
				return -1;
			} else if (x.length > y.length) {
				return 1;
			}

			for (int i = 0; i < x.length; i++) {
				if (x[i] < y[i]) {
					return -1;
				} else if (x[i] > y[i]) {
					return 1;
				}
			}

			return 0;
		}
	};

	// private Comparator<byte[]> comp8 = (x, y) -> {
	// if (x.length < y.length) {
	// return -1;
	// } else if (x.length > y.length) {
	// return 1;
	// }
	//
	// for (int i = 0; i < x.length; i++) {
	// if (x[i] < y[i]) {
	// return -1;
	// } else if (x[i] > y[i]) {
	// return 1;
	// }
	// }
	// return 0;
	// };

	public Master(ArrayList<String> hashes, String alphabet, int n_thread, int length)
	{
		this.hashes = new TreeSet<byte[]>(comp);
		this.hashes.addAll(MD5.StringToByteArray(hashes));
		this.workerThreads = new WorkerThread[n_thread];
		this.length = length;
		this.gf = new GeneratorFactory(alphabet.getBytes(), this.length);
		this.asg = gf.getASG();
		this.passwordMap = new HashMap<String, String>();
	}

	public Map<String, String> work() throws InterruptedException
	{
		MD5 md5 = new MD5();

		byte[] word;
		for (int i = 1; i <= this.length; ++i) {
			word = new byte[i];
			StringGenerator sg = this.gf.getSG(i);
			while (sg.hasWords()) {
				sg.getNextWord(word, 0);
				byte[] hash = md5.hash(word);
				if (this.hashes.contains(hash)) {
					foundPassword(hash, word);
				}
			}
		}

		for (int i = 0; i < this.workerThreads.length; i++) {
			workerThreads[i] = new WorkerThread(this, this.asg, gf.getSG());
			workerThreads[i].start();
		}

		for (int i = 0; i < workerThreads.length; i++) {
			workerThreads[i].join();
		}

		return this.passwordMap;

	}

	synchronized public void foundPassword(byte[] hash, byte[] pass)
	{
		TreeSet<byte[]> t = new TreeSet<>(comp);
		t.addAll(this.hashes);
		t.remove(hash);
		this.hashes = t;
		this.passwordMap.put(MD5.ByteArrayToString(hash), new String(pass));
	}

	synchronized public TreeSet<byte[]> getPasswords()
	{
		return this.hashes;
	}
}
