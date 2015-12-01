package homework_prima_implementazione;

import java.util.HashSet;
import java.util.Random;

public class MainClass
{
	public static void main(String args[]) throws InterruptedException
	{
		// apre file con hsah
		// prende in input numero processori e lunghezza parole
		int n_threads = Integer.parseInt(args[0]);
		int length = Integer.parseInt(args[1]);

		MD5 md5 = new MD5();
		Random r = new Random();
		String alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
		HashSet<String> passwords = new HashSet<String>();

		passwords.add(md5.hash("99999"));

		for (int i = 0; i < 5; ++i) {
			StringBuilder s = new StringBuilder();
			for (int j = 0; j < 5; ++j) {
				s.append(alphabet.charAt(r.nextInt(alphabet.length() - 1)));
			}
			passwords.add(md5.hash(s.toString()));
		}

		System.out.println(passwords);

		Master m = new Master(passwords, n_threads, length);
		m.work();

		// print results
	}
}
