# Documento de Design — Aurora Roleplay Gamemode

## Overview

O Aurora Roleplay (ARP) é uma gamemode de Roleplay completa para SA-MP, escrita inteiramente em Pawn. A arquitetura adota o padrão de **módulos separados por sistema** (arquivos `.inc` sob `modulos/`), com um arquivo principal `gamemodes/ARP.pwn` que atua como orquestrador de inicialização e callbacks. Toda a persistência é baseada em arquivos `.ini` via biblioteca **DOF2**, sem banco de dados relacional. A comunicação entre sistemas é feita por chamadas diretas de função, eventos compartilhados (callbacks públicos) e variáveis globais tipadas.

A filosofia central — *sistemas que conversam entre si* — é implementada por meio de uma **cadeia econômica** explícita:

```
Jogador → Emprego → Economia → Banco → Veículo → Empresa → Cidade
```

Cada ação com efeito econômico passa obrigatoriamente pelas funções atômicas de `sistema_economia.inc`, garantindo consistência de saldo.

---

## Architecture

### Estrutura de Diretórios

```
gamemodes/
  ARP.pwn                  ← Arquivo principal: includes, callbacks, inicialização

modulos/
  sistema_login.inc        ← Req 1: Registro, autenticação, sessão
  sistema_personagem.inc   ← Req 2, 6, 7, 20: Perfil, morte, fome/sede, HUD, salvamento
  sistema_progressao.inc   ← Req 3, 16: XP, nível, reputação, profissões, conquistas
  sistema_economia.inc     ← Req 4, 21: Operações atômicas, salários, impostos
  sistema_banco.inc        ← Req 4: ATM, extrato, PIN, operações bancárias
  sistema_inventario.inc   ← Req 5: Itens, peso, categorias
  sistema_casa.inc         ← Req 8: Imóveis, tranca, label, regen
  sistema_empresa.inc      ← Req 9: Comércio, estoque, funcionários, caixa
  sistema_veiculo.inc      ← Req 10: Veículos pessoais, combustível, garagem, seguro
  sistema_celular.inc      ← Req 11: Apps, contatos, SMS, GPS, anúncios
  sistema_chat.inc         ← Req 12, 22: Chat local, /s, /me, /do, /ame, filtro
  sistema_voip.inc         ← Req 12: SampVoice streams (local, facção, chamada)
  sistema_organizacao.inc  ← Req 13: Facções, orgs criminosas, ranks, território
  sistema_policial.inc     ← Req 14, 15: Ocorrências, evidências, algemas, ficha, prisão
  sistema_cidade.inc       ← Req 17: NPCs, clima, ciclo dia/noite, eventos urbanos
  sistema_admin.inc        ← Req 18, 19: Comandos staff, VIP, logs administrativos
  sistema_botsrck.inc      ← NPC ônibus e helicóptero (já existente)

mapas/
  mapserver.inc            ← Objetos estáticos do mapa (já existente)

scriptfiles/
  Players/%s.ini           ← Dados do jogador
  Casas/%d.ini             ← Dados de cada casa
  Empresas/%d.ini          ← Dados de cada empresa
  Organizacoes/Org%d.ini   ← Dados de cada organização
  Veiculos/%s.ini          ← Veículos pessoais por nome do dono
  Logs/Admin.ini           ← Log persistente de ações administrativas
  Logs/Moderacao.ini       ← Log de palavras proibidas e infrações de chat
  Config/Servidor.ini      ← Configurações globais do servidor
```

### Grafo de Dependências de Módulos

```
ARP.pwn
 ├── sistema_login.inc        (usa: sistema_personagem)
 ├── sistema_personagem.inc   (usa: sistema_economia, sistema_progressao, sistema_inventario)
 ├── sistema_progressao.inc   (usa: sistema_economia)
 ├── sistema_economia.inc     (núcleo — sem dependências de módulos de negócio)
 ├── sistema_banco.inc        (usa: sistema_economia)
 ├── sistema_inventario.inc   (independente)
 ├── sistema_casa.inc         (usa: sistema_economia)
 ├── sistema_empresa.inc      (usa: sistema_economia, sistema_inventario)
 ├── sistema_veiculo.inc      (usa: sistema_economia, sistema_progressao)
 ├── sistema_celular.inc      (usa: sistema_banco, sistema_voip, sistema_chat)
 ├── sistema_chat.inc         (independente — funções de formatação puras)
 ├── sistema_voip.inc         (usa: sampvoice.inc)
 ├── sistema_organizacao.inc  (usa: sistema_economia, sistema_chat)
 ├── sistema_policial.inc     (usa: sistema_economia, sistema_inventario, sistema_organizacao)
 ├── sistema_cidade.inc       (usa: sistema_economia, sistema_veiculo)
 └── sistema_admin.inc        (usa: todos — acesso administrativo geral)
```

### Convenções de Código Pawn

- **Todos os módulos** são arquivos `.inc` incluídos no `ARP.pwn` na ordem do grafo acima.
- Comandos usam `pawncmd` via macro `CMD:%0(%1,%2)` (equivalente ao `zcmd` presente no projeto).
- Parsing de argumentos usa `sscanf2` com formatos `"i"`, `"s"`, `"f"`, `"p<;>s[32]s[32]"` etc.
- Operações de arquivo usam **DOF2** como biblioteca principal (`DOF2_Open`, `DOF2_SetString`, `DOF2_SaveFile`), com fallback para `Dini` onde necessário.
- Iteração sobre jogadores usa `foreach` (`foreach(new i : Player)`).
- Timers globais usam `SetTimer` / `SetTimerEx`; timers por jogador usam `SetTimerEx` com `playerid`.

---

## Components and Interfaces

### `sistema_login.inc`

**Responsabilidade:** Registro, autenticação, sessão e fluxo inicial.

**Funções principais:**
```pawn
// Exibe dialog de registro para novo jogador
Login_ExibirRegistro(playerid);

// Exibe dialog de login para jogador com conta existente
Login_ExibirLogin(playerid);

// Processa tentativa de registro: valida senha e e-mail, cria arquivo ini
// Retorna 1 em sucesso, 0 em falha (motivo em 'erro[]')
Login_ProcessarRegistro(playerid, const senha[], const confirma[], const email[], erro[64]);

// Verifica senha contra hash MD5 armazenado
// Retorna 1 se correta, 0 se incorreta
Login_VerificarSenha(playerid, const senha[]);

// Carrega todos os dados do jogador do arquivo ini para os arrays globais
Login_CarregarDados(playerid);

// Incrementa contador de tentativas falhas; expulsa ao atingir MAX_TENTATIVAS (5)
Login_RegistrarFalha(playerid);

// Reseta o contador de tentativas falhas
Login_ResetarFalhas(playerid);
```

**Constantes:**
```pawn
#define LOGIN_MAX_TENTATIVAS    5
#define LOGIN_SENHA_MIN         6
#define LOGIN_SENHA_MAX         20
```

---

### `sistema_personagem.inc`

**Responsabilidade:** Perfil de RP, estado físico (HP, morte, fome, sede), HUD e salvamento automático.

**Funções principais:**
```pawn
// Inicia fluxo de criação de personagem (dialogs sequenciais)
Personagem_IniciarCriacao(playerid);

// Salva todos os dados do jogador no arquivo ini
Personagem_Salvar(playerid);

// Carrega dados do jogador do arquivo ini para pInfo[playerid]
Personagem_Carregar(playerid);

// Coloca jogador em estado incapacitado (HP = 0)
Personagem_Incapacitar(playerid, killer);

// Finaliza morte: teleporte para hospital, registra morte, cobra taxa
Personagem_MorteDefinitiva(playerid);

// Aplica regeneração de HP por descanso em casa (10 HP/min)
Personagem_RegenCasa(playerid);

// Atualiza HUD do jogador (HP, colete, fome, sede, dinheiro, nível)
Personagem_AtualizarHUD(playerid);

// Exibe o perfil de um jogador (dados públicos de RP)
Personagem_ExibirPerfil(playerid, alvo);

// Retorna patrimônio total calculado
Float:Personagem_Patrimonio(playerid);

// Valida se a idade está no intervalo permitido [18, 70]
// Retorna 1 se válida, 0 se inválida
Personagem_ValidarIdade(idade);
```

