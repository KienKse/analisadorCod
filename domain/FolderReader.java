package domain;

import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

public class FolderReader {
	
	private static List<List<String>> dadosAnalisados = new ArrayList<List<String>>();

//	private static String pathname = "C:\\Users\\100904037\\Desktop\\Dataset\\1";

	public static void main(String[] args) {
		try {
			new FolderReader();
			JFileChooser jFileChooser = new JFileChooser();
			jFileChooser.setCurrentDirectory(new java.io.File("."));
			jFileChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
			int returnVal = jFileChooser.showOpenDialog(jFileChooser);
			if(returnVal == JFileChooser.APPROVE_OPTION) {
//				pathname = JOptionPane.showInputDialog("Digite o caminho completo da pasta:");
//			    FolderReader.listFolders(new File(pathname));
			    File yourFolder = jFileChooser.getSelectedFile();
			    FolderReader.listarPastas(new File(yourFolder.getPath()));
			    gerarCSV();
			}
		} catch (NullPointerException e) {
			JOptionPane.showMessageDialog(null, "Arquivo nao encontrado");
		}
	}

//	private static void imprimir(List<List<String>> lista) {
//		System.out.println("________________________________");
//		lista.forEach(item->item.forEach(atributo->System.out.print("|"+atributo)));
//	}

	private static void listarPastas(File dir) {
		File[] diretorios = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File caminho) {
				return caminho.isDirectory();
			}
		});

		System.out.println("Direterio:\n" + dir.getAbsolutePath());
		System.out.println("______________________________________");
		listFile(dir);

		for (File pasta : diretorios) {
			if(!pasta.getName().contains(".git") && !pasta.getName().contains(".idea")) {
				listarPastas(pasta);
			}
		}
	}

	private static void listFile(File dir) {
		Code code = new Code();
		File[] diretorios = dir.listFiles();
		for (File arquivo : diretorios) {
			if (arquivo.getName().contains(".java")) {
				System.out.println(arquivo.getName());
				try {
//					code.executarAnalise(file.getAbsolutePath().replaceAll("\\\\", "\\\\\\\\"));
					dadosAnalisados.add(code.executarAnalise(arquivo.getAbsolutePath()));
				} catch (FileNotFoundException e) {
					System.out.println("Nao e possivel acessar a pasta por falta de privilegios.");
//					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	private static void gerarCSV() {
		FileWriter escritor;
		try {
			escritor = extrairEscritor();
	
			for (List<String> dados : dadosAnalisados) {
			    escritor.append(String.join("|", dados));
			}
	
			escritor.flush();
			escritor.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/** UTILIZAR COMPLETO FUTURO

	private static FileWriter extrairEscritorCompleto() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisdorCod.csv");
		escritor.append("Mes");
		escritor.append("|");
		escritor.append("LOC");
		escritor.append("|");
		escritor.append("CLASSES");
		escritor.append("|");
		escritor.append("METODOS");
		escritor.append("|");
		escritor.append("CLASSE DEUS");
		escritor.append("|");
		escritor.append("METODO DEUS");
		escritor.append("\n");
		return escritor;
	}
	
	*/
	
	private static FileWriter extrairEscritor() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisdorCodComNome.csv");
		escritor.append("Mes");
		escritor.append("|");
		escritor.append("Arquivo");
		escritor.append("|");
		escritor.append("LOC");
		escritor.append("|");
		escritor.append("CLASSES");
		escritor.append("|");
		escritor.append("METODOS");
		escritor.append("\n");
		return escritor;
	}
}
