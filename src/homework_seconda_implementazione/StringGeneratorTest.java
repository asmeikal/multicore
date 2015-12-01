package homework_seconda_implementazione;

import java.util.Arrays;

public class StringGeneratorTest
{
	public static void main(String args[])
	{
		final String alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
		StringGenerator sg = new StringGenerator(alphabet.getBytes(), 2);
		System.out.println("Il mio alfabeto: " + alphabet + " (lunghezza " + alphabet.length() + ").");
		System.out.println();

		System.out.println("Creo parole nella forma b**.");
		byte[] arr = new byte[3];
		arr[0] = 'b';
		int i = 1;
		sg.getNextWord(arr, 1);
		System.out.println("Prima parola: " + new String(arr) + " (bytes: " + Arrays.toString(arr) + ")");
		while (sg.hasWords()) {
			++i;
			sg.getNextWord(arr, 1);
		}
		System.out.println("Ultima parola: " + new String(arr) + " (bytes: " + Arrays.toString(arr) + ")");
		System.out.println("Ho creato " + i + " parole.");
		System.out.println();

		System.out.println("Reset del generatore.");
		sg.reset();
		System.out.println();

		System.out.println("Creo parole nella forma **.");
		arr = new byte[2];
		i = 1;
		sg.getNextWord(arr, 0);
		System.out.println("Prima parola: " + new String(arr) + " (bytes: " + Arrays.toString(arr) + ")");

		while (sg.hasWords()) {
			++i;
			sg.getNextWord(arr, 0);
		}
		System.out.println("Ultima parola: " + new String(arr) + " (bytes: " + Arrays.toString(arr) + ")");
		System.out.println("Ho creato " + i + " parole.");
		System.out.println();

		/* All these should fail */
		try {
			System.out.println("new StringGenerator(alphabet.getBytes(), 10);");
			new StringGenerator(alphabet.getBytes(), 10);
		} catch (RuntimeException e) {
			System.out.println(e);
		}

		try {
			System.out.println("sg.getNextWord(arr, 0);");
			sg.getNextWord(arr, 0);
		} catch (RuntimeException e) {
			System.out.println(e);
		}

		try {
			System.out.println("sg.getNextWord(arr, 1);");
			sg.getNextWord(arr, 1);
		} catch (RuntimeException e) {
			System.out.println(e);
		}
	}
}
