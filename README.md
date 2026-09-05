<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/logo-light.svg">
  <source media="(prefers-color-scheme: light)" srcset="assets/logo-dark.svg">
  <img alt="Logo do projeto" src="assets/logo-dark.svg" width="180">
</picture>

# Honda Fan 125 (2018) with Titan Blackout Tachometer

Sistema de tacômetro experimental baseado em Arduino Nano para adicionar leitura de RPM a uma Honda Fan 125 2018 utilizando um painel Titan 2023 Blackout aftermarket.

O projeto funciona como um conversor de sinal:

    Sinal de ignição da motocicleta
                ↓
          Arduino Nano
                ↓
          Cálculo de RPM
                ↓
       Tabela de calibração
                ↓
       Frequência específica
                ↓
       Entrada RPM do painel
                ↓
          Indicador de RPM


## Objetivo

A Honda Fan 125 2018 utilizada neste projeto não possui um tacômetro original.

O objetivo é utilizar o sinal proveniente do sistema de ignição para calcular a rotação do motor e gerar um sinal compatível com a entrada de RPM de um painel Titan 2023 Blackout aftermarket.

O painel não apresentou uma relação linear simples entre frequência de entrada e RPM indicado. Por isso, o projeto utiliza uma tabela de calibração experimental.

O Arduino não tenta reproduzir matematicamente o funcionamento interno do painel.

Em vez disso, ele responde:

> "Quando o motor estiver nesta faixa de RPM, qual frequência faz este painel indicar corretamente essa faixa?"

Essa abordagem transforma o painel em uma caixa-preta que pode ser calibrada empiricamente.


## Hardware

### Motocicleta

- Honda Fan 125 2018

### Painel

- Titan 2023 Blackout aftermarket

### Microcontrolador

- Arduino Nano
- ATmega328P

### Componentes utilizados

- Arduino Nano
- Resistor de 10 kΩ
- Fios de cobre
- Multímetro

A instalação definitiva deve utilizar conexões soldadas e devidamente isoladas.


## Pinagem do Arduino

| Arduino Nano | Função |
|---|---|
| D2 | Entrada do sinal da motocicleta |
| D9 | Saída de frequência para o painel |
| GND | Terra comum |

O D2 utiliza interrupção externa para detectar as transições do sinal.

O D9 utiliza o Timer1 do ATmega328P para gerar a frequência de saída.


## Entrada do sinal da motocicleta

O sinal utilizado neste projeto é o fio azul/amarelo associado ao pulso de ignição da motocicleta.

O sinal é conectado ao D2 através de um resistor de 10 kΩ.

Esquema simplificado:

    Sinal azul/amarelo
           |
          10 kΩ
           |
           +-------- D2 Arduino Nano
           
    GND da motocicleta
           |
           +-------- GND Arduino Nano


## Saída para o painel

O D9 é conectado à entrada de RPM do painel.

    Arduino D9
        |
        +-------- Entrada RPM do painel

O GND do Arduino e o GND do painel devem possuir referência comum.


## Como o RPM é calculado

Durante os testes foi observado que o Arduino detecta aproximadamente 9,2 eventos de pulso por volta do motor.

O código utiliza:

    PULSOS_POR_VOLTA = 9.2

A cada intervalo de medição, o Arduino calcula a quantidade de pulsos por segundo e converte para RPM:

    RPM = (pulsos_por_segundo × 60) / pulsos_por_volta

A leitura é posteriormente filtrada para reduzir oscilações.


## Por que existe uma tabela de calibração?

O comportamento do painel não é linear.

Durante os testes, algumas frequências produziram aproximadamente:

| Frequência | RPM indicado pelo painel |
|---:|---:|
| 8,0 Hz | ~2800 RPM |
| 8,2 Hz | ~4000 RPM |
| 8,4 Hz | ~6800 RPM |
| 8,6 Hz | ~11000+ RPM |
| 8,8 Hz | ~1800 RPM |
| 9,0 Hz | ~2300 RPM |
| 9,1 Hz | ~2500 RPM |
| 9,2 Hz | ~2800 RPM |
| 9,3 Hz | ~3000 RPM |
| 9,4 Hz | ~3500 RPM |
| 9,5 Hz | ~4200 RPM |
| 9,6 Hz | ~5000 RPM |
| 9,7 Hz | ~6000 RPM |
| 9,8 Hz | ~7800 RPM |
| 9,9 Hz | ~10500 RPM |
| 10,0 Hz | ~11000+ RPM |
| 25 Hz | ~4000 RPM |

Esses resultados demonstram que não é seguro assumir uma fórmula simples do tipo:

    frequência = RPM / constante

Portanto, o projeto utiliza uma tabela de calibração.


## Tabela de calibração

A tabela possui 44 posições.

Cada posição representa uma faixa de 250 RPM:

    [00] 250–499 RPM
    [01] 500–749 RPM
    [02] 750–999 RPM
    ...
    [43] 11000+ RPM

No código:

    const float frequencias[44] = {
        ...
    };

Cada posição contém a frequência que deve ser enviada ao painel naquela faixa.


## Como calibrar

A calibração é feita com o motor desligado.

O Arduino pode ser utilizado para enviar uma frequência fixa ao painel.

Para cada frequência testada, deve-se observar o RPM indicado pelo painel.

Por exemplo:

    8,20 Hz → painel indica aproximadamente 4000 RPM

Se o objetivo for fazer a faixa:

    4000–4249 RPM

