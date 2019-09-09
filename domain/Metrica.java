package domain;

import java.util.Arrays;

public class Metrica {
	
	
	private String arquivo;
	private String pasta;
	private Integer loc;
	private Integer metodos;
	private Integer classes;
	private Integer metodoDeus;
	private Integer classeDeus;
	
	public Metrica(String arquivo, String pasta) {
		this.arquivo = arquivo;
		this.pasta = pasta;
	}
	
	// mesArquivoT, arquivo, linhasCodigoT, contadorClasseT, contadorMetodoT
	
	public Metrica(String pasta, String arquivo, int loc, int classes, int metodos) {
		this.arquivo = arquivo;
		this.pasta = pasta;
		this.loc = loc;
		this.classes = classes;
		this.metodos = metodos;
	}
	
	public String getArquivo() {
		return arquivo;
	}
	public void setArquivo(String arquivo) {
		this.arquivo = arquivo;
	}
	public String getPasta() {
		return pasta;
	}
	public void setPasta(String pasta) {
		this.pasta = pasta;
	}
	public Integer getLoc() {
		return loc;
	}
	public void setLoc(Integer loc) {
		this.loc = loc;
	}
	public Integer getMetodos() {
		return metodos;
	}
	public void setMetodos(Integer metodos) {
		this.metodos = metodos;
	}
	public Integer getClasses() {
		return classes;
	}
	public void setClasses(Integer classes) {
		this.classes = classes;
	}
	public Integer getMetodoDeus() {
		return metodoDeus;
	}
	public void setMetodoDeus(Integer metodoDeus) {
		this.metodoDeus = metodoDeus;
	}
	public Integer getClasseDeus() {
		return classeDeus;
	}
	public void setClasseDeus(Integer classeDeus) {
		this.classeDeus = classeDeus;
	}
	
	
	
}
