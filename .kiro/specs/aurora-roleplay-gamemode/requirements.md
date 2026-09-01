# Documento de Requisitos — Aurora Roleplay Gamemode

## Introdução

O Aurora Roleplay (ARP) é uma gamemode de Roleplay para SA-MP desenvolvida em Pawn, representando uma reestruturação completa da gamemode GCRP. O objetivo é criar uma experiência de cidade viva e coerente, onde sistemas interdependentes substituem a coleção de funcionalidades isoladas.

A filosofia central é: **sistemas que conversem entre si**. A cadeia principal de progressão conecta jogador → emprego → economia → banco → veículo → empresa → cidade, garantindo que cada ação do jogador ressoe nos demais sistemas.

O servidor retrata uma cidade grande e moderna com regiões distintas (centro urbano, bairros residenciais, periferia, área comercial e zona rural), suportando múltiplos estilos de vida dentro do mesmo universo de RP.

Plugins utilizados: `streamer`, `sscanf`, `pawncmd`, `sampvoice`, `discord-connector`, `mapandreas`, `crashdetect`.

---

## Glossário

- **ARP**: Aurora Roleplay — nome do servidor e da gamemode.
- **Gamemode**: O programa principal que roda no servidor SA-MP, escrito em Pawn.
- **Sistema**: Módulo lógico da gamemode responsável por um conjunto de funcionalidades.
- **Jogador**: Usuário conectado ao servidor SA-MP, identificado por seu nome de personagem.
- **Personagem**: A identidade de roleplay do jogador, com atributos de RP (nome, idade, sexo, origem).
- **NPC**: Personagem não-jogador controlado pelo servidor.
- **Facção**: Organização oficial do servidor (polícia, SAMU, bombeiros, etc.) com hierarquia, rank e rádio próprio.
- **Organização_Criminal**: Grupo criminoso com território, estoque e estrutura econômica própria.
- **Organização**: Termo genérico que engloba Facção e Organização_Criminal.
- **Empresa**: Estabelecimento comercial com dono, estoque e caixa próprios.
- **Veículo_Pessoal**: Veículo registrado em nome de um Jogador, com dados persistidos.
- **Inventário**: Conjunto de itens portados pelo Jogador.
- **Celular**: Dispositivo virtual do Jogador com contatos, mensagens, chamadas e aplicativos.
- **Banco**: Sistema financeiro central que gerencia saldos, transferências e empréstimos.
- **Economia**: Conjunto de regras que governa fluxo de dinheiro, salários, impostos e preços.
- **VoIP**: Comunicação por voz via plugin SampVoice.
- **Chat_Local**: Canal de texto com alcance limitado por proximidade (raio de 15 metros).
- **Chat_Facção**: Canal de texto e voz exclusivo para membros da mesma Organização.
- **Ocorrência**: Registro de incidente policial com número, suspeitos e evidências.
- **Evidência**: Item ou dado coletado em cena de crime vinculado a uma Ocorrência.
- **Mandado**: Autorização judicial emitida pelo sistema policial contra um Jogador.
- **Ficha_Criminal**: Histórico de infrações e prisões de um Jogador.
- **Território**: Área do mapa controlada por uma Organização_Criminal.
- **Progressão**: Avanço do Jogador em reputação, nível, profissão e patrimônio.
- **Sistema_Login**: Módulo responsável por registro, autenticação e sessão do Jogador.
- **Sistema_Personagem**: Módulo responsável pelo perfil de RP do Jogador.
- **Sistema_Economia**: Módulo responsável por finanças, salários e impostos.
- **Sistema_Banco**: Módulo responsável por operações bancárias.
- **Sistema_Veículo**: Módulo responsável por veículos pessoais, garagem e concessionária.
- **Sistema_Empresa**: Módulo responsável por empresas e comércio.
- **Sistema_Casa**: Módulo responsável por imóveis residenciais.
- **Sistema_Organização**: Módulo responsável por facções e organizações criminosas.
- **Sistema_Policial**: Módulo responsável por ocorrências, mandados e investigação.
- **Sistema_Celular**: Módulo responsável pelo celular virtual integrado.
- **Sistema_Chat**: Módulo responsável por todos os canais de comunicação textual.
- **Sistema_VoIP**: Módulo responsável pela comunicação de voz via SampVoice.
- **Sistema_Inventário**: Módulo responsável por itens portados pelo Jogador.
- **Sistema_Progressão**: Módulo responsável por XP, nível, reputação e conquistas.
- **Sistema_Admin**: Módulo responsável por ferramentas de administração do servidor.
- **Sistema_Cidade**: Módulo responsável por transporte público, clima, NPCs e eventos urbanos.
- **Staff**: Administradores e moderadores do servidor.
- **VIP**: Jogador com benefícios premium adquiridos.
- **Pawn**: Linguagem de programação usada para escrever a gamemode SA-MP.
- **Dialog**: Janela de interface nativa do SA-MP exibida ao Jogador.
- **Pickup**: Ícone interativo no mundo 3D do SA-MP.
- **Label_3D**: Texto flutuante renderizado no mundo 3D do SA-MP.
- **Blip**: Marcador no radar/mapa do SA-MP.
- **MD5**: Algoritmo de hash usado para armazenar senhas.
- **ini**: Formato de arquivo de dados planos usado para persistência.

---

## Requisitos

---

### Requisito 1: Registro e Autenticação de Jogadores

**User Story:** Como jogador novo, quero me registrar com nome de personagem e senha, para que meus dados sejam salvos entre sessões.

#### Critérios de Aceitação

