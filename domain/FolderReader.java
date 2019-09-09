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
			    ordenarMetricasPeloMes(dadosAnalisados);
			    gerarMetricaGeral();
			    gerarCSV();
			    gerarCSVCompleto();
			}
		} catch (NullPointerException e) {
			JOptionPane.showMessageDialog(null, "Arquivo nao encontrado");
		}
	}

private static void gerarMetricaGeral() {
	Integer mes = 1;
	Mensal mensal = new Mensal();
	for (Metrica metrica : dadosAnalisados) {
		if(mes == metrica.getPasta()) {
			mensal.setMes(metrica.getPasta());
			mensal.getMetricas().add(metrica);
		} else {
			mes++;
			mensais.add(mensal);
			mensal = new Mensal();
			mensal.getMetricas().add(metrica);
		}
		if((dadosAnalisados.size()-1) == dadosAnalisados.indexOf(metrica)) {
			mensal.setMes(metrica.getPasta());
			mensais.add(mensal);
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
	
	private static void ordenarMetricasPeloMes(List<Metrica> metricas) {
		Collections.sort(metricas, new Comparator<Metrica>() {
		@Override
		public int compare(Metrica objeto1, Metrica objeto2) {
			return objeto1.getPasta().compareTo(objeto2.getPasta());
		}
		});
	}
	
	private static void gerarCSV() {
		FileWriter escritor;
		try {
			escritor = extrairEscritor();
	
//			List<String> data = new ArrayList<String>();
//			for (Metrica metrica: dadosAnalisados) {
//				data.add(metrica.getPasta()+"");
//				data.add(metrica.getArquivo());
//				data.add(metrica.getLoc()+"");
//				data.add(metrica.getClasses()+"");
//				data.add(metrica.getMetodos()+"");
//			}
			
			for (Metrica metrica: dadosAnalisados) {
				List<String> dados = new ArrayList<String>(Arrays.asList(
						metrica.getPasta()+"",
						metrica.getArquivo(),
						metrica.getLoc()+"",
						metrica.getClasses()+"",
						metrica.getMetodos()+"",
						"\n"
				));
				escritor.append(String.join(";", dados));
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
//			Mes;LOC;CLASSES;METODOS;CLASSE DEUS;METODO DEUS
			escritor = extrairEscritorCompleto();

			for (Mensal mensal: mensais) {
				List<String> data = new ArrayList<String>(Arrays.asList(
					mensal.getMes()+"",
					mensal.getLocTotal()+"",
					mensal.getClassesTotal()+"",
					mensal.getMetodosTotal()+"",
					mensal.getClassesDeusTotal()+"",
					mensal.getMetodosDeusTotal()+"",
					"\n"
				));
				escritor.append(String.join(";", data));
			}
			escritor.flush();
			escritor.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	

	private static FileWriter extrairEscritorCompleto() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisdorCodCompleto.csv");
		escritor.append("Mes;LOC;CLASSES;CLASSES;CLASSE DEUS;METODO DEUS");
		escritor.append("\n");
		return escritor;
	}
	
	private static FileWriter extrairEscritor() throws IOException {
		FileWriter escritor;
		escritor = new FileWriter("analisdorCodComNome.csv");
		escritor.append("Mes");
		escritor.append(";");
		escritor.append("Arquivo");
		escritor.append(";");
		escritor.append("LOC");
		escritor.append(";");
		escritor.append("CLASSES");
		escritor.append(";");
		escritor.append("METODOS");
		escritor.append("\n");
		return escritor;
	}
}