**Timer interno:**
- `TIMER_FOME_SEDE` — a cada 600.000 ms (10 min): decai fome –5, sede –7; aplica dano se < 20.
- `TIMER_AUTOSAVE` — a cada 300.000 ms (5 min): chama `Personagem_Salvar` para todos conectados.
- `TIMER_HUD` — a cada 1.000 ms: chama `Personagem_AtualizarHUD` via textdraws.
- `TIMER_INCAPACITADO` — por jogador, 300.000 ms (5 min): dispara `Personagem_MorteDefinitiva`.

---

### `sistema_progressao.inc`

**Responsabilidade:** XP, nível, reputação, profissões, conquistas, patrimônio.

**Funções principais:**
```pawn
// Adiciona XP ao jogador; verifica level up; aplica multiplicador VIP Ouro (1.5x)
Progressao_AdicionarXP(playerid, quantidade);

// Verifica e executa level up se XP >= limiar do próximo nível
Progressao_VerificarLevelUp(playerid);

// Define profissão ativa; registra histórico com timestamp
Progressao_DefinirProfissao(playerid, profissao);

// Registra conquista com data; exibe notificação
Progressao_RegistrarConquista(playerid, tipo_conquista);

// Retorna XP necessário para o próximo nível
Progressao_XPParaProximoNivel(nivel);

// Calcula desconto de concessionária por nível (1% por nível acima de 10)
Float:Progressao_DescontoConcessionaria(playerid);
```

**Enum de profissões:**
```pawn
enum E_Profissao {
    PROF_DESEMPREGADO,
    PROF_POLICIAL,
    PROF_PARAMEDICO,
    PROF_BOMBEIRO,
    PROF_MECANICO,
    PROF_TAXISTA,
    PROF_MOTORISTA_ONIBUS,
    PROF_EMPRESARIO,
    PROF_ADVOGADO,
    PROF_JORNALISTA
}
```

---

### `sistema_economia.inc`

**Responsabilidade:** Núcleo de operações financeiras atômicas. Nenhum outro módulo altera saldos diretamente — toda movimentação passa por este módulo.

**Funções principais:**
```pawn
// Tenta dar dinheiro em mão ao jogador; respeita limite R$50.000
// Retorna valor efetivamente creditado (pode ser menor se atingir o teto)
Economia_DarDinheiro(playerid, valor);

// Retira dinheiro em mão; falha silenciosa se insuficiente (retorna 0)
// Retorna 1 em sucesso, 0 se saldo insuficiente
Economia_RetirarDinheiro(playerid, valor);

// Operação atômica: retira do dinheiro em mão, credita no banco
// Retorna 1 em sucesso; 0 se dinheiro_em_mao < valor
Economia_Depositar(playerid, valor);

// Operação atômica: retira do banco, credita no dinheiro em mão
// Retorna 1 em sucesso; 0 se banco < valor ou cash + valor > 50000
Economia_Sacar(playerid, valor);

// Transferência atômica entre dois jogadores (banco → banco)
// Registra extrato em ambos; retorna 1 em sucesso
Economia_Transferir(remetente, destinatario, valor);

// Crédito direto no saldo bancário (salários, seguros, etc.)
Economia_CreditarBanco(playerid, valor);

// Débito do saldo bancário; retorna 1 em sucesso, 0 se insuficiente
Economia_DebitarBanco(playerid, valor);

// Executa ciclo de salários para todos os jogadores conectados
Economia_CicloSalarios();

// Executa cobrança de imposto mensal sobre patrimônio
Economia_CicloImpostos();

// Registra entrada no extrato (últimas 50 transações)
Economia_RegistrarExtrato(playerid, tipo[], valor, envolvido[], timestamp);
```

**Constantes:**
```pawn
#define ECO_CASH_MAXIMO         50000
#define ECO_TAXA_HOSPITAL       500
#define ECO_TAXA_ANUNCIO        100
#define ECO_TAXA_PASSAGEM       5
#define ECO_SEGURO_COBERTURA    0.80   // float: 80% do valor de mercado
#define ECO_MAX_EXTRATO         50
```

**Invariante central:** `pInfo[playerid][Dinheiro]` ≤ `ECO_CASH_MAXIMO` em toda e qualquer operação.

---

### `sistema_banco.inc`

**Responsabilidade:** Interface de ATM/agência, extrato, PIN de 4 dígitos.

**Funções principais:**
```pawn
// Exibe menu bancário (depósito, saque, transferência, extrato, saldo)
Banco_ExibirMenu(playerid);

// Verifica PIN do jogador; retorna 1 se correto
Banco_VerificarPIN(playerid, pin);

// Define/altera PIN do jogador
Banco_DefinirPIN(playerid, pin);

// Exibe extrato formatado (últimas 50 transações)
Banco_ExibirExtrato(playerid);
```

---

### `sistema_inventario.inc`

**Responsabilidade:** Lista de itens, controle de peso, categorias, descarte.

**Estrutura de dados:**
```pawn
#define INV_MAX_ITENS           20
#define INV_PESO_PADRAO         15.0
#define INV_PESO_VIP2           25.0

enum E_CategoriaItem {
    CAT_CONSUMIVEL,
    CAT_FERRAMENTA,
    CAT_ARMA,
    CAT_MUNICAO,
    CAT_DOCUMENTO,
    CAT_ELETRONICO,
    CAT_MISC
}

enum E_Item {
    item_id,
    item_nome[32],
    item_quantidade,
    Float:item_peso_unitario,
    E_CategoriaItem:item_categoria
}

new pInventario[MAX_PLAYERS][INV_MAX_ITENS][E_Item];
new Float:pPesoTotal[MAX_PLAYERS];
```

**Funções principais:**
```pawn
// Adiciona item ao inventário; respeita limite de peso e capacidade VIP
// Retorna 1 em sucesso, 0 se exceder capacidade
Inventario_AdicionarItem(playerid, item_id, quantidade, Float:peso_unitario, categoria);

// Remove item do inventário; deposita pickup no chão
Inventario_DescartarItem(playerid, slot);

// Usa item consumível; remove e aplica efeito
Inventario_UsarItem(playerid, slot);

// Retorna capacidade máxima do jogador (15 kg ou 25 kg para VIP2+)
Float:Inventario_CapacidadeMaxima(playerid);

// Retorna peso total atual do inventário
Float:Inventario_PesoAtual(playerid);

// Exibe dialog de inventário
Inventario_ExibirDialog(playerid);

// Verifica se o jogador pode adicionar item com dado peso
// Retorna 1 se cabe, 0 se não cabe
Inventario_PodeCaber(playerid, Float:peso_item);
```

---

### `sistema_casa.inc`

**Responsabilidade:** Imóveis, compra/venda, tranca, regen de HP, labels.

**Estrutura de dados:**
```pawn
#define MAX_CASAS   1000

enum E_Casa {
    casa_id,
    casa_dono[MAX_PLAYER_NAME],
    casa_preco,
    bool:casa_trancada,
    casa_pickup,
    Text3D:casa_label,
    casa_interior,
    Float:casa_entrada[3],
    Float:casa_interior_pos[3],
    casa_comodos
}

new CasaData[MAX_CASAS][E_Casa];
```