1. WHEN um Jogador se conecta ao servidor pela primeira vez, THE Sistema_Login SHALL exibir um Dialog de registro solicitando senha, confirmação de senha e e-mail.
2. WHEN um Jogador submete o formulário de registro com senha de 6 a 20 caracteres e e-mail válido, THE Sistema_Login SHALL criar o arquivo de dados do Jogador com senha hasheada em MD5 e exibir mensagem de confirmação.
3. IF um Jogador submete senha com menos de 6 ou mais de 20 caracteres, THEN THE Sistema_Login SHALL exibir mensagem de erro descritiva e reapresentar o Dialog de registro.
4. IF as senhas informadas no registro não coincidem, THEN THE Sistema_Login SHALL exibir mensagem de erro e reapresentar o Dialog de registro sem apagar o e-mail já digitado.
5. WHEN um Jogador retorna ao servidor com conta existente, THE Sistema_Login SHALL exibir um Dialog de login solicitando apenas a senha.
6. WHEN um Jogador informa a senha correta no login, THE Sistema_Login SHALL carregar todos os dados persistidos do Jogador e posicioná-lo no último local salvo.
7. IF um Jogador informa senha incorreta 5 vezes consecutivas, THEN THE Sistema_Login SHALL expulsar o Jogador do servidor com mensagem de aviso.
8. WHEN um Jogador completa o registro com sucesso, THE Sistema_Personagem SHALL iniciar o fluxo de criação de personagem antes de posicionar o Jogador no mundo.

---

### Requisito 2: Criação e Perfil de Personagem

**User Story:** Como jogador, quero definir os atributos de roleplay do meu personagem, para que eu tenha uma identidade única e verificável por outros jogadores.

#### Critérios de Aceitação

1. WHEN um Jogador completa o registro, THE Sistema_Personagem SHALL exibir Dialogs sequenciais para seleção de sexo (masculino/feminino), idade (18 a 70 anos) e origem (lista de cidades).
2. THE Sistema_Personagem SHALL armazenar nome, sexo, idade, origem, data de criação e último login de cada Jogador.
3. WHEN um Jogador usa o comando `/perfil` em outro Jogador próximo (raio de 5 metros), THE Sistema_Personagem SHALL exibir um Dialog com os dados públicos do personagem-alvo.
4. WHEN um Jogador usa o comando `/perfil` sem alvo, THE Sistema_Personagem SHALL exibir os dados do próprio personagem incluindo patrimônio total, facção atual e nível.
5. THE Sistema_Personagem SHALL permitir que o Jogador altere a skin do personagem via comando `/aparencia` em pontos de customização espalhados pelo mapa.
6. IF um Jogador tenta definir idade menor que 18 ou maior que 70, THEN THE Sistema_Personagem SHALL exibir mensagem de erro e reapresentar o Dialog de idade.

---

### Requisito 3: Progressão do Jogador

**User Story:** Como jogador, quero acumular experiência e reputação ao realizar ações de RP, para que meu progresso no servidor seja reconhecido e recompensado.

#### Critérios de Aceitação

1. WHEN um Jogador realiza ações de RP elegíveis (trabalho, interação com outros jogadores, conclusão de missões), THE Sistema_Progressão SHALL incrementar o XP do Jogador pelo valor definido para cada ação.
2. WHEN o XP do Jogador atinge o limiar do próximo nível, THE Sistema_Progressão SHALL incrementar o nível do Jogador e exibir notificação de level up com os benefícios desbloqueados.
3. THE Sistema_Progressão SHALL manter para cada Jogador: nível atual, XP acumulado, reputação, profissão ativa, total de horas jogadas, total de mortes e total de kills.
4. WHEN um Jogador muda de profissão, THE Sistema_Progressão SHALL registrar o histórico de profissões anteriores com data de início e fim.
5. THE Sistema_Progressão SHALL calcular o patrimônio total do Jogador como a soma de: dinheiro em mão, saldo bancário, valor dos veículos próprios e valor dos imóveis próprios.
6. WHEN um Jogador acumula conquistas específicas (primeiro veículo, primeiro imóvel, 100 horas jogadas), THE Sistema_Progressão SHALL registrar a conquista com data e exibi-la no perfil do Jogador.
7. WHEN um Jogador usa o comando `/stats`, THE Sistema_Progressão SHALL exibir um resumo de todas as estatísticas do personagem em um Dialog.

---

### Requisito 4: Sistema Econômico e Bancário

**User Story:** Como jogador, quero gerenciar meu dinheiro em mão e saldo bancário de forma separada, para que eu possa realizar transações financeiras realistas dentro do servidor.

#### Critérios de Aceitação

1. THE Sistema_Economia SHALL manter para cada Jogador dois saldos independentes: dinheiro em mão (máximo R$50.000) e saldo bancário (sem limite superior).
2. WHEN um Jogador usa caixa eletrônico (ATM) ou entra em agência bancária, THE Sistema_Banco SHALL exibir menu com opções de depósito, saque, transferência, extrato e saldo.
3. WHEN um Jogador deposita um valor válido no banco, THE Sistema_Banco SHALL deduzir o valor do dinheiro em mão e creditá-lo no saldo bancário na mesma operação atômica.
4. WHEN um Jogador saca um valor válido no banco, THE Sistema_Banco SHALL deduzir o valor do saldo bancário e creditá-lo no dinheiro em mão, respeitando o limite máximo de R$50.000 em mão.
5. IF um Jogador tenta sacar valor superior ao saldo bancário disponível, THEN THE Sistema_Banco SHALL recusar a operação e exibir o saldo atual disponível.
6. WHEN um Jogador transfere dinheiro para outro Jogador via celular ou ATM, THE Sistema_Banco SHALL deduzir o valor do remetente e creditá-lo no destinatário atomicamente, registrando ambas as operações no extrato.
7. THE Sistema_Banco SHALL manter extrato com as últimas 50 transações de cada Jogador contendo: tipo, valor, destinatário/origem e data/hora.
8. WHEN o Servidor inicia novo ciclo de pagamento (a cada hora de jogo), THE Sistema_Economia SHALL creditar automaticamente o salário base da profissão ativa de cada Jogador conectado no saldo bancário.
9. WHEN um Jogador paga imposto mensal (ciclo de 30 dias reais), THE Sistema_Economia SHALL deduzir automaticamente do saldo bancário a alíquota de imposto calculada sobre o patrimônio total do Jogador.
10. IF o saldo bancário do Jogador for insuficiente para cobrir o imposto, THEN THE Sistema_Economia SHALL deduzir o imposto do dinheiro em mão e notificar o Jogador.

