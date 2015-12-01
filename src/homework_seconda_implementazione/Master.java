package homework_seconda_implementazione;

import java.util.HashSet;

public class Master
{
	// HashSet<byte[]>

	// stessi metodi

	synchronized public void foundPassword(byte hash[], byte pass[]);

	synchronized public HashSet<byte[]> getPasswords();

}