**Funções principais:**
```pawn
// Carrega todas as casas dos arquivos ini
Casa_CarregarTodas();

// Salva dados de uma casa específica
Casa_Salvar(casa_id);

// Processa compra de casa; chama Economia_DebitarBanco atomicamente
// Retorna 1 em sucesso, 0 se saldo insuficiente
Casa_Comprar(playerid, casa_id);

// Marca casa como à venda com preço definido
Casa_VenderAnunciar(playerid, casa_id, preco);

// Alterna estado de tranca; atualiza label 3D
Casa_AlternarTranca(playerid, casa_id);

// Aplica regen de HP (10 HP/min) para jogador dentro de sua casa
Casa_AplicarRegen(playerid);

// Atualiza o Label_3D da entrada da casa
Casa_AtualizarLabel(casa_id);
```

**Caminho do arquivo:** `scriptfiles/Casas/%d.ini`

---

### `sistema_empresa.inc`

**Responsabilidade:** Comércio, estoque, funcionários, caixa, histórico financeiro.

**Estrutura de dados:**
```pawn
#define MAX_EMPRESAS        1000
#define EMP_MAX_HISTORICO   100

enum E_Empresa {
    emp_id,
    emp_nome[64],
    emp_tipo[32],
    emp_dono[MAX_PLAYER_NAME],
    emp_caixa,
    emp_pickup,
    emp_label,
    Float:emp_pos[3],
    Float:emp_interior_pos[3],
    emp_interior
}

enum E_EstoqueItem {
    estoque_item_id,
    estoque_nome[32],
    estoque_quantidade,
    estoque_max,
    estoque_preco
}

// Estoque por empresa (até 20 produtos)
new EmpEstoque[MAX_EMPRESAS][20][E_EstoqueItem];

// Lista de funcionários por empresa (até 30)
new EmpFuncionarios[MAX_EMPRESAS][30][MAX_PLAYER_NAME];
new EmpFuncionarioSalario[MAX_EMPRESAS][30];
```

**Funções principais:**
```pawn
// Exibe painel de gerenciamento da empresa (tabs: caixa, estoque, funcionários, histórico)
Empresa_ExibirPainel(playerid, emp_id);

// Processa venda de produto para jogador; atômico (estoque + caixa + inventário)
Empresa_VenderProduto(playerid, emp_id, item_slot);

// Executa pagamento de salários; tributos primeiro, salários depois
// Retorna total pago
Empresa_PagarSalarios(emp_id);

// Verifica estoque baixo (< 10%) e notifica dono
Empresa_VerificarEstoque(emp_id);

// Registra transação no histórico financeiro
Empresa_RegistrarHistorico(emp_id, tipo[], valor, envolvido[], timestamp);

// Coloca empresa em leilão automático após 30 dias sem atividade do dono
Empresa_VerificarInatividade(emp_id);

// Salva dados da empresa
Empresa_Salvar(emp_id);
```

**Caminho do arquivo:** `scriptfiles/Empresas/%d.ini`

---

### `sistema_veiculo.inc`

**Responsabilidade:** Veículos pessoais, combustível, garagem, seguro, transferência.

**Estrutura de dados:**
```pawn
#define MAX_VEICULOS_PESSOAIS   (MAX_PLAYERS * 3)

enum E_Veiculo {
    veh_id_samp,
    veh_modelo,
    veh_placa[8],
    veh_dono[MAX_PLAYER_NAME],
    veh_cor1,
    veh_cor2,
    Float:veh_combustivel,
    Float:veh_km,
    Float:veh_dano,
    bool:veh_trancado,
    bool:veh_motor_ligado,
    bool:veh_guardado,
    bool:veh_segurado,
    veh_seguro_expira,          // timestamp Unix
    Float:veh_seguro_valor_mercado,
    Float:veh_pos[3],
    Float:veh_angulo
}

new VeiculoData[MAX_VEICULOS_PESSOAIS][E_Veiculo];
```

**Funções principais:**
```pawn
// Compra veículo na concessionária; aplica desconto por nível; gera placa única
Veiculo_Comprar(playerid, modelo);

// Gera placa única alfanumérica (7 chars)
Veiculo_GerarPlaca(placa[8]);

// Liga/desliga motor; verifica combustível e dano crítico
Veiculo_AlternarMotor(playerid, veh_index);

// Processa consumo de combustível (1 unidade / 500m)
Veiculo_ConsumirCombustivel(veh_index, Float:distancia);

// Abastece veículo; cobra do dinheiro em mão
Veiculo_Abastecer(playerid, veh_index, litros);

// Guarda veículo na garagem; despawna do mapa
Veiculo_Guardar(playerid, veh_index);

// Retira veículo da garagem; spawna na saída
Veiculo_Retirar(playerid, veh_index);

// Transfere veículo para outro jogador
Veiculo_Transferir(playerid, alvo, veh_index);

// Registra seguro (30 dias); débita prêmio do banco
Veiculo_ContratarSeguro(playerid, veh_index);

// Processa destruição com seguro (crédito de 80% do valor de mercado)
Veiculo_ProcessarDestruicao(veh_index);

// Salva estado do veículo
Veiculo_Salvar(veh_index);
```

**Caminho do arquivo:** `scriptfiles/Veiculos/%s.ini` (por nome do dono)

**Timer:** `TIMER_VEICULO_SAVE` — a cada 300.000 ms: salva estado de todos os veículos spawnados.

---

### `sistema_celular.inc`

**Responsabilidade:** Apps, contatos, SMS, GPS, banco mobile, anúncios, serviços.

**Estrutura de dados:**
```pawn
#define CEL_MAX_CONTATOS    50
#define CEL_MAX_SMS         20

enum E_Contato {
    cont_nome[MAX_PLAYER_NAME],
    cont_numero[12]
}

new pContatos[MAX_PLAYERS][CEL_MAX_CONTATOS][E_Contato];
new pSMSPendentes[MAX_PLAYERS][CEL_MAX_SMS][128];

// Chamada ativa
new pChamadaAlvo[MAX_PLAYERS];         // playerid do outro lado, -1 se sem chamada
new SV_Stream:pStreamChamada[MAX_PLAYERS]; // stream VoIP privado da chamada
```

**Funções principais:**
```pawn
// Exibe menu principal do celular
Celular_AbrirMenu(playerid);

// Inicia chamada para outro jogador; cria stream VoIP privado
Celular_IniciarChamada(playerid, alvo);

// Encerra chamada; destrói stream VoIP; registra no histórico
Celular_EncerrarChamada(playerid);

// Envia SMS; entrega imediato se online, persiste para próximo login
Celular_EnviarSMS(playerid, alvo, const mensagem[]);

// Exibe dialog GPS com categorias de destino; cria checkpoint
Celular_AbrirGPS(playerid);

// Transferência bancária via celular (exige PIN)
Celular_TransferenciaBancaria(playerid, alvo, valor);

// Publica anúncio global (cobra R$100)
Celular_PublicarAnuncio(playerid, const texto[]);

// Lista prestadores de serviço online da profissão solicitada
Celular_SolicitarServico(playerid, tipo_servico);
```

---

### `sistema_chat.inc`

**Responsabilidade:** Formatação e entrega de mensagens em todos os canais.