---

### Requisito 5: Inventário

**User Story:** Como jogador, quero carregar itens no meu inventário com limite de peso, para que o gerenciamento de recursos seja físico e coerente com o roleplay.

#### Critérios de Aceitação

1. THE Sistema_Inventário SHALL limitar a capacidade de cada Jogador a 15 kg de peso total de itens carregados.
2. WHEN um Jogador coleta um item do mundo, THE Sistema_Inventário SHALL adicionar o item à lista de itens do Jogador e atualizar o peso total, impedindo a coleta se exceder a capacidade.
3. IF um Jogador tenta adicionar item que exceda a capacidade de 15 kg, THEN THE Sistema_Inventário SHALL recusar a coleta e exibir o peso atual e o peso do item.
4. WHEN um Jogador usa o comando `/inventario`, THE Sistema_Inventário SHALL exibir Dialog listando todos os itens com nome, quantidade e peso unitário, além do peso total atual.
5. WHEN um Jogador descarta um item do inventário, THE Sistema_Inventário SHALL remover o item da lista, depositar o pickup no chão na posição atual do Jogador e reduzir o peso total.
6. WHEN um Jogador usa um item consumível (comida, bebida, kit médico), THE Sistema_Inventário SHALL remover o item e acionar o efeito correspondente no Jogador.
7. THE Sistema_Inventário SHALL categorizar itens em: consumíveis, ferramentas, armas, munição, documentos e eletrônicos.
8. WHERE o Jogador possui VIP nível 2 ou superior, THE Sistema_Inventário SHALL aumentar o limite de capacidade para 25 kg.

---

### Requisito 6: Sistema de Saúde, Morte e Hospital

**User Story:** Como jogador, quero que o sistema de morte seja realista e ofereça chance de resgate, para que as consequências de combate incentivem o roleplay ao invés de penalizar apenas mecanicamente.

#### Critérios de Aceitação

1. WHEN a vida de um Jogador chega a zero, THE Sistema_Personagem SHALL colocá-lo em estado incapacitado no chão em vez de forçar respawn imediato.
2. WHILE um Jogador está em estado incapacitado, THE Sistema_Personagem SHALL exibir timer de 5 minutos para resgate e impedir o uso de qualquer comando exceto `/socorro`.
3. WHEN um paramédico usa o comando `/socorrer` em Jogador incapacitado no raio de 3 metros, THE Sistema_Personagem SHALL restaurar o Jogador com 50 HP, removê-lo do estado incapacitado e creditar XP ao paramédico.
4. WHEN o timer de resgate expira sem socorro, THE Sistema_Personagem SHALL teleportar o Jogador para o hospital mais próximo com 30 HP e registrar a morte nas estatísticas.
5. WHEN um Jogador respawna no hospital, THE Sistema_Economia SHALL deduzir do saldo bancário ou dinheiro em mão a taxa de internação hospitalar (R$500).
6. IF o Jogador não tiver saldo suficiente para a taxa hospitalar, THEN THE Sistema_Economia SHALL registrar a dívida hospitalar e bloquear acesso ao banco até quitação.
7. WHEN um Jogador coleta kit médico, THE Sistema_Personagem SHALL restaurar HP em 40 pontos, não ultrapassando o máximo de 100 HP.
8. THE Sistema_Personagem SHALL manter para cada Jogador o total de mortes e o total de kills registrados no perfil.

---

### Requisito 7: Fome e Sede

**User Story:** Como jogador, quero que meu personagem precise se alimentar e hidratar, para que a sobrevivência urbana seja parte do roleplay cotidiano.

#### Critérios de Aceitação

1. THE Sistema_Personagem SHALL manter para cada Jogador valores de fome (0–100) e sede (0–100), ambos iniciando em 100 ao fazer login.
2. WHEN o timer de fome executa (a cada 10 minutos de jogo), THE Sistema_Personagem SHALL reduzir fome em 5 pontos e sede em 7 pontos de cada Jogador conectado.
3. WHILE a fome do Jogador estiver abaixo de 20, THE Sistema_Personagem SHALL reduzir 2 HP por ciclo do timer e exibir aviso de fome crítica na tela.
4. WHILE a sede do Jogador estiver abaixo de 20, THE Sistema_Personagem SHALL reduzir 3 HP por ciclo do timer e exibir aviso de sede crítica na tela.
5. WHEN um Jogador consome item de comida do inventário, THE Sistema_Personagem SHALL aumentar a fome pelo valor nutritivo do item, não ultrapassando 100.
6. WHEN um Jogador consome item de bebida do inventário, THE Sistema_Personagem SHALL aumentar a sede pelo valor do item, não ultrapassando 100.
7. IF a fome ou sede do Jogador chegar a zero, THEN THE Sistema_Personagem SHALL iniciar perda de HP de 5 pontos por ciclo até que o Jogador consuma alimento ou bebida.

---

### Requisito 8: Sistema de Casas

**User Story:** Como jogador, quero comprar e gerenciar imóveis residenciais, para que meu personagem tenha um local de moradia com valor patrimonial real dentro do servidor.

#### Critérios de Aceitação

