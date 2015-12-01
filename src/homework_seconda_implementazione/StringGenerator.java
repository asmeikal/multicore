package homework_seconda_implementazione;

import java.math.BigInteger;
import java.util.Arrays;

public class StringGenerator
{
	protected final byte[] alphabet;
	protected final int alpha_len;
	protected final int length;
	protected final int max_len;
	protected int counter;

	public StringGenerator(byte alphabet[], int length)
	{
		BigInteger max = BigInteger.valueOf(alphabet.length).pow(length);
		if (max.compareTo(BigInteger.valueOf(Integer.MAX_VALUE)) >= 0) {
			throw new RuntimeException("Non posso creare un generatore di parole lunghe " + length + " su un alfabeto di " + alphabet.length + " caratteri.");
		}

		this.max_len = max.intValue();
		this.alpha_len = alphabet.length;
		this.alphabet = Arrays.copyOf(alphabet, this.alpha_len);
		this.length = length;
		this.reset();
	}

	public void getNextWord(byte arr[], int start)
	{
		if ((arr.length - start) != this.length) {
			throw new RuntimeException("Mi stai chiedendo di riempire " + (arr.length - start) + " locazioni di un array con una stringa lunga " + this.length + ". Non posso farlo.");
		}
		if (!this.hasWords()) {
			throw new RuntimeException("Mi dispiace, ma ho finito le parole.");
		}
		int tmp_counter = this.counter;
		this.counter++;
		for (int i = 1; i <= this.length; ++i) {
			arr[start + (this.length - i)] = this.alphabet[tmp_counter % this.alpha_len];
			tmp_counter /= this.alpha_len;
		}
	}

	public boolean hasWords()
	{
		return this.counter < this.max_len;
	}

	public void reset()
	{
		this.counter = 0;
	}
}
