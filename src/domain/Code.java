package domain;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import model.Metrica;
public class Code {

	private static final String OPERATING_SYSTEM = System.getProperty("os.name").toLowerCase();


	private static final String line = ".*(\\S)";
	private static final String regexMethodBkp = "(public|private|protected).*(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|BigInteger|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexMethod = "(public|private|protected).*(?!.*class|[A-Z].*).(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|BigInteger|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";

	private static int linhasCodigo = 0;
	private static int contadorClasse = 0;
	private static int contadorClasseDeus = 0;
	private static int contadorMetodo = 0;
	private static int contadorMetodoDeus = 0;

	private static int chavesAbertasMetodo = 0;
	private static int chavesAbertasClasse = 0;

	private static int auxiliarClasse = 0;
	private static int auxiliarMetodo = 0;


	public Metrica executarAnalise(String caminho) throws IOException {
		BufferedReader br = new BufferedReader(new FileReader(caminho));
		while (br.ready()) {
			String linha = br.readLine();
			verificarLinha(linha);
		}
		br.close();
		imprimirMetricas();
		
		Integer mesArquivoT;
		String arquivo = null;
		
		mesArquivoT = capturarMesArquivoPeloCaminho(caminho);

		if(OPERATING_SYSTEM.equalsIgnoreCase("linux")) {
			arquivo = caminho.substring(caminho.lastIndexOf("/") + 1);
		} else {
			arquivo = caminho.substring(caminho.lastIndexOf("\\")+1);
		}
			
		Metrica metrica = new Metrica(mesArquivoT, arquivo, linhasCodigo, contadorClasse, contadorMetodo, contadorClasseDeus, contadorMetodoDeus);

		linhasCodigo = contadorClasse = contadorMetodo = contadorClasseDeus = contadorMetodoDeus = 0;
		
		return metrica;
	}

	private Integer capturarMesArquivoPeloCaminho(String caminho) {
		Integer mes = null;
		String[] path = null;
		if(OPERATING_SYSTEM.equalsIgnoreCase("linux")) {
			path = caminho.split("/");
		} else {
			path = caminho.split("\\\\");
		}
		for (String string : path) {
	    	try {
	    		mes = Integer.parseInt(string);
	    	} catch (NumberFormatException e) {
				continue;
			}
		};
		return mes;
	}

	private void imprimirMetricas() {
		System.out.println("Metricas:\n"
				+ "LOC: " + linhasCodigo + "\n"
				+ "Numero de Classes: " + contadorClasse + "\n"
				+ "Numero de Metodos: " + contadorMetodo + "\n"
				+ "Numero de Metodos Deus: " + contadorMetodoDeus + "\n"
				+ "Numero de Classes Deus: " + contadorClasseDeus + "\n");
	}

	private static void verificarLinha(String linha) {
		if(linha.matches(line)) {
			verificarMetodoDeus(linha, 127);
			verificarClasseDeus(linha, 800);
			linhasCodigo++;
		}
	}

	private static boolean veriricarMetodos(String linha) {
		boolean metodo = false;
		Pattern pattern = Pattern.compile(regexMethod);
    	Matcher matcher = pattern.matcher(linha);
		while (matcher.find()) {
			metodo = true;
			contadorMetodo++;
		}
		return metodo;
	}

	private static boolean veriricarClasses(String linha) {
		boolean classe = false;
		Pattern pattern = Pattern.compile(regexClass);
		Matcher matcher = pattern.matcher(linha);

		while (matcher.find()) {
			classe = true;
			contadorClasse++;
		}
		return classe;
	}


	private static void verificarMetodoDeus (String linhaCodigo, int limite) {
		boolean linhaDeChamadaDoMetodo = veriricarMetodos(linhaCodigo);
		if(linhaDeChamadaDoMetodo) {
			chavesAbertasMetodo++;
		}
		if (chavesAbertasMetodo > 0) {
			if(!linhaDeChamadaDoMetodo) {
				if (linhaCodigo.matches("(.*\\})")) {
					chavesAbertasMetodo--;
				} else {
					auxiliarMetodo++;
					if (linhaCodigo.matches("(.*\\{)")) {
						chavesAbertasMetodo++;
					}
				}
				if (chavesAbertasMetodo != 0) {
					if (auxiliarMetodo == limite) {
						contadorMetodoDeus++;
					}
				} else {
					auxiliarMetodo = chavesAbertasMetodo = 0;
				}
			}
		} else {
			auxiliarMetodo = 0;
		}
	}

	private static void verificarClasseDeus (String linhaCodigo, int limite) {
		boolean linhaDeChamadaDeClasse = veriricarClasses(linhaCodigo);
		if(linhaDeChamadaDeClasse) {
			chavesAbertasClasse++;
		}
		if (chavesAbertasClasse > 0 && !linhaDeChamadaDeClasse) {
			if (linhaCodigo.matches("(.*\\})")) {
				chavesAbertasClasse--;
			} else {
				if (linhaCodigo.matches("(.*\\{)")) {
					chavesAbertasClasse++;
				} else {
					auxiliarClasse++;
				}
			}
			if (auxiliarClasse == limite) {
				contadorClasseDeus++;
			}
		} else {
			auxiliarClasse = 0;
		}
	}
	
}