**Funções principais:**
```pawn
// Envia mensagem de chat local (raio 15m); aplica degradê de opacidade
Chat_Local(playerid, const mensagem[]);

// Envia grito (raio 40m, maiúsculas)
Chat_Grito(playerid, const mensagem[]);

// Formata e envia ação /me ou descrição /do (raio 10m)
Chat_AcaoRP(playerid, const mensagem[], tipo); // tipo: 0=/me, 1=/do

// Cria Label_3D temporário para /ame (5s, visível a 20m)
Chat_AmeLabel(playerid, const mensagem[]);

// Envia mensagem para todos os membros da organização
Chat_Fracao(playerid, const mensagem[]);

// Envia mensagem de rádio organizacional (cor #FF9500)
Chat_Radio(playerid, const mensagem[]);

// Envia mensagem IC global (cooldown 60s por jogador)
Chat_Global(playerid, const mensagem[]);

// Filtra palavras proibidas; retorna string limpa; registra em log se necessário
Chat_Filtrar(playerid, const mensagem[], saida[256]);

// Formata mensagem com a cor correta para o tipo informado
// Tipos: CHAT_DESTAQUE, CHAT_ERRO, CHAT_CONFIRMACAO, CHAT_STAFF, CHAT_RADIO, CHAT_SERVIDOR
Chat_Formatar(tipo, const mensagem[], saida[256]);

// Publica reportagem de jornalista (cooldown 10min)
Chat_Reportagem(playerid, const texto[]);
```

**Constantes de raio:**
```pawn
#define CHAT_RAIO_LOCAL     15.0
#define CHAT_RAIO_GRITO     40.0
#define CHAT_RAIO_ME_DO     10.0
#define CHAT_RAIO_AME       20.0
```

**Paleta de cores:**
```pawn
#define COR_DESTAQUE        0x00C8FFFF
#define COR_ERRO            0xFF3B30FF
#define COR_CONFIRMACAO     0x32D74BFF
#define COR_STAFF           0xA855F7FF
#define COR_RADIO           0xFF9500FF
#define COR_BRANCO          0xFFFFFFFF
#define COR_SERVIDOR1       0xD0D0D0FF
#define COR_SERVIDOR5       0x707070FF
```

---

### `sistema_voip.inc`

**Responsabilidade:** Streams SampVoice para chat local, rádio de facção e chamadas telefônicas.

**Estrutura de streams:**
```pawn
// Stream local: criado por jogador, reproduz voz para jogadores próximos (15m)
new SV_Stream:streamLocal[MAX_PLAYERS];

// Stream de facção: um por organização; reproduz para todos os membros online
new SV_Stream:streamFaccao[MAX_ORGS];

// Stream de chamada: par de streams privados por chamada ativa
// (alocados dinamicamente em sistema_celular ao iniciar chamada)
```

**Funções principais:**
```pawn
// Inicializa stream local do jogador ao conectar
VoIP_CriarStreamLocal(playerid);

// Destrói stream local ao desconectar
VoIP_DestruirStreamLocal(playerid);

// Cria stream de facção para organização
VoIP_CriarStreamFaccao(org_id);

// Adiciona/remove jogador ao stream de facção
VoIP_EntrarFaccao(playerid, org_id);
VoIP_SairFaccao(playerid, org_id);

// Cria par de streams privados para chamada telefônica
// Retorna o stream criado
SV_Stream:VoIP_CriarStreamChamada(playerid_a, playerid_b);

// Destrói os streams de uma chamada ativa
VoIP_DestruirStreamChamada(playerid_a, playerid_b);
```

**Efeito de estática no rádio de facção:** Aplicado via `SV_SetStreamAttenuation` com parâmetro de distorção.

---

### `sistema_organizacao.inc`

**Responsabilidade:** Facções e organizações criminosas, ranks, territórios, caixa, estoque ilegal.

**Estrutura de dados:**
```pawn
#define MAX_ORGS            50
#define ORG_MAX_RANKS       10
#define ORG_MAX_MEMBROS     100

enum E_TipoOrg {
    ORG_FACCAO_OFICIAL,
    ORG_CRIMINAL
}

enum E_Organizacao {
    org_id,
    org_nome[MAX_ORG_NAME],
    org_tag[8],
    E_TipoOrg:org_tipo,
    org_lider[MAX_PLAYER_NAME],
    org_caixa,
    org_rank_nomes[ORG_MAX_RANKS][32],
    org_cor,
    org_label_cor[8]
}

new OrgData[MAX_ORGS][E_Organizacao];
new OrgMembros[MAX_ORGS][ORG_MAX_MEMBROS][MAX_PLAYER_NAME];
new OrgMembrosRank[MAX_ORGS][ORG_MAX_MEMBROS];
```

**Funções principais:**
```pawn
// Convida jogador para organização (dialog de confirmação)
Org_Convidar(playerid, alvo, org_id);

// Expulsa membro da organização
Org_Expulsar(playerid, alvo_nome[], org_id);

// Promove ou rebaixa membro (verifica hierarquia de rank)
Org_AlterarRank(playerid, alvo_nome[], org_id, delta); // delta: +1 ou -1

// Deposita no caixa da organização
Org_DepositarCaixa(playerid, org_id, valor);

// Captura território inimigo; atualiza GangZone; notifica organizações
Org_CapturarTerritorio(org_id_atacante, territorio_id);

// Credita percentual de vendas ilegais no caixa da org (Req 21.4)
Org_CreditarVendaIlegal(org_id, valor_venda, Float:percentual);

// Salva dados da organização
Org_Salvar(org_id);
```

**Caminho do arquivo:** `scriptfiles/Organizacoes/Org%d.ini`

---

### `sistema_policial.inc`

**Responsabilidade:** Ocorrências, evidências, algemas, ficha criminal, mandados, prisão.

**Estrutura de dados:**
```pawn
#define POL_MAX_OCORRENCIAS     200
#define POL_SECS_POR_ESTRELA    30

enum E_StatusOcorrencia {
    OC_EM_ANDAMENTO,
    OC_ENCERRADA,
    OC_ARQUIVADA
}

enum E_Ocorrencia {
    oc_numero,
    oc_descricao[128],
    oc_policial[MAX_PLAYER_NAME],
    oc_suspeitos[5][MAX_PLAYER_NAME],
    oc_evidencias[10][64],
    E_StatusOcorrencia:oc_status,
    oc_timestamp
}

new OcorrenciaData[POL_MAX_OCORRENCIAS][E_Ocorrencia];
new OcorrenciaAtual[MAX_PLAYERS]; // índice da ocorrência ativa por policial
```

**Funções principais:**
```pawn
// Cria nova ocorrência com número único e policial responsável
Policial_CriarOcorrencia(playerid, const descricao[]);

// Adiciona evidência à ocorrência ativa do policial; remove item do mundo
Policial_ColetarEvidencia(playerid, const descricao_evidencia[]);

// Algema ou desgalma suspeito (raio 2m)
Policial_AlternarAlgemas(playerid, alvo);

// Prende suspeito algemado; calcula tempo = estrelas × 30s; teleporta para prisão
Policial_Prender(playerid, alvo);

// Calcula tempo de prisão em segundos
Policial_CalcularTempoPrisao(estrelas);

// Emite multa ao jogador; registra na ficha criminal
Policial_Multar(playerid, alvo, infracao_id);

// Exibe ficha criminal do jogador alvo
Policial_ExibirFicha(playerid, alvo_nome[]);

// Emite mandado contra jogador (somente rank máximo / juiz)
Policial_EmitirMandado(playerid, alvo_nome[], tipo, const motivo[]);

// Incrementa nível de procurado (máx 6 estrelas)
Policial_AdicionarWanted(playerid, alvo);

// Decai wanted após 30 min sem prisão (timer por jogador)
Policial_DecairWanted(playerid);

// Revela inventário do suspeito algemado; permite apreensão
Policial_Revistar(playerid, alvo);
```

---

### `sistema_cidade.inc`

**Responsabilidade:** Transporte público NPC, clima dinâmico, ciclo dia/noite, eventos urbanos, NPCs estáticos.

