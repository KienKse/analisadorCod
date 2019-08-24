package src.main.java.domain;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.swing.JOptionPane;

public class Code {
	private static String path = "";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static Integer methodCount = 0;
	private static String regexClass = "(public class|private class|protected class).*";
//	public static final String class2 = "(.*class)* [A-Z].* [{]";
	public static final String PATTERMETHOD = "(.(public|private|protected)* [A-Z].* [(].* [{].*";

	public static void main(String[] args) {
		try {
			path = JOptionPane.showInputDialog("Digite o caminho completo do path do arquivo:");
			metodoPrincipal();
		} catch (NullPointerException e) {
			String message = "Nullpointer -> Path de arquivo inválido";
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
			linesCode++;
			String linha = br.readLine();
			verificarClasses(linha);
			veriricarMetodos(linha);
		}
		br.close();
	}

	private static void verificarClasses(String linha) {
		if(linha.matches(regexClass)) {
			classCount++;
		}
	}

	private static void veriricarMetodos(String linha) {
		if(linha.matches(PATTERMETHOD)) {
			methodCount++;
		}
	}
}
