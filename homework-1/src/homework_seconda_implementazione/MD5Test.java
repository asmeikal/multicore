package homework_seconda_implementazione;

import java.util.Arrays;

public class MD5Test
{

	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		MD5 m = new MD5();
		byte[] hash = m.hash("ciao".getBytes());

		System.out.println(Arrays.toString(hash));
		int hash2[] = new int[hash.length];
		for (int i = 0; i < hash.length; ++i) {
			hash2[i] = (hash[i] & 0xFF);
		}
		System.out.println(Arrays.toString(hash2));
		for (int i = 0; i < hash.length; ++i) {
			hash2[i] = hash2[i] + 0x100;
		}
		System.out.println(Arrays.toString(hash2));
		StringBuilder sa = new StringBuilder();
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < hash2.length; ++i) {
			sa.append(Integer.toString(hash2[i], 16).substring(1) + " ");
			sb.append(Integer.toString(hash2[i], 16) + " ");
		}

		System.out.println(sb.toString());
		System.out.println(sa.toString());

		String h = "6e6bc4e49dd477ebc98ef4046c067b5f";
		byte[] r = new byte[h.length() / 2];
		for (int i = 0; i < h.length() / 2; ++i) {
			r[i] = (byte) Integer.valueOf(h.substring(i * 2, (i * 2) + 2), 16).intValue();
		}
		System.out.println(Arrays.toString(r));

	}
}
