package homework_seconda_implementazione;

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

	public byte[] hash(byte[] pass)
	{
		hasher.update(pass);
		return this.hasher.digest();
	}
}
