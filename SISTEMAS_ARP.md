# Aurora Roleplay — Documentação de Sistemas
**Servidor:** Aurora Roleplay  
**Abreviação:** ARP  
**Versão:** v1.0  
**Developer:** Apxgui  
**Arquivo principal:** `gamemodes/ARP.pwn`  

---

## Paleta de Cores

| Elemento | Hex | Uso |
|---|---|---|
| Destaque principal | `#00C8FF` | Nome do servidor, títulos, labels 3D |
| Texto geral | `#FFFFFF` | Informações, corpo de texto |
| Mensagens automáticas | `#D0D0D0` → `#707070` | Avisos do servidor |
| Erros / alertas | `#FF3B30` | Mensagens de erro |
| Confirmações | `#32D74B` | Mensagens de sucesso |
| Staff global | `#A855F7` | Chat de administração |
| Rádio / comunicador | `#FF9500` | Comunicação de facção |

---

## Plugins Utilizados

| Plugin | Função |
|---|---|
| `streamer` | Objetos, pickups e áreas dinâmicas |
| `sscanf` | Parsing de argumentos de comandos |
| `pawncmd` | Sistema de comandos |
| `sampvoice` | VoIP por proximidade e streams de rádio |
| `discord-connector` | Integração com Discord |
| `mapandreas` | Cálculo de alturas do mapa |
| `crashdetect` | Detecção e log de crashes |

---

## Módulos Externos

| Arquivo | Conteúdo |
|---|---|
| `modulos/sistema_botsrck.inc` | Bots e NPCs customizados |
| `mapas/mapserver.inc` | Mapeamentos e objetos customizados |

---

## Estrutura de Arquivos (scriptfiles)

```
scriptfiles/
├── players/        — Dados dos jogadores (.ini por jogador)
├── veiculos/       — Dados dos veículos
├── casas/          — Dados das casas
├── empresas/       — Dados das empresas
├── faccoes/        — Dados das facções
├── admins/         — Lista de administradores
└── avisos/         — Mensagens agendadas automáticas
```

---

## Sistemas

---

### 1. Sistema de Login / Registro
**Descrição:** Registro e login de jogadores com senha hasheada em MD5. No primeiro acesso, o jogador escolhe sexo, idade e origem via dialogs. Possui anti-flood para tentativas incorretas de senha.  
**Linha aproximada:** 2800–3500  
**Dados salvos:** Nome, Senha (MD5), Email, IP, DataCriacao, UltimoLogin

---

### 2. Sistema de Personagem / Perfil
**Descrição:** Cada jogador possui um perfil de RP com sexo, idade, origem e skin personalizada. O comando `/perfil` exibe todos os dados públicos do personagem.  
**Linha aproximada:** 3500–4200  
**Comandos:** `/perfil`, `/aparencia`, `/skin`

---

### 3. Sistema de Level / XP
**Descrição:** Jogadores ganham XP ao realizar ações de RP. Ao acumular XP suficiente, sobem de level e desbloqueiam benefícios.  
**Linha aproximada:** 3800–4500  
**Dados:** Level, XP

---

### 4. Sistema Econômico
**Descrição:** Cada jogador possui dinheiro em mão e saldo bancário separados. Suporte a depósito, saque e transferência entre jogadores. ATMs espalhados pelo mapa para acesso ao banco.  
**Linha aproximada:** 8000–10000  
**Comandos:** `/depositar`, `/sacar`, `/transferir`, `/dinheiro`, `/banco`

---

### 5. Sistema de Casas
**Descrição:** Casas compráveis com dono, preço configurável, sistema de tranca e interiores. Pickups e labels 3D marcam as entradas. Dados persistidos em arquivos por casa.  
**Linha aproximada:** 12000–15000  
**Comandos:** `/comprarCasa`, `/venderCasa`, `/trancarcasa`, `/destrancarCasa`, `/informacoes`  
**Dados:** Dono, Preco, Trancada, Interior, Posição de entrada/saída

---

### 6. Sistema de Empresas
**Descrição:** Empresas com dono, tipo (loja, posto, etc.), estoque e caixa próprio. Comandos para compra, venda e gerenciamento pelo dono.  
**Linha aproximada:** 15000–18000  
**Dados:** Dono, Nome, Tipo, Preco, Estoque, Caixa

