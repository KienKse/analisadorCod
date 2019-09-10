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
	
	private static final String line = ".*(\\S)";
	private static final String regexMethod = "(public|private|protected).*(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|BigInteger|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";

	private static Integer linhasCodigo = 0;
	private static Integer contadorClasse = 0;
	private static Integer contadorMetodo = 0;
	
	private static Integer chavesAbertas = 0;
	private static Integer chavesAbertasClasse = 0;
	
	private static Integer contadorMetodoDeus = 0;
	private static Integer contadorClasseDeus = 0;

	public Metrica executarAnalise(String caminho) throws FileNotFoundException, IOException {
		BufferedReader br = new BufferedReader(new FileReader(caminho));
		while (br.ready()) {
			String linha = br.readLine();
			verificarLinha(linha);
//			veriricarClasses(linha);
//			veriricarMetodos(linha);
		}
		br.close();
		imprimirMetricas();
		
		Integer mesArquivoT;
		String arquivo;
		
		mesArquivoT = capturarMesArquivoPeloCaminho(caminho);
		arquivo = caminho.substring(caminho.lastIndexOf("\\")+1);
//		arquivo = caminho.substring(caminho.lastIndexOf("/")+1);
			
		Metrica metrica = new Metrica(mesArquivoT, arquivo, linhasCodigo, contadorClasse, contadorMetodo, contadorClasseDeus, contadorMetodoDeus);

		linhasCodigo = contadorClasse = contadorMetodo = contadorClasseDeus = contadorMetodoDeus = 0;
		
		return metrica;
		//return Arrays.asList(mesArquivoT, arquivo, linhasCodigoT, contadorClasseT, contadorMetodoT, "\n");
//		return Arrays.asList(mesArquivoT, linhasCodigoT, contadorClasseT, contadorMetodoT, "\n");
	}

	private Integer capturarMesArquivoPeloCaminho(String caminho) {
		Integer mes = null;
		for (String string : caminho.split("\\\\")) {
//		for (String string : caminho.split("/")) {
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
		if(veriricarMetodos(linhaCodigo)) {
			if(linhaCodigo.matches(".*(\\{)")) {
				chavesAbertas++;
				if(contadorMetodo >= limite) {
					contadorMetodoDeus++;
				}
			}
		} else if(linhaCodigo.matches("(\\})")) {
			chavesAbertas--;
		}
	}
	
	private static void verificarClasseDeus (String linhaCodigo, int limite) {
		if(veriricarClasses(linhaCodigo)) {
			if(linhaCodigo.matches(".*(\\{)")) {
				chavesAbertasClasse++;
				if(contadorClasse >= limite) {
					contadorClasseDeus++;
				}
			}
			
			
		} else if(linhaCodigo.matches("(\\})")) {
			chavesAbertas--;
		}
	}
	
}
