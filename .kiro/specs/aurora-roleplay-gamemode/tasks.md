# Implementation Plan: Aurora Roleplay Gamemode

## Overview

Plano de implementação para a gamemode Aurora Roleplay (ARP) em Pawn/SA-MP. Os módulos são implementados na ordem do grafo de dependências definido no design, começando pelo núcleo econômico e infraestrutura base, avançando para sistemas de gameplay e finalizando com integração completa.

## Tasks

- [x] 1. Estrutura principal e arquivo ARP.pwn
  - Criar `gamemodes/ARP.pwn` com callbacks principais (`OnGameModeInit`, `OnGameModeExit`, `OnPlayerConnect`, `OnPlayerDisconnect`, `OnPlayerDeath`, `OnVehicleDeath`, `OnDialogResponse`, `OnPlayerUpdate`) e inicialização de timers globais
  - Definir enum `E_PlayerInfo` com todos os campos descritos no design e array global `pInfo[MAX_PLAYERS]`
  - Incluir todos os módulos `.inc` na ordem correta do grafo de dependências
  - _Requirements: 20.3_

- [x] 2. sistema_economia.inc — Núcleo financeiro atômico
  - Implementar `Economia_DarDinheiro` e `Economia_RetirarDinheiro` com invariante `ECO_CASH_MAXIMO` (R$50.000)
  - Implementar `Economia_Depositar` e `Economia_Sacar` como operações atômicas (débito + crédito simultâneo, verificação do teto de cash)
  - Implementar `Economia_Transferir` (banco→banco atômico), `Economia_CreditarBanco` e `Economia_DebitarBanco`
  - Implementar `Economia_RegistrarExtrato` (ring buffer das últimas 50 transações por jogador)
  - Implementar `Economia_CicloSalarios` (timer `TIMER_SALARIOS` 1h real) e `Economia_CicloImpostos` (timer `TIMER_IMPOSTOS` 30 dias)
  - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 4.10, 21.1_

- [x] 3. sistema_login.inc — Registro e autenticação
  - Implementar `Login_ExibirRegistro` e `Login_ExibirLogin` (dialogs SA-MP)
  - Implementar `Login_ProcessarRegistro` com validação de comprimento (6–20 chars), confirmação de senha, e-mail e hash MD5; criar arquivo `.ini` do jogador
  - Implementar `Login_VerificarSenha`, `Login_CarregarDados`, `Login_RegistrarFalha` (expulsão após 5 tentativas) e `Login_ResetarFalhas`
  - Integrar callbacks `OnPlayerConnect` / `OnDialogResponse` para orquestrar fluxo login → criação de personagem
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8_

- [x] 4. sistema_personagem.inc — Perfil, saúde, fome/sede e HUD
  - Implementar `Personagem_IniciarCriacao` (dialogs sequenciais: sexo, idade 18–70, origem) e `Personagem_ValidarIdade`
  - Implementar `Personagem_Salvar` e `Personagem_Carregar` via DOF2 com todos os campos; retry em falha de escrita (30s)
  - Implementar `Personagem_Incapacitar`, `Personagem_MorteDefinitiva` (teleporte ao hospital, taxa R$500, registro de morte) e `Personagem_RegenCasa` (+10 HP/min)
  - Implementar `Personagem_AtualizarHUD` (textdraws: HP, colete, fome, sede, dinheiro, nível) com paleta de cores oficial
  - Implementar `Personagem_ExibirPerfil` (comando `/perfil`) e `Personagem_Patrimonio`
  - Configurar timers `TIMER_FOME_SEDE` (10 min: fome –5, sede –7), `TIMER_AUTOSAVE` (5 min), `TIMER_HUD` (1s) e `TIMER_INCAPACITADO` (5 min/jogador)
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 20.1, 20.2, 20.8, 22.4, 22.6_

- [x] 5. sistema_progressao.inc — XP, nível, profissões e conquistas
  - Implementar `Progressao_AdicionarXP` com multiplicador VIP Ouro (1,5×) e verificação de level up
  - Implementar `Progressao_VerificarLevelUp`, `Progressao_XPParaProximoNivel` e notificação de level up com benefícios desbloqueados
  - Implementar `Progressao_DefinirProfissao` com histórico (timestamp início/fim) e enum `E_Profissao` com 10 profissões
  - Implementar `Progressao_RegistrarConquista` (primeiro veículo, primeiro imóvel, 100h jogadas) com persistência e exibição no perfil
  - Implementar `Progressao_DescontoConcessionaria` (1% por nível acima de 10) e comando `/stats`
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 16.1, 16.2, 21.6_

