package src.main.java.domain;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import src.main.java.src.Exemplo;

public class Code {
	
	private static final String FILE_PATH = "";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static String regexClass = "(public class|private class|protected class).*";

	public static void main(String[] args) {
		count();
	}

	private static void count() {
		try {
			BufferedReader br = new BufferedReader(new FileReader(FILE_PATH));
			while (br.ready()) {
				linesCode++;
				String linha = br.readLine();
				if(linha.matches(regexClass)) {
					classCount++;
				}
			}
			br.close();
			System.out.println("Linhas de codigo: " + linesCode);
			System.out.println("Classes: " + classCount);
			contarMetodos();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	private static void contarMetodos() {
		Class<Exemplo> classe = Exemplo.class;
		System.out.println("Metodos: " + classe.getDeclaredMethods().length);
	}

}
