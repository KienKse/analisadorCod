package domain;

import java.io.*;
import java.util.regex.*;  

public class Code {
	
	private static String FILE_PATH = "C:\\Users\\100904037\\Desktop\\eclipse\\analisador_cod\\src\\main\\java\\src\\Example.java";

	private static Integer linesCode = 0;
	private static Integer classCount = 0;
	private static String regex = "/([(public)( )(class)])/g";
	private static String regex2 = "[public class]";
//	private static boolean classFound = false;
	
	public static void main(String[] args) {
		countLoc();
//		countClassesFileNumber();
	}

	private static void countLoc() {
		try {
			BufferedReader br = new BufferedReader(new FileReader(FILE_PATH));
			while (br.ready()) {
				linesCode++;
				String linha = br.readLine();
				System.out.println(linha);
				System.out.println(Pattern.matches(regex2, linha));
				if(Pattern.compile(regex).matcher(linha).matches()) {
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
