package homework_prima_implementazione;

public class GeneratorFactory
{
	private final String alphabet;
	private final int length;

	public GeneratorFactory(String alphabet, int length)
	{
		this.alphabet = alphabet;
		this.length = length;
	}

	// ritorna un generatore di parole
	// di lunghezza fissa impostata quando
	// si crea la factory
	public StringGenerator getSG()
	{
		return new StringGenerator(this.alphabet, this.length);
	}
	
	// ritorna un generatore di parole
	// di lunghezza length
	public StringGenerator getSG(int length) {
		return new StringGenerator(this.alphabet, length);
	}
	
	// ritorna un generatore di tutte le parole
	public AllStringGenerator getASG() {
		return new AllStringGenerator(this.alphabet);
	}
}