1. THE Sistema_Casa SHALL manter para cada casa: identificador único, dono (ou "sem dono"), preço de venda, estado de tranca, posição de entrada, posição interna e número de cômodos.
2. WHEN um Jogador sem casa própria usa o comando `/comprarCasa` próximo ao pickup de uma casa à venda, THE Sistema_Casa SHALL deduzir o valor da casa do saldo bancário do Jogador e registrá-lo como novo dono.
3. IF o saldo bancário do Jogador for inferior ao preço da casa, THEN THE Sistema_Casa SHALL recusar a compra e exibir o valor faltante.
4. WHEN o dono usa o comando `/venderCasa` dentro da própria casa, THE Sistema_Casa SHALL exibir Dialog para definir preço de venda e marcar a casa como disponível no mapa.
5. WHEN o dono usa o comando `/trancarcasa` ou `/destrancarCasa`, THE Sistema_Casa SHALL alternar o estado de tranca e atualizar o Label_3D na entrada.
6. WHEN um Jogador tenta entrar em casa trancada sem ser o dono, THE Sistema_Casa SHALL impedir a entrada e exibir mensagem informativa.
7. WHEN um Jogador entra em sua própria casa, THE Sistema_Personagem SHALL restaurar 10 HP por minuto enquanto o Jogador permanecer dentro, simulando descanso.
8. THE Sistema_Casa SHALL exibir Label_3D na entrada de cada casa com: número da casa, nome do dono (ou "À Venda") e preço (se disponível).
9. WHEN o dono usa o comando `/informacoes` dentro da casa, THE Sistema_Casa SHALL exibir Dialog com todos os dados da propriedade incluindo valor de mercado atualizado.

---

### Requisito 9: Sistema de Empresas

**User Story:** Como jogador empresário, quero administrar um estabelecimento comercial com funcionários, estoque e caixa, para que a economia do servidor tenha produtores e consumidores reais.

#### Critérios de Aceitação

1. THE Sistema_Empresa SHALL manter para cada empresa: identificador, nome, tipo (loja, restaurante, posto, oficina, etc.), dono, caixa próprio, estoque de produtos, lista de funcionários e histórico financeiro das últimas 100 transações.
2. WHEN o dono da empresa usa o comando `/empresa` dentro do estabelecimento, THE Sistema_Empresa SHALL exibir painel de gerenciamento com abas: caixa, estoque, funcionários, histórico e configurações.
3. WHEN o dono contrata um Jogador como funcionário via painel, THE Sistema_Empresa SHALL registrar o Jogador como funcionário com cargo e salário definidos.
4. WHEN o ciclo de pagamento de salários executa, THE Sistema_Economia SHALL debitar o caixa da empresa pelo total de salários dos funcionários e creditar no banco de cada funcionário conectado.
5. IF o caixa da empresa não cobrir o total de salários, THEN THE Sistema_Empresa SHALL notificar o dono e pagar salários parciais proporcionalmente ao saldo disponível.
6. WHEN um Jogador compra produto em uma empresa, THE Sistema_Empresa SHALL deduzir o item do estoque, creditar o valor no caixa da empresa e entregar o item no inventário do comprador.
7. WHEN o estoque de um produto cai abaixo de 10% da capacidade máxima, THE Sistema_Empresa SHALL notificar o dono via mensagem no celular.
8. THE Sistema_Empresa SHALL registrar no histórico financeiro cada entrada e saída do caixa com: tipo, valor, envolvido e data/hora.
9. WHEN o dono define preços de produtos via painel, THE Sistema_Empresa SHALL atualizar imediatamente os valores exibidos nos Dialogs de compra para clientes.
10. WHEN uma empresa fica sem dono (dono se desconecta permanentemente), THE Sistema_Empresa SHALL colocar o estabelecimento em leilão automático após 30 dias sem atividade do dono.

---

### Requisito 10: Sistema de Veículos

**User Story:** Como jogador, quero comprar, personalizar e manter meu veículo, para que a mobilidade urbana tenha custos e consequências reais no roleplay.

#### Critérios de Aceitação

1. THE Sistema_Veículo SHALL manter para cada Veículo_Pessoal: modelo, placa (única), proprietário, cor primária, cor secundária, combustível atual (0–100), quilometragem, estado de danos, estado de tranca e estado do motor.
2. WHEN um Jogador compra veículo na concessionária, THE Sistema_Veículo SHALL deduzir o valor do saldo bancário, gerar placa única, spawnar o veículo na posição do Jogador e registrar a compra no histórico do Jogador.
3. WHEN um Jogador usa o comando `/ligarCarro` dentro do próprio veículo, THE Sistema_Veículo SHALL ligar o motor se o combustível for superior a 0 e o veículo não estiver danificado criticamente.
4. WHEN o motor do veículo está ligado e o veículo se move, THE Sistema_Veículo SHALL consumir combustível na taxa de 1 unidade a cada 500 metros percorridos.
5. WHEN o combustível do veículo chega a zero, THE Sistema_Veículo SHALL desligar o motor automaticamente e impedir nova partida até abastecimento.
6. WHEN um Jogador abastece veículo em posto de gasolina, THE Sistema_Veículo SHALL deduzir do dinheiro em mão o valor calculado (preço por litro × litros adicionados) e atualizar o nível de combustível.
7. WHEN um Jogador usa o comando `/trancarveiculo`, THE Sistema_Veículo SHALL alternar o estado de tranca e exibir confirmação visual.
8. WHEN um veículo sofre colisão severa (velocidade de impacto superior a 80 km/h), THE Sistema_Veículo SHALL registrar danos críticos, reduzir desempenho e notificar o proprietário.
9. WHEN um Jogador usa o comando `/guardarVeiculo` em garagem própria ou pública, THE Sistema_Veículo SHALL despawnar o veículo do mapa e persistir seu estado para restauração posterior.
10. WHEN um Jogador usa o comando `/pegarVeiculo` em garagem, THE Sistema_Veículo SHALL exibir Dialog com lista dos veículos guardados e spawnar o selecionado na saída da garagem.
11. WHEN um Jogador usa o comando `/transferirVeiculo` para outro Jogador online, THE Sistema_Veículo SHALL atualizar o proprietário, registrar a transferência e notificar ambos os Jogadores.
12. WHEN um Jogador contrata seguro para o veículo via celular ou seguradora, THE Sistema_Veículo SHALL registrar o seguro com data de vencimento (30 dias) e deduzir o prêmio do saldo bancário.
13. IF um veículo segurado for destruído, THEN THE Sistema_Veículo SHALL creditar 80% do valor de mercado do veículo no saldo bancário do proprietário e remover o seguro.

