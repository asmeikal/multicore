package homework_prima_implementazione;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

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

	public String hash(String pass)
	{
		hasher.update(pass.getBytes());
		byte hash[] = this.hasher.digest();

		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < hash.length; ++i) {
			sb.append(Integer.toString((hash[i] & 0xFF) + 0x100, 16).substring(1));
		}

		return sb.toString();
	}
}
