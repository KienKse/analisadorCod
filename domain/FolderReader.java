package domain;

import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

import model.Mensal;
import model.Metrica;

public class FolderReader {

	private static List<Metrica> dadosAnalisados = new ArrayList<Metrica>();
	private static List<Mensal> mensais = new ArrayList<Mensal>();

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
			    File pasta = jFileChooser.getSelectedFile();
			    FolderReader.listarPastas(new File(pasta.getPath()));
//			    ordenarMetricasPeloMes(dadosAnalisados);
//			    gerarMetricaGeral();
//			    gerarCSV();
//			    gerarCSVCompleto();
			}
		} catch (NullPointerException e) {
//			JOptionPane.showMessageDialog(null, "Arquivo nao encontrado");
			e.printStackTrace();
		}
	}

private static void gerarMetricaGeral() {
	Integer mes = 0;
	Mensal mensal = new Mensal();
	for (Metrica metrica : dadosAnalisados) {
		if(mes == metrica.getPasta()) {
			mensal.setMes(metrica.getPasta());
			mensal.getMetricas().add(metrica);
		} else {
			mes = metrica.getPasta();
			mensal = new Mensal(mes);
			mensais.add(mensal);
			mensal.getMetricas().add(metrica);
		}
	}
}



	private static void listarPastas(File dir) {
		File[] diretorios = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File caminho) {
				return caminho.isDirectory();
			}
		});

		System.out.println("Diretorio:\n" + dir.getAbsolutePath());
		System.out.println("______________________________________");
		listFile(dir);

		for (File pasta : diretorios) {
			System.out.println(pasta.getName());
			if(!pasta.getName().contains(".git") && !pasta.getName().contains(".idea")) {
				listarPastas(pasta);
			}
		}
	}

	private static void listFile(File dir) {
		Code code = new Code();
		File[] diretorios = dir.listFiles();
		for (File arquivo : diretorios) {
			if (arquivo.getName().contains(".c")) {
				System.out.println(arquivo.getName());
				try {
					dadosAnalisados.add(code.executarAnalise(arquivo, arquivo.getAbsolutePath()));
				} catch (FileNotFoundException e) {
					System.out.println("Nao e possivel acessar a pasta por falta de privilegios.");
//					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	private static void ordenarMetricasPeloMes(List<Metrica> metricas) {
		Collections.sort(metricas, Comparator.comparing(Metrica::getPasta));
	}
	
	private static void gerarCSV() {
		FileWriter escritor;
		try {
			/** MES,ARQUIVO,LOC,CLASSES,METODOS */
			escritor = extrairEscritor();
			
			for (Metrica metrica: dadosAnalisados) {
				List<String> dados = new ArrayList<String>(Arrays.asList(
						metrica.getPasta()+"",
						metrica.getArquivo(),
						metrica.getLoc()+"","",
						metrica.getMetodos()+"",
						"\n"
				));
				escritor.append(String.join(",", dados));
			}
			escritor.flush();
			escritor.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private static void gerarCSVCompleto() {
		FileWriter escritor;
		try {
			/** Mes;LOC;CLASSES;METODOS;CLASSE DEUS;METODO DEUS */
			escritor = extrairEscritorCompleto();
			
			for (Mensal mensal: mensais) {
				List<String> data = new ArrayList<String>(Arrays.asList(
					mensal.getMes()+"",
					mensal.getLocTotal()+"",
					mensal.getMetodosTotal()+"",
					mensal.getMetodosDeusTotal()+"",
					"\n"
				));
				escritor.append(String.join(",", data));
			}
			escritor.flush();
			escritor.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	

	private static FileWriter extrairEscritorCompleto() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisador.csv");
		escritor.append("MES,LOC,CLASSES,METDOS,CLASSE DEUS,METODO DEUS");
		escritor.append("\n");
		return escritor;
	}
	
	private static FileWriter extrairEscritor() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisdorCodComNome.csv");
		escritor.append("MES,ARQUIVO,LOC,CLASSES,METODOS");
		escritor.append("\n");
		return escritor;
	}
}