---

### Requisito 11: Sistema de Celular

**User Story:** Como jogador, quero usar um celular virtual completo, para que a comunicação, serviços e informações estejam integrados em um único dispositivo de roleplay.

#### Critérios de Aceitação

1. WHEN um Jogador usa o comando `/celular` ou tecla de atalho, THE Sistema_Celular SHALL exibir Dialog com apps: Contatos, Mensagens, Ligações, GPS, Banco, Anúncios, Serviços e Rádio.
2. THE Sistema_Celular SHALL manter para cada Jogador uma lista de contatos com nome e número de telefone de até 50 entradas.
3. WHEN um Jogador faz chamada para número existente no servidor, THE Sistema_Celular SHALL exibir notificação de chamada recebida para o destinatário com opção de atender ou recusar.
4. WHILE uma chamada está ativa entre dois Jogadores, THE Sistema_Celular SHALL transmitir áudio via VoIP do plugin SampVoice em stream privado entre os dois.
5. WHEN uma chamada é encerrada (por qualquer das partes), THE Sistema_Celular SHALL fechar o stream VoIP privado e registrar a chamada no histórico com duração.
6. WHEN um Jogador envia mensagem de texto para outro, THE Sistema_Celular SHALL entregar a mensagem se o destinatário estiver online ou armazená-la para entrega no próximo login.
7. WHEN um Jogador usa o app Banco no celular, THE Sistema_Banco SHALL exibir saldo atual e permitir transferência bancária para contatos, com autenticação por PIN de 4 dígitos.
8. WHEN um Jogador usa o app GPS no celular, THE Sistema_Celular SHALL exibir Dialog com categorias de destino (hospital, banco, delegacia, postos, empresas) e criar checkpoint no mapa.
9. WHEN um Jogador publica anúncio via app Anúncios com custo de R$100, THE Sistema_Celular SHALL exibir o anúncio para todos os Jogadores conectados no formato de mensagem global e deduzir o valor.
10. WHEN um Jogador usa o app Serviços no celular, THE Sistema_Celular SHALL listar serviços disponíveis (táxi, mecânico, SAMU, pizza) e permitir chamada para o prestador mais próximo.
11. IF um Jogador tenta usar o celular enquanto está algemado ou incapacitado, THEN THE Sistema_Celular SHALL recusar a abertura e exibir mensagem de impedimento.

---

### Requisito 12: Sistema de Comunicação

**User Story:** Como jogador, quero múltiplos canais de comunicação integrados, para que o roleplay seja imersivo e cada tipo de comunicação tenha seu contexto próprio.

#### Critérios de Aceitação

1. THE Sistema_Chat SHALL limitar o Chat_Local ao raio de 15 metros, exibindo mensagens apenas para Jogadores dentro do alcance com degradê de opacidade por distância.
2. WHEN um Jogador usa o comando `/s` (grito), THE Sistema_Chat SHALL ampliar o alcance da mensagem para 40 metros e formatar a mensagem em maiúsculas.
3. WHEN um Jogador usa os comandos `/me` ou `/do`, THE Sistema_Chat SHALL formatar a mensagem como ação ou descrição de RP e limitá-la ao raio de 10 metros.
4. WHEN um Jogador usa o comando `/ame`, THE Sistema_Chat SHALL exibir a ação como Label_3D acima da cabeça do Jogador por 5 segundos visível a 20 metros.
5. WHEN um Jogador usa o comando `/f`, THE Sistema_Chat SHALL enviar a mensagem para todos os membros online da mesma Organização independentemente da distância.
6. THE Sistema_VoIP SHALL criar stream de voz local automático para cada Jogador conectado com alcance de 15 metros usando SampVoice.
7. WHEN um Jogador usa rádio de facção ativado, THE Sistema_VoIP SHALL transmitir a voz do Jogador para todos os membros da mesma Organização com sinal de estática.
8. WHEN um Jogador usa o comando `/r` (rádio texto), THE Sistema_Chat SHALL enviar a mensagem formatada em cor #FF9500 para todos os membros online da Organização.
9. THE Sistema_Chat SHALL filtrar palavras proibidas em todos os canais e registrar a tentativa no log de moderação.
10. WHEN um Jogador usa o comando `/g`, THE Sistema_Chat SHALL enviar mensagem IC global visível a todos os Jogadores conectados, limitado a 1 mensagem por minuto por Jogador.

---

### Requisito 13: Sistema de Organizações

**User Story:** Como jogador membro de organização, quero participar de uma estrutura hierárquica com recursos, comunicação e atividades próprias, para que o roleplay em grupo seja profundo e consequente.

#### Critérios de Aceitação

