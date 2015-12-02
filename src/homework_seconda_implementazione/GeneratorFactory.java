package homework_seconda_implementazione;

import java.util.Arrays;

public class GeneratorFactory
{
	private final byte[] alphabet;
	private final int length;

	public GeneratorFactory(byte[] alphabet, int length)
	{
		this.alphabet = Arrays.copyOf(alphabet, alphabet.length);
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
	public StringGenerator getSG(int length)
	{
		return new StringGenerator(this.alphabet, length);
	}

	// ritorna un generatore di tutte le parole
	public AllStringGenerator getASG()
	{
		return new AllStringGenerator(this.alphabet);
	}
}