- [x] 6. sistema_inventario.inc — Itens, peso e categorias
  - Definir enum `E_CategoriaItem`, enum `E_Item` e arrays `pInventario` / `pPesoTotal`
  - Implementar `Inventario_AdicionarItem` (verificação de peso e capacidade VIP2), `Inventario_PodeCaber` e `Inventario_CapacidadeMaxima`
  - Implementar `Inventario_DescartarItem` (remove item, deposita pickup no chão) e `Inventario_UsarItem` (consumíveis: aplica efeito e remove)
  - Implementar `Inventario_ExibirDialog` e comandos `/inventario`, `/descartar`, `/usar`
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8_

- [x] 7. sistema_banco.inc — ATM, extrato e PIN
  - Criar pickups de ATM no mapa e callback de entrada em agência bancária; exibir menu via `Banco_ExibirMenu`
  - Implementar `Banco_VerificarPIN`, `Banco_DefinirPIN` e integrar autenticação PIN nas transferências via celular
  - Implementar `Banco_ExibirExtrato` (últimas 50 transações em Dialog)
  - _Requirements: 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 11.7_

- [x] 8. sistema_casa.inc — Imóveis, tranca e regeneração
  - Definir enum `E_Casa` e array `CasaData[MAX_CASAS]`; implementar `Casa_CarregarTodas` e `Casa_Salvar`
  - Implementar `Casa_Comprar` (débito atômico via `Economia_DebitarBanco`, registro de dono) e `Casa_VenderAnunciar`
  - Implementar `Casa_AlternarTranca`, bloqueio de entrada para não-donos e `Casa_AtualizarLabel` (Label_3D com número, dono/preço)
  - Implementar `Casa_AplicarRegen` (+10 HP/min dentro da casa própria) com timer `TIMER_REGEN_CASA` (1 min/jogador)
  - Implementar comandos `/comprarCasa`, `/venderCasa`, `/trancarcasa`, `/destrancarCasa`, `/informacoes`
  - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 8.8, 8.9, 20.5_

- [x] 9. sistema_empresa.inc — Comércio, estoque e funcionários
  - Definir enums `E_Empresa` e `E_EstoqueItem`; arrays `EmpEstoque` e `EmpFuncionarios`; carga e salvamento via DOF2
  - Implementar `Empresa_ExibirPainel` (tabs: caixa, estoque, funcionários, histórico) e gerenciamento de funcionários via `/empresa`
  - Implementar `Empresa_VenderProduto` (estoque + caixa + inventário do comprador, atômico) e comando `/comprar`
  - Implementar `Empresa_PagarSalarios` (tributos primeiro, salários proporcionais se caixa insuficiente) e `Empresa_VerificarEstoque` (notificação < 10%)
  - Implementar `Empresa_RegistrarHistorico` (últimas 100 transações) e `Empresa_VerificarInatividade` (leilão após 30 dias; timer `TIMER_EMPRESA_INATIVIDADE`)
  - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.9, 9.10, 20.6, 21.2, 21.7_

- [x] 10. sistema_veiculo.inc — Veículos, combustível, garagem e seguro
  - Definir enum `E_Veiculo` e array `VeiculoData`; carga e salvamento (`Veiculos/%s.ini`); timer `TIMER_VEICULO_SAVE` (5 min)
  - Implementar `Veiculo_Comprar` (desconto por nível, débito bancário, geração de placa única via `Veiculo_GerarPlaca`)
  - Implementar `Veiculo_AlternarMotor`, `Veiculo_ConsumirCombustivel` (1 un./500m) e apagamento automático ao zerar combustível
  - Implementar `Veiculo_Abastecer` (posto de gasolina, débito do dinheiro em mão, crédito no caixa do posto se houver dono)
  - Implementar `Veiculo_Guardar` e `Veiculo_Retirar` (garagem, despawn/spawn) e comandos `/guardarVeiculo`, `/pegarVeiculo`
  - Implementar `Veiculo_Transferir`, `Veiculo_ContratarSeguro` (30 dias, débito bancário) e `Veiculo_ProcessarDestruicao` (crédito de 80% se segurado)
  - Implementar dano crítico em colisão (> 80 km/h) e comandos `/ligarCarro`, `/trancarveiculo`, `/transferirVeiculo`
  - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9, 10.10, 10.11, 10.12, 10.13, 20.4, 21.2, 21.6_

- [x] 11. sistema_voip.inc — Streams SampVoice
  - Implementar `VoIP_CriarStreamLocal` e `VoIP_DestruirStreamLocal` (stream local 15m via `SV_CONNECT_CB`)
  - Implementar `VoIP_CriarStreamFaccao`, `VoIP_EntrarFaccao` e `VoIP_SairFaccao` (stream estático por organização com efeito de estática)
  - Implementar `VoIP_CriarStreamChamada` e `VoIP_DestruirStreamChamada` (stream privado dinâmico)
  - _Requirements: 12.6, 12.7_

