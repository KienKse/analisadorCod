package usage;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.ArrayList;
public class Exemplo {

	public static void main(String[] args) {
		System.out.println("Ola, Mundo!");
		
	    //pegarNomeArquivo//System.out.println(f.getAbsolutePath().substring(f.getAbsolutePath().lastIndexOf("\\")+1));
	    		
		/** metodo2 e metodo4 => contexto static */
		metodo2("teste1");
		metodo4(null, true, 1234567890L);
		
		/** metodo 1 e 3 => nonstatic*/
	}
	
	public static class OutraClasse {
		public static void main(String[] args) {
			System.out.println("xablau");
		}
	}
	
	public static void metodo4(String nome, boolean objects, Long number) {
		// Method Body
	}
	
	private void metodo3(int i, ArrayList<Object> objects) {
		// Method Body
	}
	
	protected static void metodo2(String teste1) {
		// Method Body
		System.out.println("public class");
	}
	
	public BigDecimal metodo1() {
		// Method Body
		return null;
	}
	
	public BigInteger metodo2() {
		// Method Body
		return null;
	}
}