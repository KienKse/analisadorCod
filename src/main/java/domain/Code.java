package src.main.java.domain;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.JOptionPane;

public class Code {
	
	private static final String line = ".*(\\S)";
	private static final String regexMethod = "(public|private|protected).*(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|BigInteger|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static Integer methodCount = 0;

	public void executarAnalise(String path) throws FileNotFoundException, IOException {
		BufferedReader br = new BufferedReader(new FileReader(path));
		while (br.ready()) {
			String linha = br.readLine();
			verificarLinha(linha);
			veriricarClasses(linha);
			veriricarMetodos(linha);
		}
		br.close();
		imprimirMetricas();
	}
	
	private void imprimirMetricas() {
		System.out.println("Métricas:\n"
				+ "LOC: " + linesCode + "\n"
				+ "Número de classes: " + classCount  + "\n"
				+ "Múmero de Métodos: " + methodCount + "\n");
		
		linesCode = 0;
		classCount = 0;
		methodCount = 0;
	}

	private static void verificarLinha(String linha) {
		if(linha.matches(line)) {
			linesCode++;
		}
	}

	private static void veriricarMetodos(String linha) {
		Pattern pattern = Pattern.compile(regexMethod);
        Matcher matcher = pattern.matcher(linha);
		if(matcher.find()) {
			methodCount++;
		}
	}
	
	
	private static void veriricarClasses(String linha) {
        Pattern pattern = Pattern.compile(regexClass);
        Matcher matcher = pattern.matcher(linha);
        
        while (matcher.find()) {
        	classCount++;
        }
		
		/**
			List<List<String>> dadosAnalisados = Arrays.asList(
			//exampleList
			    Arrays.asList("Jean", "author", "Java"),
			    Arrays.asList("David", "editor", "Python"),
			    Arrays.asList("Scott", "editor", "Node.js")
			    //adicionar dados da analise das pastas
			);

			FileWriter writer = new FileWriter("analisdorCod.csv");
			writer.append("MÊS");
			writer.append("|");
			writer.append("LOC");
			writer.append("|");
			writer.append("CLASSES");
			writer.append("|");
			writer.append("MÉTODOS");
			writer.append("|");
			writer.append("CLASSE DEUS");
			writer.append("|");
			writer.append("MÉTODO DEUS");
			writer.append("\n");

			for (List<String> rowData : dadosAnalisados) {
			    writer.append(String.join("|", rowData));
			    writer.append("\n");
			}

			writer.flush();
			writer.close();
		*/
	}
}
