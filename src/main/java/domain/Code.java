package src.main.java.domain;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.swing.JOptionPane;

public class Code {
	
	private static String path = "C:\\Users\\kiens\\eclipse-workspace\\analisadorCod\\src\\main\\java\\src\\Exemplo.java";
//	private static String path = "";
	private static final String line = ".*(\\S)";
	//TODO: fix method
	private static final String regexMethod = "(public|private|protected).*(static|void|String|int|long|float|boolean|double|char|Bitmap|BigDecimal|Double|Long|Float).*(\\()*(\\{)";
	private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";
	
	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static Integer methodCount = 0;

	public static void main(String[] args) {
		try {
//			path = JOptionPane.showInputDialog("Digite o caminho completo do path do arquivo:");
			metodoPrincipal();
		} catch (NullPointerException e) {
			String message = "Nullpointer -> Path de arquivo invalido";
			JOptionPane.showMessageDialog(null,message);;
//			System.err.println(message);
		}
	}

	private static void metodoPrincipal() {
		try {
			executarAnalise();
			System.out.println("Linhas de codigo: " + linesCode);
			System.out.println("Classes: " + classCount);
			System.out.println("Metodos: " + methodCount);
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	private static void executarAnalise() throws FileNotFoundException, IOException {
		BufferedReader br = new BufferedReader(new FileReader(path));
		while (br.ready()) {
			String linha = br.readLine();
			verificarLinha(linha);
			verificarClasses(linha);
			veriricarMetodos(linha);
		}
		br.close();
	}
	
	private static void verificarLinha(String linha) {
		if(linha.matches(line)) {
			linesCode++;
		}
	}

	private static void verificarClasses(String linha) {
		if(linha.matches(regexClass)) {
			classCount++;
		}
	}

	private static void veriricarMetodos(String linha) {
		if(linha.matches(regexMethod)) {
			methodCount++;
		}
	}
}
