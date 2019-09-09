package domain;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
public class Code {
	
	private static final String line = ".*(\\S)";
	private static final String regexMethod = "(public|private|protected).*(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|BigInteger|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";

	private static Integer linhasCodigo = 0;
	private static Integer contadorClasse = 0;
	private static Integer contadorMetodo = 0;
	
	private static Integer chavesAbertas = 0;
	
	private static Integer contadorMetodoDeus = 0;
	private static Integer contadorClasseDeus = 0;

	public List<String> executarAnalise(String caminho) throws FileNotFoundException, IOException {
		BufferedReader br = new BufferedReader(new FileReader(caminho));
		while (br.ready()) {
			String linha = br.readLine();
			verificarLinha(linha);
			veriricarClasses(linha);
			veriricarMetodos(linha);
		}
		br.close();
		imprimirMetricas();
		
		String mesArquivoT, arquivo, linhasCodigoT, contadorClasseT, contadorMetodoT;
		
		linhasCodigoT = linhasCodigo+"";
		contadorClasseT = contadorClasse+"";
		contadorMetodoT = contadorMetodo+"";
		mesArquivoT = capturarMesArquivoPeloCaminho(caminho);
		arquivo = caminho.substring(caminho.lastIndexOf("\\")+1);
		
		
		linhasCodigo = contadorClasse = contadorMetodo = 0;
		
		return Arrays.asList(mesArquivoT, arquivo, linhasCodigoT, contadorClasseT, contadorMetodoT, "\n");
//		return Arrays.asList(mesArquivoT, linhasCodigoT, contadorClasseT, contadorMetodoT, "\n");
	}

	private String capturarMesArquivoPeloCaminho(String caminho) {
		String mes = null;
		for (String string : caminho.split("\\\\")) {
	    	try {
	    		mes = String.valueOf(Integer.parseInt(string));
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
		System.out.println("Chaves: " + chavesAbertas);
	}

	private static void verificarLinha(String linha) {
		if(linha.matches(line)) {
			verificarMetodoDeus(linha, "Metodo Deus", 127);
			//verificarClasseDeus(linha, "Classe Deus", 800);
			linhasCodigo++;
		}
	}

	private static void veriricarMetodos(String linha) {
		Pattern pattern = Pattern.compile(regexMethod);
        Matcher matcher = pattern.matcher(linha);
		if(matcher.find()) {
			contadorMetodo++;
		}
	}
	
	
	private static void verificarMetodoDeus(String linhaCodigo, String vericacao, int limite) {
		//TODO CONTINUAR METODO DEUS E APOS CLASSE VERIFY
		chavesAbertas++;
		contadorMetodo++;
		//todo: FIX count error
		if(linhaCodigo.matches(".*(\\{)")) {
			if(contadorMetodo >= limite) {
				contadorMetodoDeus++;
				contadorMetodo = 0;
			}
			
			if(linhaCodigo.matches("(\\})")) {
				chavesAbertas--;
			}
		}
		
		if(linhaCodigo.matches("(\\})")) {
			chavesAbertas--;
		}
	}

	private static void veriricarClasses(String linha) {
        Pattern pattern = Pattern.compile(regexClass);
        Matcher matcher = pattern.matcher(linha);
        
        while (matcher.find()) {
        	contadorClasse++;
        }	
	}

	
}