---

### 7. Sistema de Veículos
**Descrição:** Veículos pessoais com placa, combustível, km rodados, dono e cores personalizáveis. Concessionária para compra. Veículos persistem entre sessões.  
**Linha aproximada:** 18000–22000  
**Comandos:** `/ligarCarro`, `/desligarCarro`, `/trancarveiculo`, `/abastecerveiculo`, `/motor`, `/guardarVeiculo`, `/pegarVeiculo`  
**Dados:** Modelo, Placa, Proprietário, Cor1, Cor2, Combustivel, Km, Trancado

---

### 8. Sistema de Combustível
**Descrição:** Veículos consomem combustível progressivamente ao se moverem. Postos de gasolina distribuídos pelo mapa para abastecimento. O veículo para automaticamente quando o combustível acaba.  
**Linha aproximada:** 22000–23500  
**Timer:** `TimerCombustivel`

---

### 9. Sistema de Estacionamento / Garagem
**Descrição:** Jogadores podem guardar veículos em garagens. Quando guardado, o veículo é removido do mapa e restaurado ao ser retirado.  
**Linha aproximada:** 22000–22500  
**Comandos:** `/guardarVeiculo`, `/pegarVeiculo`

---

### 10. Sistema de Saúde / Morte / Respawn
**Descrição:** Ao morrer, o jogador fica em estado incapacitado. Paramédicos podem socorrê-lo. Após tempo limite sem socorro, respawn automático no hospital. Mortes e kills são registrados no perfil.  
**Linha aproximada:** 5500–6500  
**Dados:** TotalMortes, TotalKills

---

### 11. Sistema de Fome e Sede
**Descrição:** Fome e sede diminuem com o tempo. Quando chegam a zero, o jogador perde HP gradualmente. Itens de comida e bebida espalhados pelo servidor restauram os valores.  
**Linha aproximada:** 23500–25000  
**Timer:** `TimerFome`  
**Dados:** Fome, Sede

---

### 12. Sistema de Facções / Organizações
**Descrição:** Sistema completo de facções (polícia, bombeiros, facções criminosas, etc.) com ranks hierárquicos, líder, caixa próprio e tag. Inclui comunicação interna por rádio e comandos de gestão de membros.  
**Linha aproximada:** 25000–32000  
**Comandos:** `/convidar`, `/expulsar`, `/promover`, `/rebaixar`, `/rankname`, `/tagfaccao`, `/caixafaccao`, `/membros`, `/f`, `/r`  
**Dados:** Nome, Tag, Tipo, Lider, Caixa, Ranks[], Membros

---

### 13. Sistema de Armas
**Descrição:** Cada jogador pode carregar uma arma primária e uma pistola. Munição controlada individualmente. Itens de arma disponíveis no mundo. Integrado com permissões de facção.  
**Linha aproximada:** 32000–35000  
**Dados:** ArmaPrincipal, ArmaPistola, MunicaoPrincipal, MunicaoPistola, Colete

---

### 14. Sistema Policial
**Descrição:** Conjunto de comandos exclusivos para policiais. Controle de algemas, prisão, revista de suspeitos, multas e sistema de procurado (wanted level).  
**Linha aproximada:** 35000–40000  
**Comandos:** `/algemar`, `/desalgemar`, `/prender`, `/soltar`, `/revista`, `/multar`, `/wanted`, `/pa`

---

### 15. Sistema de Prisão
**Descrição:** Jogadores presos são teleportados para a penitenciária. Timer de prisão regressivo. Saída automática após cumprir a pena.  
**Linha aproximada:** 36000–38000  
**Timer:** `TimerPrisao`  
**Dados:** Preso, Algemado

---

### 16. Sistema de SAMU / Bombeiro
**Descrição:** Paramédicos podem socorrer jogadores incapacitados no chão com o comando `/socorrer`. Bombeiros possuem comandos específicos para seu papel de RP.  
**Linha aproximada:** 40000–42000  
**Comandos:** `/socorrer`, `/reanimar`

---

