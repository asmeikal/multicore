package homework_seconda_implementazione;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;

public class MD5
{
	private MessageDigest hasher;

	public MD5()
	{
		try {
			this.hasher = MessageDigest.getInstance("md5");
		} catch (NoSuchAlgorithmException e) {
			throw new RuntimeException("Impossibile creare hasher md5.");
		}
	}

	static byte[] StringToByteArray(String s)
	{
		byte[] r = new byte[s.length() / 2];
		for (int i = 0; i < s.length() / 2; ++i) {
			r[i] = (byte) Integer.valueOf(s.substring(i * 2, (i * 2) + 2), 16).intValue();
		}
		return r;
	}

	static ArrayList<byte[]> StringToByteArray(Iterable<String> s)
	{
		ArrayList<byte[]> result = new ArrayList<byte[]>();

		for (String str : s) {
			result.add(MD5.StringToByteArray(str));
		}
		return result;
	}

	public byte[] hash(byte[] pass)
	{
		hasher.update(pass);
		return this.hasher.digest();
	}
}
