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
		if (s.length() != 32) {
			throw new RuntimeException("La stringa \"" + s + "\" non sembra rappresentare un hash MD5.");
		}
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

	static String ByteArrayToString(byte[] b)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < b.length; ++i) {
			sb.append(Integer.toString((b[i] & 0xFF) + 0x100, 16).substring(1));
		}

		return sb.toString();
	}

	public byte[] hash(byte[] pass)
	{
		hasher.update(pass);
		return this.hasher.digest();
	}

	public String hash(String pass)
	{
		return MD5.ByteArrayToString(this.hash(pass.getBytes()));
	}
}
