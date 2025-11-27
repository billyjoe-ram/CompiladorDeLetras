# Documentacao da Aplicacao de Analise de Cadeias

## Visao Geral

Esta aplicacao em C++ realiza a analise completa de cadeias pertencentes a uma linguagem ficticia. Ela executa tres etapas principais: analise lexica, analise sintatica e analise semantica. A aplicacao valida a palavra de entrada, identifica as unidades sintaticas (como silabas) e pode gerar uma expressao semantica baseada nos caracteres da palavra.

## Como Usar

1. Execute o programa compilado.
2. Digite uma palavra composta apenas por letras de A a Z. A palavra sera automaticamente convertida para minusculas.
3. O programa exibira:

   * Tokens lexicos: cada caractere anotado como vogal ou consoante.
   * Unidades sintaticas: blocos classificados como V, C, VC, CV, CCV, VVV ou VCCV.
   * Se a palavra e valida segundo as regras.
4. Comandos disponiveis:

   * **0**: encerra o programa.
   * **1**: executa a analise semantica sobre a ultima palavra valida fornecida.

## Regras de Validacao

A cadeia deve obedecer:

* Apenas caracteres alfabeticos.
* Maximo de 3 vogais consecutivas.
* Deve terminar em uma vogal.
* Sequencias de consoantes sao validadas conforme restricoes especificas (por exemplo, grupos intermediarios devem respeitar a combinacao com vogais nas extremidades).

## Funcionamento da Aplicacao

### 1. Analise Lexica

A funcao `tokenizar` percorre a cadeia verificando:

* Se todos os caracteres sao alfabeticos.
* Identifica cada caractere como vogal ou consoante.
* Gera um vetor de `Simbolo`, contendo caractere, tipo (V/C) e posicao.

### 2. Validacao Sintatica Geral

A funcao `validar_cadeia_globais` aplica as regras da linguagem:

* Verifica limite de vogais consecutivas.
* Garante que a cadeia termina em vogal.
* Valida sequencias de consoantes.

### 3. Parser e Segmentacao por Unidades

A classe `Parser` utiliza programacao dinamica para encontrar a melhor segmentacao da cadeia, baseando-se em pesos predefinidos das estruturas sintaticas como:

* VCCV
* VVV
* CCV
* CV
* VC
* C
* V
  A segmentacao escolhida maximiza a soma dos pesos.

### 4. Reconstrucao da AST

O metodo `reconstruir_ast` retorna uma lista de unidades sintaticas (silabas) baseadas na segmentacao calculada.

### 5. Analise Semantica

A analise semantica converte cada unidade sintatica em uma expressao numerica baseada nos valores ASCII dos caracteres:

* Vogais possuem valor positivo.
* Consoantes possuem valor negativo.
* Estruturas como CCV, VVV e VCCV geram expressoes agrupadas.
  A expressao final do tipo `x = ...` representa a combinacao de todas as unidades.

## Fluxo de Execucao

1. O usuario digita uma cadeia.
2. O sistema valida lexica e sintaticamente.
3. Se valida, exibe a analise e salva a palavra.
4. Caso o usuario pressione **1**, a analise semantica e exibida.

## Exemplo Simplificado

Entrada: `agua`

* Tokens: a(V), g(C), u(V), a(V)
* Unidades: a / gu / a
* Semantica: x = +97 -103 +117 +97

Este documento resume o funcionamento interno e o modo de uso da aplicacao.
