SOFTWARE PARA SIMULAÇÂO DO MONITORAMENTO DOS SINAIS VITAIS

-> Introdução ao projeto

  Este é um software didático voltado para a área da saúde com o objetivo de possibilitar o cadastro e simulação de quadros clínicos de arritmia em um paciente fictício.

  Após realizado o download ou clone deste repositório, a execução deste projeto pode ser realizada simplesmente através da execução do arquivo ”Projeto.exe” localizado na pasta ”src”.

  O programa conta atualmente com os seguintes atalhos de teclado:

- F7: essa tecla alterna entre os modos tela e modo janela. Útil para reposicionar janelas na presença de 2 ou mais monitores no modo estender. É importante observar que a alternância entre os modos de exibição do programa através dessa tecla é possível somente após a tela Formulário.

-> Edição do código fonte e adição de novos quadros

  O código fonte do projeto encontra-se no arquivo ”Projeto.cbp” dentro da pasta ”src”. É altamente recomendável que a edição deste projeto seja realizada na IDE Codeblocks.

-> Adição de novos quadros clínicos ao programa
  
  Para adicionar novos quadros é necessário adicionar um novo arquivo .txt contendo os pontos correspondentes do gráfico na pasta ”db”. Em seguida deve-se adicionar também o som que deseja-se executar juntamente com o quadro na pasta ”audios”. Por fim, necessita-se a realização de algumas alterações no código fonte como adicionar a expressão dos valores dos sinais no quadro e opção no menu popup do painel de controle.
