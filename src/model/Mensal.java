package model;

import java.util.ArrayList;
import java.util.List;

public class Mensal {
	
	private Integer mes;
	private List<Metrica> metricas;
	private Integer locTotal = 0;
	private Integer classesTotal = 0;
	private Integer metodosTotal = 0;
	private Integer classesDeusTotal = 0;
	private Integer metodosDeusTotal = 0;
	
	public Mensal(Integer mes) {
		this.mes = mes;
	}
	
	public Mensal() {
		// TODO Auto-generated constructor stub
	}

	public List<Metrica> getMetricas() {
		if(metricas == null) {
			metricas = new ArrayList<Metrica>();
		}
		return metricas;
	}
	
	public Integer getLocTotal() {
		for (Metrica metrica : metricas) {
			locTotal += metrica.getLoc();
		}
		return locTotal;
	}
	
	public Integer getClassesTotal() {
		for (Metrica metrica : metricas) {
			classesTotal += metrica.getClasses();
		}
		return classesTotal;
	}
	
	public Integer getMetodosTotal() {
		for (Metrica metrica : metricas) {
			metodosTotal += metrica.getMetodos();
		}
		return metodosTotal;
	}
	
	public Integer getClassesDeusTotal() {
		for (Metrica metrica : metricas) {
			classesDeusTotal += metrica.getClasseDeus();
		}
		return classesDeusTotal;
	}
	
	public Integer getMetodosDeusTotal() {
		for (Metrica metrica : metricas) {
			metodosDeusTotal += metrica.getMetodoDeus();
		}
		return metodosDeusTotal;
	}
	
	
	public Integer getMes() {
		return mes;
	}
	
	public void setMes(Integer mes) {
		this.mes = mes;
	}
	
	

}
