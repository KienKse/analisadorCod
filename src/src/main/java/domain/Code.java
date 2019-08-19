package src.main.java.domain;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Code {
	
	private static final String FILE_PATH = "/home/icaro/Documentos/workspace/analisadorCod/src/src/main/java/src/Example.java";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static Integer methodCount = 0;
	private static String regexClass = "(public class|private class|protected class).*";
	private static String regexMethod = "(public static void|private static void|protected static void).*";

	public static void main(String[] args) {
		count();
	}

	private static void count() {
		try {
			BufferedReader br = new BufferedReader(new FileReader(FILE_PATH));
			while (br.ready()) {
				linesCode++;
				String linha = br.readLine();
//				System.out.println(linha);
				
				if(linha.matches(regexClass)) {
					classCount++;
				}
				if(linha.matches(regexMethod)) {
					methodCount++;
				}
			}
			br.close();
			System.out.println("Linhas de codigo: " + linesCode);
			System.out.println("Classes: " + classCount);
			System.out.println("Metodos: " + methodCount);
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

}
