package homework_prima_implementazione;

class AllStringGenerator
{
	final private String alphabet;
	private int size;
	private StringGenerator current_gen;

	public AllStringGenerator(String alphabet)
	{
		this.alphabet = alphabet;
		this.size = 1;
		this.current_gen = new StringGenerator(this.alphabet, this.size);
	}

	synchronized public String getNextWord()
	{
		if (!this.current_gen.hasWords()) {
			this.size += 1;
			this.current_gen = new StringGenerator(this.alphabet, this.size);
		}
		return this.current_gen.getNextWord();
	}

}
