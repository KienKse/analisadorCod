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
		for (String string : caminho.split("\\\\")) {
	    	try {
	    		return String.valueOf(Integer.parseInt(string));
	    	} catch (NumberFormatException e) {
				continue;
			}
		};
		System.out.println("Nao foi possivel extrair o mes da pasta\n" + caminho);
		return null;
	}

	private void imprimirMetricas() {
		System.out.println("Metricas:\n"
				+ "LOC: " + linhasCodigo + "\n"
				+ "Numero de classes: " + contadorClasse  + "\n"
				+ "Numero de Metodos: " + contadorMetodo + "\n");
	}

	private static void verificarLinha(String linha) {
		if(linha.matches(line)) {
			linhasCodigo++;
		}
	}

	private static void veriricarMetodos(String linha) {
		Pattern pattern = Pattern.compile(regexMethod);
        Matcher matcher = pattern.matcher(linha);
		while (matcher.find()) {
			contadorMetodo++;
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
