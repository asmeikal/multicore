package homework_seconda_implementazione;

import java.util.HashSet;
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

		final byte[] alphabet = "abcdefghijklmnopqrstuvwxyz0123456789".getBytes();
		HashSet<byte[]> passwords = new HashSet<byte[]>();
		Random r = new Random();
		MD5 md5 = new MD5();

		for (int i = 1; i <= p_length; ++i) {
			/*
			 * Generate n_pass * p_length passwords of length
			 * p_length
			 */
			for (int j = 0; j < n_pass * i; ++j) {
				byte[] s = new byte[i];
				for (int k = 0; k < i; ++k) {
					s[k] = alphabet[r.nextInt(alphabet.length - 1)];
				}
				System.out.println("Adding password " + new String(s));
				passwords.add(md5.hash(s));
			}
			/*
			 * Generate the last password in lexicographic order
			 * with length p_length
			 */
			byte[] s = new byte[i];
			for (int k = 0; k < i; ++k) {
				s[k] = alphabet[alphabet.length - 1];
			}
			System.out.println("Adding password " + new String(s));
			passwords.add(md5.hash(s));
		}

		System.out.println("This is the set of hashes we're testing against:");
		System.out.println(passwords);

		Master m = new Master(passwords, n_threads, length);
		m.work();

	}
}