**Funções principais:**
```pawn
// Inicializa NPCs de ônibus e helicóptero (wraps sistema_botsrck)
Cidade_IniciarTransportePublico();

// Atualiza clima; transições graduais entre ENSOLARADO→NUBLADO→CHUVOSO→TEMPESTADE
Cidade_AtualizarClima();

// Aplica efeitos de tempestade: visibilidade reduzida, +20% consumo combustível
Cidade_AplicarEfeitosClima();

// Avança hora do servidor (1h real = 1 dia de jogo)
Cidade_AtualizarHora();

// Exibe blips de estabelecimentos abertos baseado no horário
Cidade_AtualizarBlips();

// Exibe mensagem automática temática para todos os jogadores
Cidade_EnviarMensagemAutomatica();

// Gera evento de "incidente de segurança" em território controlado por 24h+
Cidade_GerarEventoTerritorio(territorio_id);

// Responde ao comando /falar em NPC estático; exibe informações de serviços locais
Cidade_RespostaNCPEstatico(playerid, npc_id);
```

**Timers:**
- `TIMER_CLIMA` — a cada 1.800.000 ms (30 min): chama `Cidade_AtualizarClima`.
- `TIMER_HORA` — a cada 60.000 ms (60s real = ~1h de jogo): chama `Cidade_AtualizarHora`.
- `TIMER_MSG_AUTO` — a cada 900.000 ms (15 min): chama `Cidade_EnviarMensagemAutomatica`.
- `TIMER_PASSAGEM_ONIBUS` — a cada 2.000 ms: verifica jogadores em raio de parada de ônibus; cobra R$5.

---

### `sistema_admin.inc`

**Responsabilidade:** Comandos staff, VIP, logs administrativos persistentes.

**Funções principais:**
```pawn
// Verifica se jogador tem nível admin suficiente para a ação
// Retorna 1 se permitido, 0 se não
Admin_TemPermissao(playerid, nivel_requerido);

// Executa ban; registra log com IP, executor, alvo, motivo, timestamp
Admin_Banir(playerid, alvo_nome[], const motivo[]);

// Executa kick; registra log
Admin_Expulsar(playerid, alvo_nome[], const motivo[]);

// Silencia jogador por N minutos em todos os canais
Admin_Mutar(playerid, alvo, minutos);

// Teleporta admin para posição do jogador alvo
Admin_Goto(playerid, alvo);

// Entra em modo espectador invisível no alvo
Admin_Spectate(playerid, alvo);

// Exibe anúncio global formatado (cor destaque)
Admin_Anuncio(playerid, const texto[]);

// Define nível VIP de um jogador; notifica; registra log
Admin_DefinirVIP(playerid, alvo_nome[], nivel_vip);

// Define nível admin de um jogador (somente Owner)
Admin_DefinirAdmin(playerid, alvo_nome[], nivel);

// Registra ação administrativa no log persistente
Admin_RegistrarLog(playerid, const acao[], const alvo[], const params[]);

// Notifica jogador 3 dias antes do vencimento do VIP; remove no vencimento
Admin_VerificarVIPExpirado(playerid);
```

**Caminho do arquivo de log:** `scriptfiles/Logs/Admin.ini`

---

## Data Models

### Enum principal do jogador (`pInfo`)

```pawn
enum E_PlayerInfo {
    // Autenticação
    pi_senha[24],
    pi_email[64],
    pi_tentativas_login,

    // Personagem
    pi_nivel,
    pi_xp,
    pi_reputacao,
    pi_skin,
    pi_sexo,                  // 0=masculino, 1=feminino
    pi_idade,
    pi_origem[32],
    pi_data_criacao[20],
    pi_ultimo_login[20],
    pi_horas_jogadas,
    pi_mortes,
    pi_kills,

    // Financeiro
    pi_dinheiro,              // Máx R$50.000
    pi_banco,
    pi_divida_hospital,

    // Status físico
    Float:pi_hp,
    Float:pi_colete,
    pi_fome,                  // 0–100
    pi_sede,                  // 0–100

    // Localização persistida
    Float:pi_pos_x,
    Float:pi_pos_y,
    Float:pi_pos_z,
    Float:pi_angulo,
    pi_interior,
    pi_virtual_world,

    // Progressão
    E_Profissao:pi_profissao,

    // Organização
    pi_org_id,                // -1 se sem org
    pi_org_rank,

    // Empresa
    pi_emp_id,                // -1 se sem empresa
    pi_emp_cargo,

    // Casa
    pi_casa_id,               // -1 se sem casa

    // Admin / VIP
    pi_nivel_admin,
    pi_nivel_vip,
    pi_vip_expira,            // timestamp Unix

    // Estado de sessão (não persistido em arquivo)
    bool:pi_logado,
    bool:pi_incapacitado,
    bool:pi_algemado,
    bool:pi_preso,
    pi_timer_incapacitado,
    pi_timer_regen_casa,
    pi_timer_prisao,
    pi_wanted,                // 0–6 estrelas

    // PIN bancário
    pi_pin[5],

    // Celular
    pi_numero_celular[12],
    bool:pi_celular_aberto,

    // Mutado
    bool:pi_mutado,
    pi_mute_expira             // timestamp Unix
}

new pInfo[MAX_PLAYERS][E_PlayerInfo];
```

---

## Convenções de I/O de Arquivos

### Padrões de caminho

| Sistema         | Padrão de caminho                                      |
|-----------------|--------------------------------------------------------|
| Jogador         | `scriptfiles/Players/%s.ini`                           |
| Casa            | `scriptfiles/Casas/%d.ini`                             |
| Empresa         | `scriptfiles/Empresas/%d.ini`                          |
| Organização     | `scriptfiles/Organizacoes/Org%d.ini`                   |
| Veículo (dono)  | `scriptfiles/Veiculos/%s.ini`                          |
| Log Admin       | `scriptfiles/Logs/Admin.ini`                           |
| Log Moderação   | `scriptfiles/Logs/Moderacao.ini`                       |
| Config servidor | `scriptfiles/Config/Servidor.ini`                      |
| Banidos         | `scriptfiles/Banidos/%s.ini`                           |

### Convenção de funções de arquivo

Todos os módulos seguem o padrão **abrir → ler/escrever → fechar** usando DOF2:

```pawn
// Exemplo de salvamento (sistema_personagem.inc)
Personagem_Salvar(playerid) {
    new caminho[64], nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, sizeof(nome));
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    DOF2_SetString(caminho, "Senha",       pInfo[playerid][pi_senha]);
    DOF2_SetInt   (caminho, "Nivel",       pInfo[playerid][pi_nivel]);
    DOF2_SetInt   (caminho, "XP",          pInfo[playerid][pi_xp]);
    DOF2_SetInt   (caminho, "Dinheiro",    pInfo[playerid][pi_dinheiro]);
    DOF2_SetInt   (caminho, "Banco",       pInfo[playerid][pi_banco]);
    DOF2_SetFloat (caminho, "PosX",        pInfo[playerid][pi_pos_x]);
    // ... demais campos ...
    DOF2_SaveFile(caminho);
    return 1;
}

// Exemplo de carregamento
Personagem_Carregar(playerid) {
    new caminho[64], nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, sizeof(nome));
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    if(!DOF2_FileExists(caminho)) return 0; // Novo jogador

    pInfo[playerid][pi_nivel]    = DOF2_GetInt(caminho, "Nivel");
    pInfo[playerid][pi_xp]       = DOF2_GetInt(caminho, "XP");
    pInfo[playerid][pi_dinheiro] = DOF2_GetInt(caminho, "Dinheiro");
    pInfo[playerid][pi_banco]    = DOF2_GetInt(caminho, "Banco");
    DOF2_GetString(caminho, "Senha", pInfo[playerid][pi_senha], 24);
    // ... demais campos ...
    return 1;
}
```

### Verificação de existência de arquivo

```pawn
// Verifica se conta já existe (Login)
Login_ContaExiste(playerid) {
    new caminho[64], nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, sizeof(nome));
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);
    return DOF2_FileExists(caminho);
}
```