### 17. Sistema de Mecânico
**Descrição:** Mecânicos podem consertar veículos de outros jogadores quando estão próximos. Suporte a reboque de veículos danificados.  
**Linha aproximada:** 41500–42500  
**Comandos:** `/consertar`, `/rebocar`

---

### 18. Sistema de Celular / Telefone
**Descrição:** Chamadas telefônicas em tempo real entre jogadores, envio de SMS e lista de contatos. As chamadas simulam distância — o jogador precisa estar disponível para atender.  
**Linha aproximada:** 42000–44500  
**Comandos:** `/ligar`, `/atender`, `/desligar`, `/sms`

---

### 19. Sistema de VoIP (SampVoice)
**Descrição:** Voz local por proximidade usando SampVoice. Streams separados para rádio de facção e megafone. Configuração independente por facção.  
**Linha aproximada:** 2000–2200  
**Dados:** SvStream (por jogador)  
**Comandos:** `/microfone`, `/voip`

---

### 20. Sistema de Chat RP
**Descrição:** Comandos de roleplay para descrever ações e emotes no chat local. Mensagens com alcance limitado por proximidade para imersão.  
**Linha aproximada:** 56000–58000  
**Comandos:** `/me`, `/do`, `/ame`, `/todo`, `/s` (grito), `/g` (global IC)

---

### 21. Sistema de Animações
**Descrição:** Comandos de animação para ações de RP. Override do ApplyAnimation para controle centralizado das animações disponíveis.  
**Linha aproximada:** 54000–56000

---

### 22. Sistema de VIP
**Descrição:** Níveis VIP com benefícios exclusivos como skins especiais, comandos extras e outras vantagens. Gerenciado por admins.  
**Linha aproximada:** 52000–54000  
**Comandos:** `/vip`, `/vipinfo`  
**Dados:** VipLevel, PontosVip

---

### 23. Sistema de Administração
**Descrição:** Níveis de admin de 1 a 5+. Arsenal completo de comandos administrativos com log de ações. Suporte a spectate, teleporte, manipulação de dados de jogadores e anúncios globais.  
**Linha aproximada:** 44500–52000  
**Comandos:** `/ban`, `/kick`, `/mute`, `/unmute`, `/setlevel`, `/setadmin`, `/setvip`, `/goto`, `/bring`, `/tele`, `/spectate`, `/stopspec`, `/giveweapon`, `/givemoney`, `/setmoney`, `/setbank`, `/setskin`, `/setfome`, `/setsede`, `/setcolete`, `/announce`, `/admins`, `/players`, `/logs`  
**Dados:** AdminLevel

---

### 24. Sistema de Avisos / Anúncios Agendados
**Descrição:** Mensagens automáticas exibidas periodicamente para todos os jogadores online. Conteúdo configurável por arquivo externo em `scriptfiles/avisos/`.  
**Timer:** `TimerAvisos`

---

### 25. Sistema de Clima e Hora
**Descrição:** Hora do servidor sincronizada e clima dinâmico. Mudanças automáticas de clima ao longo do dia.  
**Linha aproximada:** 2644–2700  
**Timer:** `TimerHora`

---

### 26. Sistema de Blips / Radar
**Descrição:** Blips personalizados no radar para todos os locais importantes do servidor (hospital, banco, delegacia, postos, etc.).  
**Linha aproximada:** 2700–2800

---

### 27. Sistema de Bots (sistema_botsrck)
**Descrição:** Bots com comportamento simples para lojas, mecânicos e outros pontos de serviço. Configurados no módulo `modulos/sistema_botsrck.inc`.  
**Arquivo:** `modulos/sistema_botsrck.inc`

---

### 28. Sistema de NPCs (Ônibus / Helicóptero)
**Descrição:** NPCs autônomos dirigindo ônibus em rotas gravadas e helicópteros. Passageiros podem embarcar e desembarcar em pontos de parada.  
**Arquivos:** `npcmodes/onibusrck.pwn`, `npcmodes/helils.pwn`  
**Gravações:** `npcmodes/recordings/`

---

### 29. Sistema de Save / Load
**Descrição:** Todos os dados de jogadores, veículos, casas, empresas e facções são salvos em arquivos `.ini` no scriptfiles. Carregados no login/inicialização e salvos periodicamente e no logout.  
**Timer:** `TimerSalvar`  
**Linha aproximada:** Load: 3200–3600 | Save: 60000–62000

