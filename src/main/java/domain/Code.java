package domain;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Code {
	
	private static String FILE_PATH = "C:\\Users\\100904037\\Desktop\\analisadorCod\\src\\main\\java\\src\\Example.java";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static String regex = "(public class|private class|protected class).*";
	
	public static void main(String[] args) {
		count();
	}

	private static void count() {
		try {
			BufferedReader br = new BufferedReader(new FileReader(FILE_PATH));
			while (br.ready()) {
				linesCode++;
				String linha = br.readLine();
				System.out.println(linha);
				
				if(linha.matches(regex)) {
					classCount++;
				}
			}
			br.close();
			System.out.println("Linhas de código: " + linesCode);
			System.out.println("Classes: " + classCount);
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}
	
}