---

## Arquitetura de Timers

| Timer | Intervalo | Módulo | Ação |
|---|---|---|---|
| `TIMER_AUTOSAVE` | 300.000 ms (5 min) | sistema_personagem | Salva todos os jogadores conectados |
| `TIMER_FOME_SEDE` | 600.000 ms (10 min) | sistema_personagem | Decai fome –5 e sede –7; aplica dano se < 20 |
| `TIMER_HUD` | 1.000 ms (1 seg) | sistema_personagem | Atualiza textdraws de HUD |
| `TIMER_INCAPACITADO` | 300.000 ms (5 min) | sistema_personagem | Por jogador: dispara morte definitiva |
| `TIMER_REGEN_CASA` | 60.000 ms (1 min) | sistema_casa | Por jogador: regen +10 HP se dentro de casa própria |
| `TIMER_SALARIOS` | 3.600.000 ms (1h real) | sistema_economia | Ciclo de salários para todos conectados |
| `TIMER_IMPOSTOS` | 2.592.000.000 ms (30 dias) | sistema_economia | Cobrança de imposto sobre patrimônio |
| `TIMER_WANTED_DECAY` | 1.800.000 ms (30 min) | sistema_policial | Por jogador: reduz wanted em –1 se > 0 |
| `TIMER_VEICULO_SAVE` | 300.000 ms (5 min) | sistema_veiculo | Salva estado de todos os veículos spawnados |
| `TIMER_CLIMA` | 1.800.000 ms (30 min) | sistema_cidade | Atualiza clima e aplica efeitos |
| `TIMER_HORA` | 60.000 ms (1 min real) | sistema_cidade | Avança hora do servidor |
| `TIMER_MSG_AUTO` | 900.000 ms (15 min) | sistema_cidade | Mensagem automática global |
| `TIMER_PASSAGEM_ONIBUS` | 2.000 ms | sistema_cidade | Verifica embarque em ponto de ônibus |
| `TIMER_EMPRESA_INATIVIDADE` | 86.400.000 ms (1 dia) | sistema_empresa | Verifica empresas sem dono há 30+ dias |
| `TIMER_VIP_CHECK` | 86.400.000 ms (1 dia) | sistema_admin | Verifica VIPs expirados / prestes a expirar |
| `TIMER_MUTE_CHECK` | 60.000 ms (1 min) | sistema_admin | Dessilencia jogadores com mute expirado |

---

## Roteamento de Comandos (pawncmd / zcmd)

Todos os comandos são definidos com a macro `CMD:` (equivalente `ZCMD`) nos respectivos módulos `.inc`. O arquivo `ARP.pwn` não define comandos diretamente — cada módulo é responsável pelos seus.

**Convenções:**
- Comandos de administrador verificam `pInfo[playerid][pi_nivel_admin]` no início da função.
- Parsing de argumentos usa `sscanf` com `"i"` (inteiro), `"s"` (string), `"f"` (float), `"r"` (restante da string).
- Resposta de erro padrão: `SendClientMessage(playerid, COR_ERRO, "...")`.

**Mapeamento de comandos por módulo:**

| Módulo | Comandos |
|---|---|
| sistema_login | *(sem comandos — gerenciado via callbacks `OnPlayerConnect` / dialogs)* |
| sistema_personagem | `/perfil`, `/aparencia`, `/stats`, `/socorro` |
| sistema_progressao | `/profissao`, `/conquistas` |
| sistema_banco | *(ATM via pickup — sem comando direto)* |
| sistema_inventario | `/inventario`, `/descartar`, `/usar` |
| sistema_casa | `/comprarCasa`, `/venderCasa`, `/trancarcasa`, `/destrancarCasa`, `/informacoes` |
| sistema_empresa | `/empresa`, `/comprar` |
| sistema_veiculo | `/ligarCarro`, `/trancarveiculo`, `/guardarVeiculo`, `/pegarVeiculo`, `/transferirVeiculo`, `/abastecer` |
| sistema_celular | `/celular` |
| sistema_chat | `/s`, `/me`, `/do`, `/ame`, `/f`, `/r`, `/g`, `/reportagem` |
| sistema_organizacao | `/convidar`, `/expulsar`, `/promover`, `/rebaixar`, `/caixafaccao`, `/membros`, `/operacao` |
| sistema_policial | `/ocorrencia`, `/evidencia`, `/algemar`, `/prender`, `/multar`, `/ficha`, `/mandado`, `/wanted`, `/revista` |
| sistema_cidade | `/falar` |
| sistema_admin | `/ban`, `/kick`, `/mute`, `/goto`, `/spectate`, `/announce`, `/setadmin`, `/setvip`, `/admins` |

---

## Integração VoIP (SampVoice)

A integração com o plugin `sampvoice.inc` é encapsulada em `sistema_voip.inc` e usada por `sistema_celular.inc` (chamadas) e `sistema_organizacao.inc` + `sistema_chat.inc` (rádio de facção).

### Tipos de Stream

| Stream | Tipo SampVoice | Criado em | Destruído em | Alcance |
|---|---|---|---|---|
| Chat local por voz | `SV_CreateLocalStream` | `OnPlayerConnect` | `OnPlayerDisconnect` | 15m automático |
| Rádio de facção | `SV_CreateStaticStream` | Inicialização do servidor | Nunca (por-org) | Ilimitado (facção) |
| Chamada privada | `SV_CreateStaticStream` | `Celular_IniciarChamada` | `Celular_EncerrarChamada` | Privado (2 membros) |

### Callbacks SampVoice relevantes

```pawn
public SV_CONNECT_CB(playerid)    → VoIP_CriarStreamLocal(playerid)
public SV_DISCONNECT_CB(playerid) → VoIP_DestruirStreamLocal(playerid)
```

---

## Integração Discord (discord-connector)

O módulo `sistema_admin.inc` usa o plugin `discord-connector.inc` para espelhar ações críticas em um canal de logs do Discord.

**Eventos espelhados:**
- Bans e kicks (canal `#logs-admin`)
- Level up de jogadores (canal `#eventos-servidor`)
- Captura de território (canal `#guerras`)
- Anúncios de `/announce` (canal `#anuncios`)

**Funções:**
```pawn
// Envia embed de log para o canal Discord configurado
Discord_LogAdmin(const acao[], const executor[], const alvo[], const detalhes[]);

// Envia notificação de evento de servidor
Discord_LogEvento(const titulo[], const descricao[]);
```

**Inicialização em `ARP.pwn`:**
```pawn
public OnDCConnect(botid) {
    DC_SendChannelMessage(botid, LOG_CHANNEL_ID, "✅ Servidor Aurora Roleplay online.");
}
```

---

## Padrões de Comunicação entre Sistemas

A cadeia econômica central é implementada pelos seguintes padrões:

### 1. Operação Atômica Econômica

Toda transação financeira segue o padrão:

```pawn
// CORRETO — usa sistema_economia como intermediário
Veiculo_Comprar(playerid, modelo) {
    new preco = Veiculo_PrecoConcessionaria(modelo);
    new Float:desconto = Progressao_DescontoConcessionaria(playerid);
    new preco_final = floatround(preco * (1.0 - desconto));

    if(!Economia_DebitarBanco(playerid, preco_final)) {
        return SendClientMessage(playerid, COR_ERRO, "Saldo bancário insuficiente.");
    }
    // ... restante da compra ...
}

// INCORRETO — nunca alterar pInfo[pi_banco] diretamente
pInfo[playerid][pi_banco] -= preco; // PROIBIDO fora de sistema_economia.inc
```

### 2. Salvamento Reativo

Módulos de dados persistidos (casa, empresa, org, veículo) salvam imediatamente em qualquer mutação de estado crítica:

