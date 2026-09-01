# AURORA-RP-

Gamemode Aurora Roleplay para SA-MP, escrita em Pawn.

---

## Status de Compilação

O projeto ainda **não compila** (22 erros restantes). Abaixo está o detalhamento completo de cada erro e o que precisa ser corrigido.

---

## Erros de Compilação — Detalhamento

### 1. `sistema_empresa.inc` — erro 007: operator cannot be redefined (linha ~199)
**Causa:** Uso de `!=` ou operador lógico dentro de um contexto de `foreach` que o compilador interpreta como redefinição de operador.  
**Correção necessária:** Revisar o loop `foreach` na função `Empresa_PagarSalarios` e garantir que não há conflito de tags no iterador.

---

### 2. `sistema_cidade.inc` — erro 017: undefined symbol "SetPlayerFog" (linha 71)
**Causa:** `SetPlayerFog` não existe na API SA-MP. Foi usado por engano.  
**Correção necessária:** Substituir por `SetPlayerWeather` ou remover a linha, pois `SetWeather` já cobre o efeito de tempestade.

---

### 3. `sistema_cidade.inc` — erro 001: expected token "-identifier-", but found "new" (linha 98)
**Causa:** Declaração `new` dentro de um `static const` ou contexto inválido.  
**Correção necessária:** Mover a declaração `new gMsgAutoIdx` para fora da `static const` ou transformar em variável global.

---

### 4. `sistema_cidade.inc` — erro 017: undefined symbol "gMsgAutoIdx" (linhas 111–112)
**Causa:** A variável `gMsgAutoIdx` foi declarada como `static` dentro de uma função mas referenciada fora do escopo, ou a declaração foi removida no processo de correção anterior.  
**Correção necessária:** Declarar `new gMsgAutoIdx = 0;` como variável global no topo do arquivo.

---

### 5. `sistema_cidade.inc` — erro 017: undefined symbol "sizeof@YSII_Ag" (linha 131)
**Causa:** Uso de `foreach(new p : 0, sizeof(...))` com sintaxe inválida para o YSI/y_iterate disponível.  
**Correção necessária:** Substituir o `foreach` por um loop `for` simples: `for(new p = 0; p < sizeof(gParadasOnibus); p++)`.

---

### 6. `sistema_admin.inc` — erro 035: argument type mismatch (argument 2) (linhas 259, 268)
**Causa:** `DCC_SendChannelMessage(botid, 0, msg)` — o segundo argumento precisa ser do tipo `DCC_Channel:` e não um inteiro literal `0`.  
**Correção necessária:** Remover as chamadas do Discord ou usar a macro correta do plugin discord-connector para obter o ID do canal.

---

### 7. `sistema_botsrck.inc` — erro 010: invalid function or declaration (linhas 6–41) + erro 021: symbol already defined "ConnectNPC"
**Causa:** O arquivo `sistema_botsrck.inc` usa a sintaxe `CallBack:: NomeFuncao()` que é inválida em Pawn padrão. Além disso, `ConnectNPC` é uma native do SA-MP já definida nos includes.  
**Correção necessária:** Remover o `#include "../modulos/sistema_botsrck.inc"` do `ARP.pwn` (o sistema de bots é gerenciado separadamente como NPC mode) e remover a chamada `Cidade_IniciarTransportePublico()` que depende dele, ou reescrever `sistema_botsrck.inc` com sintaxe Pawn válida.

---

### 8. `ARP.pwn` — erro 017: undefined symbol "BotSpawn" (linha 532)
**Causa:** `BotSpawn()` é chamado em `OnPlayerSpawn` mas não está definido em nenhum módulo.  
**Correção necessária:** Remover a chamada `BotSpawn()` de `OnPlayerSpawn` em `ARP.pwn`, pois os bots NPC são controlados pelo `npcmodes/`, não pelo gamemode principal.

---

### 9. `ARP.pwn` — erro 035: argument type mismatch no `OnDCConnect` (linha 520)
**Causa:** `DCC_SendChannelMessage(botid, 0, "...")` — o canal `0` precisa ser `DCC_Channel:0`.  
**Correção necessária:** Usar `DCC_Channel:0` ou desativar a integração Discord até ter o canal correto configurado.

---

## Avisos relevantes (não bloqueantes mas importantes)

| Arquivo | Aviso | Descrição |
|---|---|---|
| `sistema_veiculo.inc` linha 337 | warning 200 | Nome `Veiculo_ProcessarDestruicao_PorSAMPID` truncado para 31 chars |
| `sistema_celular.inc` | warning 213 | Tag mismatch no stream VoIP — `SV_STREAM:` vs sem tag |
| `sistema_organizacao.inc` | warning 213 | Tag mismatch em `OrgData[id][org_tipo]` — uso de `_:` pode resolver |
| Vários `.inc` | warning 209 | Funções `stock` que usam `return;` (void) mas o compilador espera valor |

---

## Resumo das correções pendentes

1. **`sistema_botsrck.inc`** — remover do include do ARP.pwn (sintaxe incompatível)
2. **`ARP.pwn`** — remover `BotSpawn()` e `DCC_SendChannelMessage` sem tag correta
3. **`sistema_cidade.inc`** — corrigir `SetPlayerFog`, `gMsgAutoIdx` global, loop `for` no lugar de `foreach` com sizeof
4. **`sistema_admin.inc`** — corrigir chamadas Discord com tag `DCC_Channel:`
5. **`sistema_empresa.inc`** — investigar erro 007 no `foreach` de `PagarSalarios`

---

## Estrutura do Projeto

```
gamemodes/ARP.pwn          ← Arquivo principal
modulos/
  sistema_economia.inc     ← Núcleo financeiro atômico
  sistema_login.inc        ← Registro e autenticação
  sistema_personagem.inc   ← Perfil, saúde, HUD
  sistema_progressao.inc   ← XP, nível, profissões
  sistema_inventario.inc   ← Itens e peso
  sistema_banco.inc        ← ATM e extrato
  sistema_casa.inc         ← Imóveis
  sistema_empresa.inc      ← Comércio e estoque
  sistema_veiculo.inc      ← Veículos e garagem
  sistema_celular.inc      ← Apps e VoIP
  sistema_chat.inc         ← Canais de texto
  sistema_voip.inc         ← SampVoice streams
  sistema_organizacao.inc  ← Facções e territórios
  sistema_policial.inc     ← Ocorrências e prisão
  sistema_cidade.inc       ← Clima, transporte, NPCs
  sistema_admin.inc        ← Administração e VIP
scriptfiles/
  Players/                 ← Dados dos jogadores
  Casas/                   ← Dados dos imóveis
  Empresas/                ← Dados das empresas
  Organizacoes/            ← Dados das organizações
  Veiculos/                ← Dados dos veículos
  Logs/                    ← Logs admin e moderação
  Config/                  ← Configurações do servidor
  Banidos/                 ← Lista de banimentos
```