1. THE Sistema_Organização SHALL manter para cada Organização: nome, tag, tipo (facção oficial ou organização criminosa), líder, caixa próprio, até 10 ranks com nomes personalizáveis e lista de membros com rank individual.
2. WHEN o líder usa o comando `/convidar` em Jogador próximo, THE Sistema_Organização SHALL exibir Dialog de confirmação para o convidado e, se aceito, adicionar o Jogador à Organização no rank inicial.
3. WHEN o líder ou oficial usa o comando `/expulsar` informando nome do membro, THE Sistema_Organização SHALL remover o Jogador da Organização e notificá-lo.
4. WHEN um oficial usa `/promover` ou `/rebaixar` em membro de rank inferior ao próprio, THE Sistema_Organização SHALL alterar o rank do membro e registrar a mudança no log da Organização.
5. WHEN o líder usa o comando `/caixafaccao` para depositar dinheiro, THE Sistema_Organização SHALL deduzir o valor do dinheiro em mão do líder e creditá-lo no caixa da Organização.
6. THE Sistema_Organização SHALL exibir via comando `/membros` um Dialog listando todos os membros com: nome, rank, status (online/offline) e tempo de membro.
7. WHEN uma Organização_Criminal captura Território inimigo, THE Sistema_Organização SHALL atualizar o proprietário do Território, notificar ambas as Organizações e registrar o evento no histórico.
8. THE Sistema_Organização SHALL permitir que Organizações_Criminais gerenciem estoque de itens ilegais com rastreamento de entradas, saídas e responsáveis.
9. WHEN o líder da Organização_Criminal usa o comando `/operacao`, THE Sistema_Organização SHALL exibir painel de operações disponíveis (tráfico, roubo planejado, lavagem) com requisitos de membros e materiais.

---

### Requisito 14: Sistema Policial e Investigação

**User Story:** Como policial, quero ferramentas profundas de investigação e gestão de ocorrências, para que o combate ao crime seja uma atividade de roleplay estruturada e não apenas mecânica de /prender.

#### Critérios de Aceitação

1. WHEN um policial usa o comando `/ocorrencia novo` com descrição, THE Sistema_Policial SHALL criar uma Ocorrência com número único, timestamp, policial responsável e status "Em Andamento".
2. THE Sistema_Policial SHALL manter para cada Ocorrência: número, descrição, policial responsável, lista de suspeitos, lista de Evidências coletadas e status (em andamento, encerrada, arquivada).
3. WHEN um policial usa o comando `/evidencia coletar` em item marcado no chão, THE Sistema_Policial SHALL adicionar a Evidência à Ocorrência ativa do policial e remover o item do mundo.
4. WHEN um policial usa o comando `/algemar` em Jogador próximo (raio de 2 metros), THE Sistema_Policial SHALL aplicar algemas ao suspeito, impedindo uso de armas e corrida.
5. WHEN um policial usa o comando `/prender` em Jogador algemado com nível de procurado maior que zero, THE Sistema_Policial SHALL calcular o tempo de prisão (30 segundos por estrela de wanted), teleportar o suspeito para a prisão e registrar a detenção na Ficha_Criminal.
6. WHEN um policial usa o comando `/multar` em Jogador, THE Sistema_Policial SHALL exibir Dialog com lista de infrações e valores, deduzir o valor do dinheiro em mão do multado e registrar na Ficha_Criminal.
7. WHEN um policial usa o comando `/ficha` informando nome de Jogador, THE Sistema_Policial SHALL exibir Dialog com Ficha_Criminal contendo: total de prisões, total de multas, infrações registradas e Mandados ativos.
8. WHEN juiz (rank policial máximo) emite Mandado contra Jogador, THE Sistema_Policial SHALL registrar o Mandado com validade de 7 dias reais, tipo (prisão, busca) e motivo.
9. WHEN um policial usa o comando `/wanted` em Jogador, THE Sistema_Policial SHALL incrementar o nível de procurado em 1 estrela (máximo 6) e notificar o Jogador.
10. IF um Jogador com nível de procurado maior que zero permanecer 30 minutos sem ser preso, THEN THE Sistema_Policial SHALL reduzir o nível de procurado em 1 estrela automaticamente.
11. WHEN um policial usa o comando `/revista` em Jogador algemado, THE Sistema_Policial SHALL exibir o conteúdo do Inventário do suspeito e permitir apreensão de itens ilegais.

---

### Requisito 15: Sistema de Prisão e Hospital

**User Story:** Como jogador, quero que prisão e hospitalização tenham tempo definido e consequências reais, para que as punições sejam parte da narrativa e não apenas punições mecânicas arbitrárias.

#### Critérios de Aceitação

1. WHEN um Jogador é preso, THE Sistema_Policial SHALL teleportá-lo para a penitenciária, iniciar timer de prisão e bloquear uso de todos os comandos exceto `/celular` e `/me`.
2. THE Sistema_Policial SHALL exibir continuamente o tempo restante de prisão na tela do Jogador preso.
3. WHEN o timer de prisão atinge zero, THE Sistema_Policial SHALL teleportar o Jogador para frente da delegacia com equipamentos devolvidos e notificar sobre pena cumprida.
4. WHEN um Jogador preso usa `/celular` para contatar advogado (Jogador com profissão Advogado), THE Sistema_Policial SHALL permitir a chamada e possibilitar redução de 30% no tempo de prisão mediante pagamento de honorários.
5. IF um Jogador hospitalizado não tiver o valor da taxa de internação, THEN THE Sistema_Personagem SHALL registrar dívida no perfil e aplicar bloqueio parcial de funcionalidades bancárias até pagamento.

---

### Requisito 16: Sistema de Empregos e Profissões

**User Story:** Como jogador, quero exercer uma profissão com atividades próprias e salário, para que meu personagem tenha um papel econômico e social definido na cidade.

#### Critérios de Aceitação