- [x] 12. sistema_chat.inc — Canais de comunicação textual
  - Implementar `Chat_Local` (raio 15m, degradê de opacidade), `Chat_Grito` (raio 40m, maiúsculas) e `Chat_AcaoRP` (`/me`, `/do`, raio 10m)
  - Implementar `Chat_AmeLabel` (Label_3D temporária 5s, visível a 20m) e `Chat_Global` (`/g`, cooldown 60s/jogador)
  - Implementar `Chat_Fracao` (`/f`) e `Chat_Radio` (`/r`, cor `#FF9500`, membros da org)
  - Implementar `Chat_Filtrar` (palavras proibidas, log em `Logs/Moderacao.ini`) e `Chat_Formatar` com paleta oficial
  - Implementar `Chat_Reportagem` (`/reportagem` para Jornalista, cooldown 10 min)
  - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9, 12.10, 16.7, 22.1, 22.2, 22.3_

- [x] 13. sistema_celular.inc — Apps, SMS, GPS e VoIP privado
  - Implementar `Celular_AbrirMenu` (Dialog com apps) e bloqueio de uso quando incapacitado ou algemado
  - Implementar `Celular_IniciarChamada` e `Celular_EncerrarChamada` com stream VoIP privado via `sistema_voip`
  - Implementar `Celular_EnviarSMS` (entrega imediata se online; persistência para próximo login) e gerenciamento de contatos (até 50)
  - Implementar `Celular_AbrirGPS` (Dialog com categorias, checkpoint no mapa) e `Celular_TransferenciaBancaria` (PIN via `sistema_banco`)
  - Implementar `Celular_PublicarAnuncio` (R$100, mensagem global) e `Celular_SolicitarServico` (listagem de prestadores online + rastreamento)
  - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5, 11.6, 11.7, 11.8, 11.9, 11.10, 11.11, 21.5_

- [x] 14. sistema_organizacao.inc — Facções, organizações criminosas e territórios
  - Definir enums `E_TipoOrg` e `E_Organizacao`; arrays `OrgData`, `OrgMembros`, `OrgMembrosRank`; carga e salvamento (`Organizacoes/Org%d.ini`)
  - Implementar `Org_Convidar` (dialog de confirmação), `Org_Expulsar`, `Org_AlterarRank` (verificação de hierarquia) e `Org_DepositarCaixa`
  - Implementar `Org_CapturarTerritorio` (atualização de GangZone, notificação, histórico) e `Org_CreditarVendaIlegal`
  - Implementar comandos `/convidar`, `/expulsar`, `/promover`, `/rebaixar`, `/caixafaccao`, `/membros`, `/operacao`
  - Integrar tag de organização em Label_3D com cor da facção e garantir salvamento reativo em toda mutação de estado crítica
  - _Requirements: 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.7, 13.8, 13.9, 20.7, 21.4, 22.5_

- [x] 15. sistema_policial.inc — Ocorrências, investigação e prisão
  - Definir enums `E_StatusOcorrencia` e `E_Ocorrencia`; array `OcorrenciaData`; implementar `Policial_CriarOcorrencia` e `Policial_ColetarEvidencia`
  - Implementar `Policial_AlternarAlgemas` (raio 2m), `Policial_Prender` (tempo = estrelas × 30s, teleporte para prisão, ficha criminal) e `Policial_CalcularTempoPrisao`
  - Implementar `Policial_Multar` (Dialog com infrações, débito, ficha), `Policial_ExibirFicha`, `Policial_EmitirMandado` (somente rank máximo/juiz, 7 dias) e `Policial_Revistar`
  - Implementar `Policial_AdicionarWanted`, `Policial_DecairWanted` (timer `TIMER_WANTED_DECAY` 30 min) e lógica de prisão (bloqueio de comandos, timer na tela, devolução de equipamentos)
  - Implementar redução de 30% no tempo de prisão por advogado via celular
  - _Requirements: 14.1, 14.2, 14.3, 14.4, 14.5, 14.6, 14.7, 14.8, 14.9, 14.10, 14.11, 15.1, 15.2, 15.3, 15.4, 15.5_