---

## Callbacks Implementados

| Callback | Descrição |
|---|---|
| `OnGameModeInit` | Inicialização completa do servidor |
| `OnGameModeExit` | Salvamento final ao encerrar |
| `OnPlayerConnect` | Conexão (anti-flood, init de variáveis) |
| `OnPlayerDisconnect` | Desconexão (salvamento de dados) |
| `OnPlayerSpawn` | Spawn (posicionamento, HUD) |
| `OnPlayerDeath` | Morte (sistema incapacitado, stats) |
| `OnVehicleDeath` | Destruição de veículo |
| `OnPlayerText` | Chat (filtros, RP local por proximidade) |
| `OnPlayerCommandText` | Fallback de comandos (erro) |
| `OnDialogResponse` | Tratamento de todos os dialogs (150+ IDs) |
| `OnPlayerEnterVehicle` | Entrada em veículo |
| `OnPlayerExitVehicle` | Saída de veículo |
| `OnPlayerStateChange` | Mudança de estado do jogador |
| `OnPlayerEnterCheckpoint` | Entrada em checkpoint |
| `OnPlayerLeaveCheckpoint` | Saída de checkpoint |
| `OnPlayerPickUpPickup` | Coleta de pickups |
| `OnPlayerKeyStateChange` | Teclas especiais pressionadas |
| `OnVehicleMod` | Modificação de veículo |
| `OnVehiclePaintjob` | Pintura de veículo |
| `OnVehicleRespray` | Respray de veículo |
| `OnPlayerClickMap` | Clique no mapa (admin teleporte) |
| `OnPlayerUpdate` | Atualização contínua (combustível, fome, sede) |
| `OnRconCommand` | Comandos RCON |

---

## Timers

| Timer | Função |
|---|---|
| `TimerSalvar` | Salva dados de todos os jogadores periodicamente |
| `TimerFome` | Reduz fome e sede ao longo do tempo |
| `TimerCombustivel` | Consome combustível dos veículos em movimento |
| `TimerHora` | Atualiza hora do servidor |
| `TimerAvisos` | Exibe mensagens agendadas para todos |
| `TimerPrisao` | Conta regressiva da pena de prisão |

---

## Lista Completa de Comandos

### Gerais
`/ajuda` `/help` `/cmds` `/stats` `/perfil` `/inventario` `/id` `/report` `/newbie` `/ad`

### Conta / Personagem
`/registro` `/login` `/sair` `/skin` `/aparencia`

### Econômicos
`/depositar` `/sacar` `/transferir` `/dinheiro` `/banco`

### Veículos
`/ligarCarro` `/desligarCarro` `/trancarveiculo` `/abastecerveiculo` `/motor` `/guardarVeiculo` `/pegarVeiculo`

### Imóveis
`/comprarCasa` `/venderCasa` `/trancarcasa` `/destrancarCasa` `/informacoes`

### Facção
`/convidar` `/expulsar` `/promover` `/rebaixar` `/rankname` `/tagfaccao` `/caixafaccao` `/membros` `/f` `/r`

### Policial
`/algemar` `/desalgemar` `/prender` `/soltar` `/revista` `/multar` `/wanted` `/pa`

### SAMU / Bombeiro
`/socorrer` `/reanimar`

### Mecânico
`/consertar` `/rebocar`

### Comunicação
`/ligar` `/atender` `/desligar` `/sms` `/me` `/do` `/ame` `/todo` `/s` `/g`

### VoIP
`/microfone` `/voip`

### VIP
`/vip` `/vipinfo`

### Admin
`/ban` `/kick` `/mute` `/unmute` `/setlevel` `/setadmin` `/setvip` `/goto` `/bring` `/tele` `/spectate` `/stopspec` `/giveweapon` `/givemoney` `/setmoney` `/setbank` `/setskin` `/setfome` `/setsede` `/setcolete` `/announce` `/admins` `/players` `/logs`

---

*Documento gerado automaticamente com base na análise do código-fonte — Aurora Roleplay v1.0*
