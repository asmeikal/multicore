package homework_seconda_implementazione;

import java.util.Arrays;

public class AllStringGenerator
{
	protected final byte[] alphabet;
	protected final int alpha_len;
	protected int[] word;
	protected int word_len;

	public AllStringGenerator(byte[] alphabet)
	{
		this.alphabet = Arrays.copyOf(alphabet, alphabet.length);
		this.alpha_len = alphabet.length;
		this.word_len = 1;
		this.word = new int[this.word_len];
		Arrays.fill(this.word, 0);
	}

	private void increment()
	{
		int i = 0;
		boolean done = false;
		this.word[i] += 1;
		while (!done) {
			if (this.word[i] >= this.alpha_len) {
				this.word[i] = 0;
				if (i + 1 < this.word_len) {
					this.word[i + 1] += 1;
					++i;
				} else {
					this.word_len += 1;
					this.word = new int[this.word_len];
					Arrays.fill(this.word, 0);
					done = true;
				}
			} else {
				done = true;
			}
		}
	}

	public byte[] getNextWord()
	{
		int[] w_copy;
		synchronized (this) {
			w_copy = Arrays.copyOf(this.word, this.word.length);
			this.increment();
		}
		byte[] result = new byte[w_copy.length];

		for (int i = 1; i <= w_copy.length; ++i) {
			result[w_copy.length - i] = this.alphabet[w_copy[i - 1]];
		}

		return result;
	}
}
