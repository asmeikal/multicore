package homework_prima_implementazione;

import java.util.HashSet;

public class MainClass
{
	public static void main(String args[]) throws InterruptedException
	{
		// apre file con hsah
		// prende in input numero processori e lunghezza parole
		int n_threads = Integer.parseInt(args[0]);
		int length = Integer.parseInt(args[1]);
		// int n_threads = 4;
		// int length = 3;


		HashSet<String> passwords = new HashSet<String>();
		passwords.add("6e6bc4e49dd477ebc98ef4046c067b5f");
		passwords.add("cdd7a616f977556bf3bce39917a37a30");
		passwords.add("e7e9ec3723447a642f762b2b6a15cfd7");
		System.out.println(passwords);

		Master m = new Master(passwords, n_threads, length);
		m.work();

		// print results
	}
}