```pawn
// Após troca de dono da casa
Casa_Comprar(playerid, casa_id) {
    // ... lógica ...
    Casa_Salvar(casa_id); // Salvamento imediato após mutação
}
```

### 3. Notificação Cross-Sistema

Quando um sistema precisa notificar outro, chama diretamente a função pública do módulo destino:

```pawn
// Mecânico conclui serviço → 3 sistemas são acionados simultaneamente
Profissao_ConcluirServico_Mecanico(mecanico, dono_veiculo, veh_index) {
    new taxa = Veiculo_TaxaReparo(veh_index);
    Economia_Transferir(dono_veiculo, mecanico, taxa);   // sistema_economia
    Progressao_AdicionarXP(mecanico, XP_REPARO);          // sistema_progressao
    Veiculo_Reparar(veh_index);                           // sistema_veiculo
}
```

---

## Correctness Properties

*Uma propriedade é uma característica ou comportamento que deve ser verdadeiro em toda execução válida do sistema — essencialmente, uma declaração formal do que o sistema deve fazer. As propriedades servem como ponte entre especificações legíveis por humanos e garantias de correção verificáveis por máquinas.*

---

### Property 1: Senha armazenada preserva hash MD5 no ciclo de persistência

*Para qualquer* senha válida (6 a 20 caracteres), o hash MD5 computado no momento do registro deve ser idêntico ao hash carregado do arquivo `.ini` após uma operação de salvar e carregar.

**Validates: Requirements 1.2**

---

### Property 2: Validação de comprimento de senha

*Para qualquer* string de comprimento N, o sistema de login deve aceitar a senha se e somente se 6 ≤ N ≤ 20. Nenhuma senha com comprimento fora deste intervalo pode resultar em criação de conta.

**Validates: Requirements 1.3**

---

### Property 3: Validação de faixa etária do personagem

*Para qualquer* valor inteiro de idade I, o sistema de criação de personagem deve aceitar a idade se e somente se 18 ≤ I ≤ 70. Qualquer valor fora desta faixa deve ser rejeitado sem alterar o estado do personagem.

**Validates: Requirements 2.6**

---

### Property 4: XP incrementa corretamente por ação elegível

*Para qualquer* jogador com XP inicial X e qualquer ação elegível A com valor de XP definido V(A), após a execução da ação o XP do jogador deve ser exatamente X + V(A) (antes da aplicação de multiplicadores VIP). Com VIP Ouro, deve ser X + floor(V(A) × 1.5).

**Validates: Requirements 3.1, 19.4**

---

### Property 5: Cálculo de patrimônio total é soma dos componentes

*Para qualquer* combinação de valores (dinheiro em mão C, saldo bancário B, soma do valor dos veículos V, soma do valor dos imóveis I), o patrimônio total retornado por `Personagem_Patrimonio` deve ser exatamente C + B + V + I.

**Validates: Requirements 3.5**

---

### Property 6: Invariante do limite de dinheiro em mão

*Para qualquer* sequência de operações financeiras (depósito, saque, salário, transferência recebida, pagamento de serviço), o valor de `pInfo[playerid][pi_dinheiro]` nunca deve exceder `ECO_CASH_MAXIMO` (R$50.000). Esta propriedade deve ser preservada mesmo quando operações tentam ultrapassar o limite — o sistema deve creditar apenas o valor que não excede o teto.

**Validates: Requirements 4.1**

---

### Property 7: Atomicidade do depósito bancário

*Para qualquer* valor de depósito D onde D ≤ dinheiro_em_mão do jogador, após um depósito bem-sucedido:
- `dinheiro_em_mão_após` = `dinheiro_em_mão_antes` − D
- `saldo_banco_após` = `saldo_banco_antes` + D
- A soma total (dinheiro_em_mão + banco) é preservada.

Não pode existir estado intermediário onde D foi deduzido do dinheiro em mão mas ainda não foi creditado no banco, nem o inverso.

**Validates: Requirements 4.3**

---

### Property 8: Atomicidade do saque bancário

*Para qualquer* valor de saque W onde W ≤ saldo_banco e (dinheiro_em_mão + W) ≤ ECO_CASH_MAXIMO, após um saque bem-sucedido:
- `saldo_banco_após` = `saldo_banco_antes` − W
- `dinheiro_em_mão_após` = `dinheiro_em_mão_antes` + W
- A soma total é preservada e o dinheiro em mão não excede R$50.000.

**Validates: Requirements 4.4**

---

### Property 9: Conservação de dinheiro em transferências bancárias

*Para qualquer* transferência T entre remetente R e destinatário D onde T ≤ saldo_banco[R]:
- `saldo_banco[R]_após` = `saldo_banco[R]_antes` − T
- `saldo_banco[D]_após` = `saldo_banco[D]_antes` + T
- O dinheiro total no sistema (soma de todos os saldos) é conservado; nenhum valor é criado ou destruído pela operação de transferência.

**Validates: Requirements 4.6**

---

### Property 10: Invariante de peso do inventário

*Para qualquer* sequência de adições de itens ao inventário de um jogador, o peso total `pPesoTotal[playerid]` nunca deve exceder `Inventario_CapacidadeMaxima(playerid)` (15 kg para jogadores normais, 25 kg para VIP nível 2+). Tentativas de adicionar itens que excederiam a capacidade devem ser rejeitadas sem alterar o estado do inventário.

**Validates: Requirements 5.1, 5.2, 5.8**

---

### Property 11: Cálculo de decaimento de fome e sede

*Para qualquer* valores iniciais de fome F e sede S (0 ≤ F, S ≤ 100), após um ciclo do timer `TIMER_FOME_SEDE`:
- `fome_após` = max(0, F − 5)
- `sede_após` = max(0, S − 7)

A função de decaimento deve ser pura e determinística para quaisquer valores de entrada válidos.

**Validates: Requirements 7.2**

---

### Property 12: Consumo de item limita fome/sede a 100

*Para qualquer* valor de fome atual H (0 ≤ H ≤ 100) e valor nutritivo de um item N (N > 0):
- `fome_após` = min(100, H + N)

Nenhuma sequência de consumo de itens pode resultar em fome ou sede superior a 100.

**Validates: Requirements 7.5, 7.6**

---

### Property 13: Atomicidade da compra de imóvel

*Para qualquer* jogador com saldo bancário B e casa com preço P onde B ≥ P, após uma compra bem-sucedida:
- `banco_após` = `banco_antes` − P
- `casa_dono_após` = nome do jogador comprador
- Se B < P, a operação é rejeitada e nenhum campo é alterado.

**Validates: Requirements 8.2**

---

### Property 14: Conservação de dinheiro no ciclo de salários

*Para qualquer* empresa com caixa C e lista de funcionários com salários {S₁, S₂, ..., Sₙ}, após um ciclo de pagamento:
- A soma total débito_empresa + créditos_funcionários = min(C, Σ Sᵢ).
- Se C < Σ Sᵢ: salários pagos proporcionalmente, caixa vai a zero.
- O dinheiro não é criado nem destruído pelo ciclo de salários.

**Validates: Requirements 9.4**

---

### Property 15: Consumo de combustível proporcional à distância

*Para qualquer* distância percorrida D (metros) com o motor ligado, o combustível consumido deve ser:
- `consumo` = floor(D / 500.0)

Esta é uma função pura e determinística. A distância total percorrida divide-se em blocos de 500m, cada um consumindo exatamente 1 unidade de combustível.

**Validates: Requirements 10.4**

---

### Property 16: Cobertura de seguro veicular é 80% do valor de mercado

*Para qualquer* veículo segurado com valor de mercado V, se o veículo for destruído, o crédito no saldo bancário do proprietário deve ser exatamente `floor(V × 0.80)`. Veículos não segurados não geram crédito.

