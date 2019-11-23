package domain;

import model.Auxiliar;
import model.Metrica;

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Code {

    private static final String OPERATING_SYSTEM = System.getProperty("os.name").toLowerCase();
    private static final String line = ".*(\\S)";
    private static final String regexFuncao2 = "(static|void|int|List|int|Integer|double|Double|String|string|char|long|Long|boolean|float).([A-z]|main).[(].([A-z]|void|.)\\n";
    private static final String regexFuncao = "\\w+.*\\(";

    private static int linhasCodigo = 0;
    private static int contadorFuncao = 0;
    private static int contadorFuncaoDeusa = 0;
    private static int contadorStruct = 0;
    
    private static boolean funcao = false;

    private static boolean first = true;

    private static boolean possivelEstruturaCodigoFonte = false;
    
    private static boolean verificacaoRegular = true;

    public Metrica executarAnalise(File arq, String caminho) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(caminho));
        String linhaAnterior = "";
        while (br.ready()) {
        	
            String linha = br.readLine();
//            verificacaoComentario(linha);
            verificarLinha(linha);
            
            if(linha.contains("if(!(dc = DC_LockDc(hDC)))")) {
            	System.out.println("");
            }
            
            if(funcao && linha.contains("{")) {
            	System.out.println("                        adicionado: " + linha);
            	contadorFuncao++;
            	funcao = false;
            }
            if(linha.contains(";")) {
            	funcao = false;
            }
//            if(verificacaoRegular) {
				if((!funcao || first) && !possivelEstruturaCodigoFonte) {
					first = false;
	            	funcao = verificarFuncoes(linha);
	            	if(funcao) {
	            		System.out.println(linha);
	            	}
            	}
				if(linha.contains(")")) {
					possivelEstruturaCodigoFonte = false;
				}
//            }
        }
        first = true;
        br.close();

//        if(verificacaoRegular) {
            contadorFuncaoDeusa = verificarExistenciasDeEntidadesDeuses(arq, 127, true);
//        }

        imprimirMetricas();

        Integer mesArquivoT;
        String arquivo ;

        mesArquivoT = capturarMesArquivoPeloCaminho(caminho);

        arquivo = caminho.substring(caminho.lastIndexOf(OPERATING_SYSTEM.equalsIgnoreCase("linux") ? "/" : "\\" ) + 1);

        Metrica metrica = new Metrica(mesArquivoT, arquivo, linhasCodigo, contadorFuncao, contadorFuncaoDeusa);

        linhasCodigo = contadorFuncao = contadorFuncaoDeusa = 0;

        return metrica;
    }

    private void verificacaoComentario(String linha) {
        if(linha.contains("/*")) {
            verificacaoRegular = false;
        } else if(linha.contains("*/")) {
            verificacaoRegular = true;
        }
    }

    private Integer capturarMesArquivoPeloCaminho(String caminhoInformado) {
        Integer mes = null;
        String[] caminho = caminhoInformado.split(OPERATING_SYSTEM.equalsIgnoreCase("linux") ? "/" : "\\\\");
        for (String string : caminho) {
            try {
                mes = Integer.parseInt(string);
            } catch (NumberFormatException e) {
                continue;
            }
        }
        return mes;
    }

    private void imprimirMetricas() {
        System.out.println("Metricas:\n"
                + "LOC: " + linhasCodigo + "\n"
                + "Numero de Fun��es: " + contadorFuncao + "\n"
                + "Numero de Fun��es Deusas: " + contadorFuncaoDeusa + "\n"
                + "Numero de Struct: " + contadorStruct + "\n");
    }

    private static void verificarLinha(String linha) {
        if (linha.matches(line)) {
            linhasCodigo++;
        }
    }

    private static boolean verificarFuncoes(String linha) {
        Pattern padrao = Pattern.compile(regexFuncao);
        Matcher encontrador = padrao.matcher(linha);
        while (encontrador.find()) {
//        	if(!possuiEstruturaDeCodigoFonte(linha, "for") && !possuiEstruturaDeCodigoFonte(linha, "if")
//        			&& !linha.contains(";") && !possuiEstruturaDeCodigoFonte(linha, "else")) {
//            	return true;
//        	}
        	if(!possuiEstruturaDeCodigoFonte(linha)) {
        		return true;
        	}
        }
        return false;
    }
    
    private static boolean possuiEstruturaDeCodigoFonte(String linhaCodigo) {
//    	Pattern padrao = Pattern.compile("(if |for |;)");
//        Matcher encontrador = padrao.matcher(linhaCodigo);
//        while (encontrador.find()) {
//        	if(!linhaCodigo.contains(")")) {
//            	possivelEstruturaCodigoFonte = true;
//        	}
//        	return true;
//        }
//        return false;
    	if(!linhaCodigo.contains(")")) {
        	possivelEstruturaCodigoFonte = true;
    	}
    	return Pattern.compile("(if\\(|if |for\\(|for |;)").matcher(linhaCodigo).find();
//    	return linhaCodigo.contains(validacao+"(") || linhaCodigo.contains(validacao+" (") || linhaCodigo.contains(validacao);
    }

    private static int verificarExistenciasDeEntidadesDeuses(File arquivo, int limite, boolean metodo) {
        try {
            int contGod = 0;
            List<Auxiliar> contadores = new ArrayList<Auxiliar>();

            FileReader arqv = new FileReader(arquivo);
            BufferedReader leitura = new BufferedReader(arqv);

            String line = leitura.readLine();
            while (line != null) {
                if (!contadores.isEmpty() || contadores != null) {
                    for (Auxiliar contAuxiliar : contadores) {
                        if (contAuxiliar.getContador() != 0) {
                            contAuxiliar.setLinha(contAuxiliar.getLinha() + 1);
                        }
                    }
                }
                if (line.contains("{") && (!contadores.isEmpty() || contadores != null)) {
                    for (Auxiliar contAuxiliar : contadores) {
                        if (contAuxiliar.getContador() != 0) {
                            contAuxiliar.setContador(contAuxiliar.getContador() + 1);
                        }
                    }
                }
                if (line.contains("}")) {

                    if (!contadores.isEmpty() || contadores != null) {
                        for (Auxiliar contAuxiliar : contadores) {
                            if (contAuxiliar.getContador() != 0) {
                                contAuxiliar.setContador(contAuxiliar.getContador() - 1);
                            }
                        }
                    }
                }
                if (!contadores.isEmpty() || contadores != null) {
                    for (Auxiliar contAuxiliar : contadores) {
                        if (contAuxiliar.getContador() == 0) {
                            if (contAuxiliar.getLinha() > limite) {
                                contGod++;
                                contAuxiliar.setLinha(0);
                            } else {
                                contAuxiliar.setLinha(0);
                            }
                        }
                    }
                }
                line = leitura.readLine();
            }
            return contGod;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        return 0;
    }
}
