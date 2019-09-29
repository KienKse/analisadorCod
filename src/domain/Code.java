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
    private static final String regexMethod = ".*(public|private|protected|void| Bitmap)(.*\\) \\{|throws)";
    private static final String regexClass = "(public|private|protected).*(class).*(\\()*(\\{)";

    private static int linhasCodigo = 0;
    private static int contadorClasse = 0;
    private static int contadorClasseDeus = 0;
    private static int contadorMetodo = 0;
    private static int contadorMetodoDeus = 0;

    private static boolean verificacaoRegular = true;

    public Metrica executarAnalise(File arq, String caminho) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(caminho));
        while (br.ready()) {
            String linha = br.readLine();
            verificacaoComentario(linha);
            verificarLinha(linha);
            if(verificacaoRegular) {
                veriricarMetodos(linha);
                veriricarClasses(linha);
            }
        }
        br.close();

        if(verificacaoRegular) {
            contadorMetodoDeus = verificarExistenciasDeEntidadesDeuses(arq, 127, true);
            contadorClasseDeus = verificarExistenciasDeEntidadesDeuses(arq, 800, false);
        }

        imprimirMetricas();

        Integer mesArquivoT;
        String arquivo ;

        mesArquivoT = capturarMesArquivoPeloCaminho(caminho);

        arquivo = caminho.substring(caminho.lastIndexOf(OPERATING_SYSTEM.equalsIgnoreCase("linux") ? "/" : "\\" ) + 1);

        Metrica metrica = new Metrica(mesArquivoT, arquivo, linhasCodigo, contadorClasse, contadorMetodo, contadorClasseDeus, contadorMetodoDeus);

        linhasCodigo = contadorClasse = contadorMetodo = contadorClasseDeus = contadorMetodoDeus = 0;

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
                + "Numero de Classes: " + contadorClasse + "\n"
                + "Numero de Metodos: " + contadorMetodo + "\n"
                + "Numero de Metodos Deus: " + contadorMetodoDeus + "\n"
                + "Numero de Classes Deus: " + contadorClasseDeus + "\n");
    }

    private static void verificarLinha(String linha) {
        if (linha.matches(line)) {
            linhasCodigo++;
        }
    }

    private static void veriricarMetodos(String linha) {
        Pattern padrao = Pattern.compile(regexMethod);
        Matcher encontrador = padrao.matcher(linha);
        while (encontrador.find()) {
            contadorMetodo++;
        }
    }

    private static void veriricarClasses(String linha) {
        Pattern padrao = Pattern.compile(regexClass);
        Matcher encontrador = padrao.matcher(linha);

        while (encontrador.find()) {
            contadorClasse++;
        }
    }

    private static int verificarExistenciasDeEntidadesDeuses(File arquivo, int limite, boolean metodo) {
        try {
            int contGod = 0;
            List<Auxiliar> contadores = new ArrayList<Auxiliar>();
            Auxiliar auxiliar;

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
                if (line.contains("{")) {
                    if (!contadores.isEmpty() || contadores != null) {
                        for (Auxiliar contAuxiliar : contadores) {
                            if (contAuxiliar.getContador() != 0) {
                                contAuxiliar.setContador(contAuxiliar.getContador() + 1);
                            }
                        }
                    }
                    if (line.matches(metodo == true ? regexMethod : regexClass)) {
                        auxiliar = new Auxiliar();
                        auxiliar.setContador(auxiliar.getContador() + 1);
                        contadores.add(auxiliar);


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
