package src.main.java.domain;

import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

public class FolderReader {

//	private static String pathname = "";

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
			    FolderReader.listFolders(new File(yourFolder.getPath()));
			    
			}
		} catch (NullPointerException e) {
			JOptionPane.showMessageDialog(null, "Arquivo não encontrado");
		}
	}

	private static void listFolders(File dir) {
		File[] directories = dir.listFiles(new FileFilter() {

			@Override
			public boolean accept(File pathname) {
				return pathname.isDirectory();
			}
		});

		System.out.println("Diretório:\n" + dir.getAbsolutePath());
		System.out.println("______________________________________");
		listFile(dir);

		for (File folder : directories) {
			if(!folder.getName().contains(".git") && !folder.getName().contains(".idea")) {
				listFolders(folder);
			}
		}
	}

	private static void listFile(File dir) {
		Code code = new Code();
		File[] directories = dir.listFiles();
		for (File file : directories) {
			if (file.getName().contains(".java")) {
				System.out.println(file.getName());
				try {
//					code.executarAnalise(file.getAbsolutePath().replaceAll("\\\\", "\\\\\\\\"));
					code.executarAnalise(file.getAbsolutePath());
				} catch (FileNotFoundException e) {
					System.out.println("Não é possível acessar a pasta por falta de privilégios.");
//					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}
}
