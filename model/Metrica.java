package model;

public class Metrica {

	private String arquivo;
	private Integer pasta;
	private Integer loc;
	private Integer funcoes;
	private Integer funcoesDeusas;
	
	public Metrica(String arquivo, Integer pasta) {
		this.arquivo = arquivo;
		this.pasta = pasta;
	}
	
	public Metrica(Integer pasta, String arquivo, int loc, int funcoes, int funcoesDeusas) {
		this.arquivo = arquivo;
		this.pasta = pasta;
		this.loc = loc;
		this.funcoes = funcoes;
		this.funcoesDeusas = funcoesDeusas;
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
		return funcoes;
	}
	public void setMetodos(Integer metodos) {
		this.funcoes = metodos;
	}
	
	public Integer getMetodoDeus() {
		return funcoesDeusas;
	}
	public void setMetodoDeus(Integer metodoDeus) {
		this.funcoesDeusas = metodoDeus;
	}

	public Integer getFuncoes() {
		return funcoes;
	}

	public void setFuncoes(Integer funcoes) {
		this.funcoes = funcoes;
	}

	public Integer getFuncoesDeusas() {
		return funcoesDeusas;
	}

	public void setFuncoesDeusas(Integer funcoesDeusas) {
		this.funcoesDeusas = funcoesDeusas;
	}
	
	
	
	
}
