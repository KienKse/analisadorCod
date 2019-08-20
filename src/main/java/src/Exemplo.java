package src.main.java.src;

import java.math.BigDecimal;
import java.util.ArrayList;

public class Exemplo {

	private static String regexMethod = "(public static void|private static void|protected static void).*";

	public static void main(String[] args) {
		System.out.println("Ola, Mundo!");
		System.out.println("xprotected class {private String x;} private class {private String x;}");
		System.out.println(" protected class {private String x;} private class {private String x;}");

		/** metodo2 e metodo4 => contexto static */
		metodo2("teste1");
		metodo4(null, true, 1234567890L);

		/** metodo 1 e 3 => nonstatic*/
	}

	public static void metodo4(String nome, boolean objects, Long number) {
		// Method Body
	}

	private void metodo3(int i, ArrayList<Object> objects) {
		// Method Body
	}

	protected static void metodo2(String teste1) {
		// Method Body
	}

	public BigDecimal metodo1() {
		// Method Body
		return null;
	}
}