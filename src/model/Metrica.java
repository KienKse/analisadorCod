package model;

public class Metrica {
	
	
	private String arquivo;
	private Integer pasta;
	private Integer loc;
	private Integer metodos;
	private Integer classes;
	private Integer metodoDeus;
	private Integer classeDeus;
	
	public Metrica(String arquivo, Integer pasta) {
		this.arquivo = arquivo;
		this.pasta = pasta;
	}
	
	// mesArquivoT, arquivo, linhasCodigoT, contadorClasseT, contadorMetodoT
	
	public Metrica(Integer pasta, String arquivo, int loc, int classes, int metodos, int classeDeus, int metodoDeus) {
		this.arquivo = arquivo;
		this.pasta = pasta;
		this.loc = loc;
		this.classes = classes;
		this.metodos = metodos;
		this.classeDeus = classeDeus;
		this.metodoDeus = metodoDeus;
	}
	
	public String getArquivo() {
		return arquivo;
	}
	public void setArquivo(String arquivo) {
		this.arquivo = arquivo;
	}
	public Integer getPasta() {
		return pasta;
	}
	public void setPasta(Integer pasta) {
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
