package homework_prima_implementazione;

import java.math.BigInteger;

class StringGenerator
{
	final private String alphabet;
	final private int length;

	final private BigInteger alpha_len;
	private BigInteger counter;
	private String nextWord;

	public StringGenerator(String alphabet, int length)
	{
		this.alphabet = alphabet;
		this.alpha_len = BigInteger.valueOf(alphabet.length());
		this.length = length;
		this.reset();
	}

	private void createNextWord()
	{
		StringBuilder newWord = new StringBuilder();
		BigInteger tmp = this.counter;
		this.counter = this.counter.add(BigInteger.ONE);
		for (int i = 0; i < this.length; ++i) {
			int index = tmp.mod(this.alpha_len).intValue();
			newWord.insert(0, this.alphabet.charAt(index));
			tmp = tmp.divide(this.alpha_len);
		}
		if (tmp.compareTo(BigInteger.ZERO) > 0) {
			this.nextWord = null;
		} else {
			this.nextWord = newWord.toString();
		}
	}

	public String getNextWord()
	{
		if (!this.hasWords()) {
			throw new RuntimeException("Non ho parole!");
		}
		String result = this.nextWord;
		this.createNextWord();
		return result;
	}

	public boolean hasWords()
	{
		return this.nextWord != null;
	}

	public void reset()
	{
		this.counter = BigInteger.ZERO;
		this.createNextWord();
	}

}