1. THE Sistema_Progressão SHALL oferecer as seguintes profissões disponíveis: Policial (via facção), Paramédico (via facção SAMU), Bombeiro (via facção), Mecânico, Taxista, Motorista de Ônibus, Empresário, Advogado, Jornalista e Desempregado.
2. WHEN um Jogador seleciona profissão em centro de empregos, THE Sistema_Progressão SHALL registrar a profissão, definir o salário-base correspondente e exibir tutorial das atividades disponíveis.
3. WHEN um Mecânico usa o comando `/consertar` em veículo danificado próximo (raio de 5 metros), THE Sistema_Veículo SHALL reparar o veículo, THE Sistema_Progressão SHALL creditar XP ao Mecânico e THE Sistema_Economia SHALL cobrar taxa de serviço do proprietário do veículo.
4. WHEN um Taxista usa o comando `/corrida aceitar` para passageiro aguardando, THE Sistema_Progressão SHALL registrar a corrida ativa e calcular a tarifa por distância percorrida.
5. WHEN a corrida de táxi é concluída, THE Sistema_Economia SHALL transferir a tarifa do passageiro para o taxista automaticamente e creditar XP ao taxista.
6. WHEN um Motorista de Ônibus assume NPC de ônibus em terminal, THE Sistema_Cidade SHALL ativar a rota do ônibus com o Jogador como condutor e pagar passagem automática aos passageiros que embarcam.
7. WHEN um Jornalista usa o comando `/reportagem` com texto, THE Sistema_Chat SHALL publicar a reportagem como anúncio formatado para todos os Jogadores sem custo adicional além do tempo de cooldown de 10 minutos.

---

### Requisito 17: Sistema de Cidade Viva

**User Story:** Como jogador, quero uma cidade com transporte público, clima dinâmico e atividades urbanas, para que o ambiente de roleplay tenha vida própria independente dos jogadores online.

#### Critérios de Aceitação

1. THE Sistema_Cidade SHALL operar NPCs de ônibus em rotas gravadas com horários definidos, permitindo que Jogadores embarquem em pontos de parada e paguem passagem de R$5 deduzida do dinheiro em mão.
2. THE Sistema_Cidade SHALL atualizar o clima do servidor a cada 30 minutos de jogo com transições graduais entre: ensolarado, nublado, chuvoso e tempestade.
3. WHILE está em modo de tempestade, THE Sistema_Cidade SHALL reduzir a visibilidade, aumentar o consumo de combustível em 20% e exibir aviso climático para todos os Jogadores.
4. THE Sistema_Cidade SHALL manter hora do servidor sincronizada com progressão de 1 hora real = 1 dia de jogo, com mudanças automáticas de iluminação ambiente.
5. WHEN um Jogador entra em zona de comércio ativa, THE Sistema_Cidade SHALL exibir blips de estabelecimentos abertos no radar baseado no horário de funcionamento configurado.
6. THE Sistema_Cidade SHALL exibir mensagens automáticas temáticas a cada 15 minutos para todos os Jogadores com informações sobre eventos, dicas de RP e notícias do servidor.
7. WHEN uma Organização_Criminal controla Território por mais de 24 horas, THE Sistema_Cidade SHALL gerar evento de "incidente de segurança" naquela região visível no mapa com blip especial.
8. THE Sistema_Cidade SHALL manter NPCs estáticos em pontos-chave (banco, hospital, delegacia) que respondem ao comando `/falar` com informações sobre serviços locais.

---

### Requisito 18: Sistema de Administração

**User Story:** Como administrador do servidor, quero ferramentas completas de moderação e gestão, para que o servidor opere com segurança, justiça e rastreabilidade de todas as ações.

#### Critérios de Aceitação

1. THE Sistema_Admin SHALL definir 6 níveis de administrador (1 a 5 + Owner) com permissões progressivas, sendo nível 1 para moderação básica e nível 5/Owner para acesso total.
2. WHEN um administrador usa o comando `/ban` informando nome e motivo, THE Sistema_Admin SHALL banir o Jogador, registrar no log com: executor, alvo, motivo, IP banido e data/hora.
3. WHEN um administrador usa o comando `/kick` informando nome e motivo, THE Sistema_Admin SHALL expulsar o Jogador com exibição do motivo na tela e registrar no log.
4. WHEN um administrador usa `/mute` em Jogador com duração em minutos, THE Sistema_Admin SHALL silenciar o Jogador em todos os chats pelo tempo definido e notificar o Jogador.
5. WHEN um administrador usa `/goto` informando nome de Jogador, THE Sistema_Admin SHALL teleportar o administrador para a posição do Jogador-alvo.
6. WHEN um administrador usa `/spectate` em Jogador, THE Sistema_Admin SHALL entrar em modo espectador invisível seguindo o Jogador-alvo sem que o alvo perceba.
7. THE Sistema_Admin SHALL registrar em log persistente todas as ações administrativas com: nível do admin, nome do admin, ação executada, alvo, parâmetros e timestamp.
8. WHEN um Owner usa o comando `/setadmin` informando nome e nível, THE Sistema_Admin SHALL atualizar o nível administrativo do Jogador alvo e registrar a alteração.
9. THE Sistema_Admin SHALL exibir via `/admins` a lista de todos os administradores online com seus níveis.
10. WHEN um administrador usa `/announce` com texto, THE Sistema_Admin SHALL exibir a mensagem em destaque para todos os Jogadores conectados com formatação de aviso global.

---

### Requisito 19: Sistema VIP

**User Story:** Como jogador VIP, quero benefícios exclusivos que enriqueçam a experiência sem comprometer o equilíbrio do servidor, para que o suporte financeiro seja recompensado de forma justa.

#### Critérios de Aceitação