**Validates: Requirements 10.13**

---

### Property 17: Alcance de mensagens de chat é filtrado por distância

*Para qualquer* par de jogadores com posições P₁ e P₂, e qualquer tipo de mensagem com raio R:
- O jogador P₂ recebe a mensagem se e somente se `distancia(P₁, P₂)` ≤ R.
- Chat local: R = 15.0m; Grito: R = 40.0m; /me /do: R = 10.0m; /ame: R = 20.0m.

**Validates: Requirements 12.1, 12.2, 12.3, 12.4**

---

### Property 18: Cálculo de tempo de prisão

*Para qualquer* nível de procurado E (1 ≤ E ≤ 6), o tempo de prisão calculado por `Policial_CalcularTempoPrisao(E)` deve ser exatamente E × 30 segundos. Esta é uma função pura sem efeitos colaterais.

**Validates: Requirements 14.5**

---

### Property 19: Multiplicador VIP Ouro de XP é 1,5×

*Para qualquer* ganho de XP base V concedido a um jogador com VIP Ouro ativo, o XP efetivamente adicionado deve ser `floor(V × 1.5)`. Para jogadores sem VIP Ouro, o XP adicionado deve ser exatamente V.

**Validates: Requirements 19.4**

---

### Property 20: Round-trip de dados do jogador na desconexão

*Para qualquer* estado de dados de um jogador no momento da desconexão, após o salvamento em arquivo `.ini` e o recarregamento no próximo login, todos os campos persistidos devem ser idênticos ao estado original (exceto campos de sessão como `pi_logado`, `pi_timer_*`).

**Validates: Requirements 20.2**

---

### Property 21: Atomicidade do serviço do mecânico (cadeia econômica)

*Para qualquer* serviço de mecânico com taxa T cobrada do proprietário, após a conclusão:
- `banco[proprietario]_após` = `banco[proprietario]_antes` − T  (via `Economia_Transferir`)
- `banco[mecanico]_após` = `banco[mecanico]_antes` + T
- `xp[mecanico]_após` = `xp[mecanico]_antes` + XP_REPARO
- A soma total de dinheiro no sistema é conservada pela transferência.

**Validates: Requirements 21.3**

---

### Property 22: Formatação de mensagens respeita a paleta de cores

*Para qualquer* mensagem do servidor de tipo T (destaque, erro, confirmação, staff, rádio), a cor hexadecimal da mensagem formatada por `Chat_Formatar(T, ...)` deve corresponder exatamente à cor definida na paleta oficial:
- `CHAT_DESTAQUE` → `0x00C8FFFF`
- `CHAT_ERRO` → `0xFF3B30FF`
- `CHAT_CONFIRMACAO` → `0x32D74BFF`
- `CHAT_STAFF` → `0xA855F7FF`
- `CHAT_RADIO` → `0xFF9500FF`

**Validates: Requirements 22.1, 22.2, 22.3**

---

## Error Handling

### Erros de I/O de Arquivo

```pawn
// Em Personagem_Salvar — padrão de retry
Personagem_Salvar(playerid) {
    // ... tentativa de escrita ...
    if(!DOF2_SaveFile(caminho)) {
        new nome[MAX_PLAYER_NAME];
        GetPlayerName(playerid, nome, sizeof(nome));
        // Registra no log do servidor
        printf("[ERRO-SAVE] Falha ao salvar %s. Tentativa em 30s.", nome);
        // Agenda retry
        SetTimerEx("Personagem_Salvar", 30000, false, "i", playerid);
        return 0;
    }
    return 1;
}
```

### Saldo Insuficiente

Todas as funções de débito em `sistema_economia.inc` retornam 0 em caso de saldo insuficiente. O módulo chamador é responsável por exibir a mensagem de erro ao jogador:

```pawn
if(!Economia_DebitarBanco(playerid, valor)) {
    new str[128];
    format(str, sizeof(str), "{FF3B30}Saldo bancário insuficiente. Saldo atual: R$%d", pInfo[playerid][pi_banco]);
    SendClientMessage(playerid, COR_ERRO, str);
    return 1;
}
```

### Verificação de Raio

Todas as funções que exigem proximidade usam `IsPlayerInRangeOfPoint` com verificação explícita antes de executar a ação:

```pawn
Policial_AlternarAlgemas(playerid, alvo) {
    if(!IsPlayerInRangeOfPoint(playerid, 2.0,
        pInfo[alvo][pi_pos_x], pInfo[alvo][pi_pos_y], pInfo[alvo][pi_pos_z])) {
        SendClientMessage(playerid, COR_ERRO, "O jogador está longe demais.");
        return 0;
    }
    // ...
}
```

### Estado Inválido de Sessão

Operações críticas verificam o estado de sessão antes de executar:

```pawn
// Bloqueia uso do celular quando incapacitado ou algemado
Celular_AbrirMenu(playerid) {
    if(pInfo[playerid][pi_incapacitado] || pInfo[playerid][pi_algemado]) {
        SendClientMessage(playerid, COR_ERRO, "Você não pode usar o celular agora.");
        return 1;
    }
    // ...
}
```

---

## Testing Strategy

### Abordagem Dual

O Aurora Roleplay utiliza duas camadas complementares de testes:

1. **Testes unitários com exemplos**: verificam comportamentos específicos, casos de borda e integrações pontuais.
2. **Testes baseados em propriedades (PBT)**: verificam propriedades universais através de entradas geradas aleatoriamente.

Como o Pawn não possui biblioteca nativa de property-based testing, as propriedades de correção são validadas usando um **harness de testes em linguagem hospedeira** (Python com `hypothesis` ou JavaScript com `fast-check`) que simula a lógica pura das funções Pawn (operações aritméticas, validações, formatação de strings) e verifica as propriedades contra centenas de entradas geradas.

Funções com efeitos colaterais (I/O de arquivo, SampVoice, discord-connector) são testadas com **mocks/stubs** e testes de integração com 1–3 exemplos representativos.

### Biblioteca de PBT

**Python + `hypothesis`** para as propriedades puras. Cada propriedade é implementada como uma função `@given` com geradores de dados aleatórios e executada com mínimo de **100 iterações**.

**Tag de rastreabilidade:** `# Feature: aurora-roleplay-gamemode, Property {N}: {texto_da_propriedade}`

### Cobertura por Categoria

| Categoria | Estratégia | Exemplos de Teste |
|---|---|---|
| Funções puras (cálculos, validações) | Property-based testing (Propriedades 1–22) | `test_invariante_dinheiro_mao`, `test_atomicidade_deposito` |
| Operações de arquivo (.ini) | Property: round-trip (Propriedade 20) | `test_save_load_player_data` |
| Comandos de chat | Property: filtro por distância (Propriedade 17) | `test_chat_local_raio_15m` |
| Timers de sistema | Testes de integração (1–3 exemplos) | `test_timer_fome_decai_10min` |
| Integração SampVoice | Testes de integração com mock | `test_chamada_cria_stream_privado` |
| Integração Discord | Testes de integração com mock | `test_ban_envia_log_discord` |
| Admin commands | Testes de exemplo por nível de permissão | `test_ban_requer_nivel_2` |

### Casos de Borda Prioritários

- **Economia:** depósito que excede exatamente o teto de R$50.000; transferência com saldo exato; imposto quando banco < imposto devido.
- **Inventário:** item com peso exatamente igual à capacidade restante; peso em ponto flutuante com arredondamento.
- **Prisão:** wanted = 0 ao tentar prender; redução de 30% do advogado quando tempo restante < 30%.
- **VoIP:** chamada encerrada por desconexão abrupta do receptor; chamada em jogador já em outra chamada.
- **Salvamento:** `OnGameModeExit` com jogador em estado incapacitado; erro de escrita no retry.