ser indicada como 4000 RPM pelo painel, a posição correspondente na tabela deve receber:

    8,20

Assim, quando o Arduino detectar um RPM entre 4000 e 4249, ele enviará 8,20 Hz.

O processo é repetido para cada faixa.


## Estrutura da tabela

A tabela segue esta lógica:

    RPM detectado
          ↓
    (RPM - 250) / 250
          ↓
    índice da tabela
          ↓
    frequência correspondente
          ↓
    painel


Por exemplo:

    4000 RPM
        ↓
    índice 15
        ↓
    frequencias[15]
        ↓
    frequência configurada para 4000–4249 RPM


## Alterando a calibração

A única parte que normalmente precisa ser alterada durante a calibração é:

    const float frequencias[44]

Os valores podem ser completamente não lineares.

Por exemplo:

    const float frequencias[44] = {
        8.20,
        8.20,
        9.01,
        8.74,
        9.03,
        8.91,
        9.14,
        ...
    };

Não existe necessidade de os valores aumentarem de maneira uniforme.

O objetivo da tabela é reproduzir empiricamente a resposta do painel.


## Auditoria

O projeto foi desenvolvido para que a calibração possa ser auditada diretamente no código.

Cada entrada da tabela possui um comentário indicando sua faixa de RPM:

    8.200,   // [15] 4000 - 4249

Dessa maneira é possível verificar visualmente:

1. Qual faixa de RPM está sendo considerada.
2. Qual índice da tabela representa essa faixa.
3. Qual frequência será enviada ao painel.
4. Alterar individualmente qualquer frequência sem modificar a lógica do programa.


## Monitor Serial

O Arduino envia informações pelo Serial Monitor a 115200 baud.

Exemplo:

    RPM REAL: 4032 | QUADRADO: 15 | Hz ENVIADO: 8.200

Isso permite verificar simultaneamente:

- RPM calculado pelo Arduino;
- índice da faixa utilizada;
- frequência enviada ao painel.


## Procedimento recomendado de calibração

Para cada faixa:

1. Determinar a frequência que faz o painel indicar o RPM desejado.
2. Deixar o painel estabilizar.
3. Repetir o teste para confirmar que a leitura não é apenas um pico momentâneo.
4. Registrar a frequência encontrada.
5. Colocar o valor na posição correspondente da tabela.
6. Testar novamente com o motor funcionando.
7. Ajustar somente aquela posição se necessário.

Recomenda-se registrar os resultados externamente durante os testes antes de alterar a tabela definitiva.


## Exemplo

Suponha que os testes produzam:

    8,17 Hz → 3000 RPM
    8,21 Hz → 3250 RPM
    8,26 Hz → 3500 RPM
    8,34 Hz → 3750 RPM
    8,20 Hz → 4000 RPM

A tabela pode conter:

    8.17,   // 3000
    8.21,   // 3250
    8.26,   // 3500
    8.34,   // 3750
    8.20,   // 4000

A ordem não precisa ser crescente.

Se 4000 RPM exigir uma frequência menor que 3750 RPM, isso é aceitável. O painel é tratado como uma caixa-preta e a tabela reproduz seu comportamento observado.


## Compilação

O projeto foi desenvolvido para Arduino Nano baseado no ATmega328P.

No Arduino IDE:

    Board:
    Arduino Nano

    Processor:
    ATmega328P

Caso seja utilizado um Nano com outro bootloader, selecione a opção correspondente no Arduino IDE.


## Instalação

1. Conecte o sinal da motocicleta ao D2 através do resistor de 10 kΩ.
2. Conecte o GND da motocicleta ao GND do Arduino.
3. Conecte o D9 à entrada de RPM do painel.
4. Faça upload do firmware.
5. Abra o Serial Monitor em 115200 baud.
6. Verifique se o Arduino está detectando RPM.
7. Verifique a frequência que está sendo enviada.
8. Faça a calibração da tabela.
9. Após a calibração, faça os testes com o motor funcionando.


## Segurança

Este projeto trabalha diretamente com sinais elétricos de uma motocicleta.

O circuito apresentado é experimental e não deve ser considerado uma interface automotiva profissional.

O sinal de entrada deve ser devidamente condicionado antes de uma instalação permanente caso sejam observadas tensões ou transientes fora das especificações do ATmega328P.

Nunca conecte diretamente ao Arduino um sinal que possa exceder os limites elétricos do microcontrolador.

Todas as conexões devem ser isoladas e mecanicamente protegidas contra vibração, umidade e curto-circuito.


## Limitações conhecidas

O projeto depende do comportamento específico do painel Titan 2023 Blackout utilizado nos testes.

Outro painel pode utilizar uma entrada de RPM diferente e exigir outra calibração.

A tabela de frequências também é específica para a combinação:

    Honda Fan 125 2018
    +
    Titan 2023 Blackout
    +
    Arduino Nano


Portanto, os valores da tabela não devem ser considerados uma especificação universal.


## Estado do projeto

O firmware está funcional como protótipo, mas não utilizável na prática.

A arquitetura de leitura, cálculo de RPM, seleção da faixa e geração do sinal está implementada.

A tabela de frequência é deliberadamente editável e deve ser calibrada experimentalmente para obter a melhor correspondência possível entre o RPM real do motor e a indicação do painel.


## Licença

This project is licensed under the MIT License.

You are free to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software, subject to the conditions of the license.

See the [LICENSE](LICENSE) file for the complete license text.