- [x] 16. sistema_cidade.inc — Transporte, clima, ciclo e eventos
  - Implementar `Cidade_IniciarTransportePublico` (wraps `sistema_botsrck`), timer `TIMER_PASSAGEM_ONIBUS` (2s, R$5) e suporte a motorista de ônibus jogador
  - Implementar `Cidade_AtualizarClima` (timer 30 min, transições graduais) e `Cidade_AplicarEfeitosClima` (visibilidade, +20% consumo combustível em tempestade)
  - Implementar `Cidade_AtualizarHora` (timer 1 min real = 1h de jogo, iluminação) e `Cidade_AtualizarBlips` (estabelecimentos abertos por horário)
  - Implementar `Cidade_EnviarMensagemAutomatica` (timer 15 min) e `Cidade_GerarEventoTerritorio` (blip especial após 24h de controle criminal)
  - Implementar `Cidade_RespostaNPCEstatico` (NPCs em banco, hospital, delegacia respondendo a `/falar`)
  - _Requirements: 17.1, 17.2, 17.3, 17.4, 17.5, 17.6, 17.7, 17.8, 16.6_

- [x] 17. sistema_admin.inc — Administração, VIP e logs
  - Implementar `Admin_TemPermissao` (6 níveis: 1–5 + Owner) e comandos `/ban`, `/kick` com registro em `Logs/Admin.ini`
  - Implementar `Admin_Mutar` (todos os canais, timer `TIMER_MUTE_CHECK` 1 min), `/announce` e `/admins`
  - Implementar `/goto`, `/spectate` (modo espectador invisível) e `Admin_RegistrarLog`
  - Implementar `Admin_DefinirVIP` (`/setvip`, 3 níveis VIP com benefícios progressivos) e `Admin_DefinirAdmin` (`/setadmin`, somente Owner)
  - Implementar `Admin_VerificarVIPExpirado` (notificação 3 dias antes, remoção na expiração; timer `TIMER_VIP_CHECK` diário)
  - Integrar benefícios VIP nos módulos: Bronze (–10% taxa hospital, 5 skins), Prata (inventário 25 kg, slot extra garagem), Ouro (isenção taxas bancárias, XP ×1,5)
  - Integrar espelhamento no Discord via `discord-connector` (bans, kicks, level up, captura de território, `/announce`)
  - _Requirements: 18.1, 18.2, 18.3, 18.4, 18.5, 18.6, 18.7, 18.8, 18.9, 18.10, 19.1, 19.2, 19.3, 19.4, 19.5, 19.6_

- [ ] 18. Integração final e OnGameModeExit
  - Implementar `OnGameModeExit` em `ARP.pwn`: salvar todos os jogadores, veículos, casas, empresas e organizações em sequência
  - Revisar e garantir inicialização de todos os timers globais em `OnGameModeInit` e cancelamento em `OnGameModeExit`
  - Verificar que nenhum módulo altera `pi_dinheiro` ou `pi_banco` fora de `sistema_economia.inc`; corrigir violações
  - Criar diretórios `scriptfiles/Players/`, `scriptfiles/Casas/`, `scriptfiles/Empresas/`, `scriptfiles/Organizacoes/`, `scriptfiles/Veiculos/`, `scriptfiles/Logs/` e `scriptfiles/Config/` se ausentes
  - _Requirements: 20.3, 20.8_

## Task Dependency Graph

```json
{
  "waves": [
    { "wave": 1, "tasks": [1] },
    { "wave": 2, "tasks": [2, 3, 5, 6, 11, 12] },
    { "wave": 3, "tasks": [4, 7, 8, 10] },
    { "wave": 4, "tasks": [9, 13, 14] },
    { "wave": 5, "tasks": [15, 16, 17] },
    { "wave": 6, "tasks": [18] }
  ],
  "dependencies": {
    "1": [],
    "2": ["1"],
    "3": ["1"],
    "4": ["1", "2", "3"],
    "5": ["1", "2"],
    "6": ["1"],
    "7": ["1", "2"],
    "8": ["1", "2"],
    "9": ["1", "2", "6"],
    "10": ["1", "2", "5"],
    "11": ["1"],
    "12": ["1"],
    "13": ["1", "2", "7", "11"],
    "14": ["1", "2", "12"],
    "15": ["1", "2", "6", "14"],
    "16": ["1", "2", "10", "14"],
    "17": ["1", "2", "4", "5", "6", "10"],
    "18": ["2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17"]
  }
}
```

## Notes

- Toda movimentação financeira passa obrigatoriamente por `sistema_economia.inc` — nenhum módulo altera `pInfo[pi_dinheiro]` ou `pInfo[pi_banco]` diretamente.
- Persistência usa DOF2 como biblioteca principal; fallback para Dini onde necessário.
- Comandos usam macro `CMD:` (zcmd/pawncmd); parsing de argumentos usa `sscanf2`.
- Iteração sobre jogadores usa `foreach(new i : Player)`.
- Módulos com dados persistidos (casa, empresa, org, veículo) salvam imediatamente após qualquer mutação de estado crítica (salvamento reativo), além do autosave global de 5 min.
- O arquivo `sistema_botsrck.inc` já existe em `modulos/` e deve ser reutilizado pelo `sistema_cidade.inc` sem modificação.
