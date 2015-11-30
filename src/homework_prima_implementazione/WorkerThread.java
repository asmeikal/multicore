package homework_prima_implementazione;

import java.util.HashSet;

public class WorkerThread extends Thread
{
	private MD5 hasher;
	private Master master;
	private AllStringGenerator asg;
	private StringGenerator sg;

	public WorkerThread(Master m, AllStringGenerator asg, StringGenerator sg)
	{
		this.master = m;
		this.asg = asg;
		this.sg = sg;
		
		// creare un MD5
		this.hasher = new MD5();
	}

	public void run()
	{
        HashSet<String> s = this.master.getPasswords();
		while ( !s.isEmpty()) {
			String w1 = this.asg.getNextWord();
			while (this.sg.hasWords()) {
				String w2 = this.sg.getNextWord();
				String w3 = w1 + w2;
				String hash = this.hasher.hash(w3);
				if (s.contains(hash)) {
					this.master.foundPassword(hash, w3);
				}
			}
			this.sg.reset();
            s = this.master.getPasswords();
		}
	}
}
