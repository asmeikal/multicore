package homework_prima_implementazione;

import java.util.ArrayList;
import java.util.Random;

public class MainClass
{
	public static void main(String args[]) throws InterruptedException
	{
		int n_threads = Integer.parseInt(args[0]);
		int length = Integer.parseInt(args[1]);
		int p_length = Integer.parseInt(args[2]);
		int n_pass = Integer.parseInt(args[3]);

		assert n_threads > 0;
		assert length > 0;
		assert p_length > 0;
		assert n_pass > 0;

		final String alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
		ArrayList<String> passwords = new ArrayList<String>();
		Random r = new Random();
		MD5 md5 = new MD5();

		for (int i = 1; i <= p_length; ++i) {
			/*
			 * Generate n_pass * p_length passwords of length
			 * p_length
			 */
			for (int j = 0; j < n_pass * i; ++j) {
				StringBuilder s = new StringBuilder();
				for (int k = 0; k < i; ++k) {
					s.append(alphabet.charAt(r.nextInt(alphabet.length() - 1)));
				}
				System.out.println("Adding password " + s.toString());
				passwords.add(md5.hash(s.toString()));
			}
			/*
			 * Generate the last password in lexicographic order
			 * with length p_length
			 */
			StringBuilder s = new StringBuilder();
			for (int k = 0; k < i; ++k) {
				s.append(alphabet.charAt(alphabet.length() - 1));
			}
			System.out.println("Adding password " + s.toString());
			passwords.add(md5.hash(s.toString()));
		}

		System.out.println("This is the set of hashes we're testing against:");
		System.out.println(passwords);

		Master m = new Master(passwords, alphabet, n_threads, length);
		m.work();

	}
}