1. THE Sistema_Admin SHALL definir 3 níveis de VIP com benefícios progressivos: VIP Bronze (nível 1), VIP Prata (nível 2) e VIP Ouro (nível 3).
2. WHERE o Jogador possui VIP Bronze, THE Sistema_Personagem SHALL conceder: acesso a 5 skins exclusivas e redução de 10% na taxa hospitalar.
3. WHERE o Jogador possui VIP Prata, THE Sistema_Inventário SHALL aumentar capacidade de inventário para 25 kg e THE Sistema_Veículo SHALL conceder slot extra de veículo na garagem.
4. WHERE o Jogador possui VIP Ouro, THE Sistema_Banco SHALL isentar o Jogador de taxas bancárias e THE Sistema_Progressão SHALL aplicar multiplicador de 1,5x no XP ganho.
5. WHEN um administrador usa o comando `/setvip` informando nome e nível, THE Sistema_Admin SHALL atualizar o nível VIP do Jogador, registrar no log e notificar o Jogador.
6. WHEN o VIP de um Jogador expira (após 30 dias), THE Sistema_Admin SHALL notificar o Jogador 3 dias antes do vencimento e remover os benefícios automaticamente na expiração.

---

### Requisito 20: Persistência e Salvamento de Dados

**User Story:** Como jogador, quero que todos os meus dados sejam salvos de forma segura e automática, para que desconexões e reinicializações do servidor não resultem em perda de progresso.

#### Critérios de Aceitação

1. THE Sistema_Personagem SHALL salvar automaticamente os dados completos de cada Jogador em arquivo `.ini` individual a cada 5 minutos enquanto conectado.
2. WHEN um Jogador se desconecta por qualquer motivo, THE Sistema_Personagem SHALL executar salvamento imediato de todos os dados do Jogador antes de liberar os recursos.
3. WHEN o servidor é encerrado via `OnGameModeExit`, THE Sistema_Personagem SHALL salvar dados de todos os Jogadores conectados, veículos, casas, empresas e facções.
4. THE Sistema_Veículo SHALL salvar estado de cada Veículo_Pessoal (posição, combustível, km, danos) a cada 5 minutos e na destruição ou quando guardado.
5. THE Sistema_Casa SHALL salvar dados de cada casa sempre que houver alteração de dono, tranca ou preço.
6. THE Sistema_Empresa SHALL salvar estado de cada empresa sempre que houver transação no caixa, alteração de estoque ou mudança de funcionários.
7. THE Sistema_Organização SHALL salvar dados de cada Organização sempre que houver entrada ou saída de membro, alteração de rank ou movimentação no caixa.
8. IF ocorrer erro de escrita em arquivo durante salvamento, THEN THE Sistema_Personagem SHALL registrar o erro em log do servidor com nome do Jogador afetado e tentar novo salvamento em 30 segundos.

---

### Requisito 21: Integração entre Sistemas (Cadeia Econômica)

**User Story:** Como designer do servidor, quero que os sistemas estejam interligados em cadeia, para que cada ação do jogador ressoe nos demais sistemas e crie um ciclo econômico coerente.

#### Critérios de Aceitação

1. WHEN um Jogador recebe salário, THE Sistema_Economia SHALL automaticamente calcular e reservar o percentual de imposto correspondente ao nível patrimonial, tornando o ciclo econômico autossustentável.
2. WHEN um Jogador abastece veículo em posto de gasolina, THE Sistema_Economia SHALL creditar o valor pago no caixa da empresa do posto se o posto tiver dono registrado.
3. WHEN um Mecânico realiza serviço, THE Sistema_Economia SHALL transferir o valor do proprietário do veículo para o Mecânico e creditar XP na progressão do Mecânico simultaneamente.
4. WHEN uma Organização_Criminal controla Território com ponto de comércio ilegal, THE Sistema_Organização SHALL creditar percentual das vendas ilegais realizadas naquele Território no caixa da Organização automaticamente.
5. WHEN um Jogador contrata serviço via app Serviços do celular, THE Sistema_Celular SHALL criar notificação para prestadores da profissão correspondente online e, ao ser atendido, iniciar rastreamento de localização entre os dois.
6. WHEN um Jogador compra veículo na concessionária, THE Sistema_Progressão SHALL verificar o nível do Jogador e aplicar desconto progressivo (1% por nível acima de 10) como benefício de progressão.
7. WHEN uma empresa paga salários e tributos ao mesmo tempo, THE Sistema_Empresa SHALL garantir que as operações sejam realizadas na ordem: tributos primeiro, salários em seguida, e notificar o dono se o caixa for insuficiente para ambos.

---

### Requisito 22: Identidade Visual e Interface

**User Story:** Como jogador, quero uma interface consistente com a identidade visual do Aurora Roleplay, para que a experiência visual seja coesa e profissional em todas as telas e mensagens.

#### Critérios de Aceitação

1. THE Sistema_Chat SHALL formatar todas as mensagens do servidor usando a paleta oficial: destaque em #00C8FF, texto geral em #FFFFFF, avisos automáticos em gradiente #D0D0D0 a #707070, erros em #FF3B30 e confirmações em #32D74B.
2. THE Sistema_Admin SHALL formatar todas as mensagens de Staff com a cor #A855F7 para diferenciação imediata.
3. THE Sistema_Chat SHALL formatar todas as mensagens de rádio/comunicador organizacional com a cor #FF9500.
4. THE Sistema_Personagem SHALL exibir HUD permanente mostrando: HP atual, colete atual, fome, sede, dinheiro em mão e nível do personagem com cores da paleta oficial.
5. THE Sistema_Organização SHALL exibir tag da Organização do Jogador acima do nome na Label_3D com a cor da facção definida pelo líder.
6. WHEN qualquer Dialog é exibido ao Jogador, THE Sistema_Personagem SHALL usar fonte e formatação consistentes com o tema Aurora: título em #00C8FF em negrito e corpo em #FFFFFF.
