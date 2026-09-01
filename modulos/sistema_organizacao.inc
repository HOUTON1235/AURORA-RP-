// =============================================================================
//   sistema_organizacao.inc — Facções, organizações criminosas e territórios
//   Aurora Roleplay 2026
//   Requirements: 13.1–13.9, 20.7, 21.4, 22.5
// =============================================================================

#define MAX_ORGS        50
#define ORG_MAX_RANKS   10
#define ORG_MAX_MEMBROS 100
#define MAX_ORG_NAME    50

enum E_TipoOrg { ORG_FACCAO_OFICIAL, ORG_CRIMINAL }

enum E_Organizacao
{
    org_id,
    org_nome[MAX_ORG_NAME],
    org_tag[8],
    E_TipoOrg:org_tipo,
    org_lider[MAX_PLAYER_NAME],
    org_caixa,
    org_cor
}

new OrgData[MAX_ORGS][E_Organizacao];
new OrgMembros[MAX_ORGS][ORG_MAX_MEMBROS][MAX_PLAYER_NAME];
new OrgMembrosRank[MAX_ORGS][ORG_MAX_MEMBROS];

stock Org_Salvar(id)
{
    if(id < 0 || id >= MAX_ORGS) return;
    if(strlen(OrgData[id][org_nome]) == 0) return;
    new caminho[64];
    format(caminho, sizeof(caminho), "Organizacoes/Org%d.ini", id);
    DOF2_SetInt   (caminho, "ID",    id);
    DOF2_SetString(caminho, "Nome",  OrgData[id][org_nome]);
    DOF2_SetString(caminho, "Tag",   OrgData[id][org_tag]);
    DOF2_SetInt   (caminho, "Tipo",  _:OrgData[id][org_tipo]);
    DOF2_SetString(caminho, "Lider", OrgData[id][org_lider]);
    DOF2_SetInt   (caminho, "Caixa", OrgData[id][org_caixa]);
    DOF2_SetInt   (caminho, "Cor",   OrgData[id][org_cor]);
    for(new m = 0; m < ORG_MAX_MEMBROS; m++)
    {
        if(strlen(OrgMembros[id][m]) == 0) continue;
        new chave[16];
        format(chave, sizeof(chave), "Membro%d", m); DOF2_SetString(caminho, chave, OrgMembros[id][m]);
        format(chave, sizeof(chave), "MbRank%d", m); DOF2_SetInt   (caminho, chave, OrgMembrosRank[id][m]);
    }
    DOF2_SaveFile(caminho);
}

stock Org_Carregar(id)
{
    new caminho[64];
    format(caminho, sizeof(caminho), "Organizacoes/Org%d.ini", id);
    if(!DOF2_FileExists(caminho)) return;
    OrgData[id][org_id] = id;
    DOF2_GetString(caminho, "Nome",  OrgData[id][org_nome],  MAX_ORG_NAME);
    DOF2_GetString(caminho, "Tag",   OrgData[id][org_tag],   8);
    DOF2_GetString(caminho, "Lider", OrgData[id][org_lider], MAX_PLAYER_NAME);
    OrgData[id][org_tipo]  = E_TipoOrg:DOF2_GetInt(caminho, "Tipo");
    OrgData[id][org_caixa] = DOF2_GetInt(caminho, "Caixa");
    OrgData[id][org_cor]   = DOF2_GetInt(caminho, "Cor");
    for(new m = 0; m < ORG_MAX_MEMBROS; m++)
    {
        new chave[16];
        format(chave, sizeof(chave), "Membro%d", m); DOF2_GetString(caminho, chave, OrgMembros[id][m], MAX_PLAYER_NAME);
        format(chave, sizeof(chave), "MbRank%d", m); OrgMembrosRank[id][m] = DOF2_GetInt(caminho, chave);
    }
    // Cria stream VoIP apenas se org tem nome válido
    // Stream criado quando jogador entrar na org (VoIP_EntrarFaccao)
    // if(strlen(OrgData[id][org_nome]) > 0)
    //     VoIP_CriarStreamFaccao(id);
}

stock Org_CarregarTodas()
{
    for(new id = 0; id < MAX_ORGS; id++) Org_Carregar(id);
}

static stock Org_IndiceMembro(id, const nome[])
{
    for(new m = 0; m < ORG_MAX_MEMBROS; m++)
        if(strcmp(OrgMembros[id][m], nome, true) == 0) return m;
    return -1;
}

static stock Org_SlotLivreMembro(id)
{
    for(new m = 0; m < ORG_MAX_MEMBROS; m++)
        if(strlen(OrgMembros[id][m]) == 0) return m;
    return -1;
}

stock Org_Convidar(playerid, alvo, org_id)
{
    if(org_id < 0 || org_id >= MAX_ORGS) return;
    if(pInfo[alvo][pi_org_id] != -1) { SendClientMessage(playerid, COR_ERRO, "[ORG] Este jogador ja esta em uma organizacao."); return; }
    new slot = Org_SlotLivreMembro(org_id);
    if(slot == -1) { SendClientMessage(playerid, COR_ERRO, "[ORG] Organizacao cheia."); return; }
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    strcopy(OrgMembros[org_id][slot], nome, MAX_PLAYER_NAME);
    OrgMembrosRank[org_id][slot] = 0;
    pInfo[alvo][pi_org_id]   = org_id;
    pInfo[alvo][pi_org_rank] = 0;
    VoIP_EntrarFaccao(alvo, org_id);
    new msg[128];
    format(msg, sizeof(msg), "[ORG] Voce foi convidado para %s!", OrgData[org_id][org_nome]);
    SendClientMessage(alvo, COR_CONFIRMACAO, msg);
    Org_Salvar(org_id);
}

stock Org_Expulsar(playerid, alvo_nome[], org_id)
{
    new idx = Org_IndiceMembro(org_id, alvo_nome);
    if(idx == -1) { SendClientMessage(playerid, COR_ERRO, "[ORG] Membro nao encontrado."); return; }
    OrgMembros[org_id][idx][0]  = EOS;
    OrgMembrosRank[org_id][idx] = 0;
    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0)
        {
            pInfo[i][pi_org_id]   = -1;
            pInfo[i][pi_org_rank] = 0;
            VoIP_SairFaccao(i, org_id);
            SendClientMessage(i, COR_ERRO, "[ORG] Voce foi expulso da organizacao.");
            break;
        }
    }
    Org_Salvar(org_id);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[ORG] Membro expulso.");
}

stock Org_AlterarRank(playerid, alvo_nome[], org_id, delta)
{
    new idx = Org_IndiceMembro(org_id, alvo_nome);
    if(idx == -1) { SendClientMessage(playerid, COR_ERRO, "[ORG] Membro nao encontrado."); return; }
    new novoRank = OrgMembrosRank[org_id][idx] + delta;
    if(novoRank < 0) novoRank = 0;
    if(novoRank >= ORG_MAX_RANKS) novoRank = ORG_MAX_RANKS - 1;
    OrgMembrosRank[org_id][idx] = novoRank;
    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0) { pInfo[i][pi_org_rank] = novoRank; break; }
    }
    Org_Salvar(org_id);
    new msg[64];
    format(msg, sizeof(msg), "[ORG] Rank de %s: %d.", alvo_nome, novoRank);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

stock Org_DepositarCaixa(playerid, org_id, valor)
{
    if(!Economia_RetirarDinheiro(playerid, valor)) { SendClientMessage(playerid, COR_ERRO, "[ORG] Dinheiro insuficiente."); return; }
    OrgData[org_id][org_caixa] += valor;
    Org_Salvar(org_id);
    new msg[64];
    format(msg, sizeof(msg), "[ORG] R$%d depositado no caixa.", valor);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

stock Org_CapturarTerritorio(org_id_atacante, territorio_id)
{
    new msg[128];
    format(msg, sizeof(msg), "[TERRITORIO] Organizacao #%d capturou o territorio #%d!", org_id_atacante, territorio_id);
    SendClientMessageToAll(COR_ERRO, msg);
}

stock Org_CreditarVendaIlegal(org_id, valor_venda, Float:percentual)
{
    new credito = floatround(float(valor_venda) * percentual, floatround_floor);
    OrgData[org_id][org_caixa] += credito;
    Org_Salvar(org_id);
}

stock Org_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused playerid, dialogid, response, listitem, inputtext
    return 1;
}

CMD:convidar(playerid, params[])
{
    new destNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", destNome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /convidar [nome]"), 1;
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1) return SendClientMessage(playerid, COR_ERRO, "[ORG] Voce nao esta em uma organizacao."), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, destNome, true) == 0) { Org_Convidar(playerid, i, org_id); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[ORG] Jogador nao encontrado."), 1;
}

CMD:expulsar(playerid, params[])
{
    new nome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", nome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /expulsar [nome]"), 1;
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1) return 1;
    Org_Expulsar(playerid, nome, org_id);
    return 1;
}

CMD:promover(playerid, params[])
{
    new nome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", nome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /promover [nome]"), 1;
    Org_AlterarRank(playerid, nome, pInfo[playerid][pi_org_id], 1);
    return 1;
}

CMD:rebaixar(playerid, params[])
{
    new nome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", nome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /rebaixar [nome]"), 1;
    Org_AlterarRank(playerid, nome, pInfo[playerid][pi_org_id], -1);
    return 1;
}

CMD:caixafaccao(playerid, params[])
{
    new valor;
    if(sscanf(params, "i", valor) || valor <= 0) return SendClientMessage(playerid, COR_ERRO, "Uso: /caixafaccao [valor]"), 1;
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1) return SendClientMessage(playerid, COR_ERRO, "[ORG] Voce nao esta em uma organizacao."), 1;
    Org_DepositarCaixa(playerid, org_id, valor);
    return 1;
}

CMD:membros(playerid, params[])
{
    #pragma unused params
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1) return SendClientMessage(playerid, COR_ERRO, "[ORG] Voce nao esta em uma organizacao."), 1;
    new lista[1024], linha[64];
    for(new m = 0; m < ORG_MAX_MEMBROS; m++)
    {
        if(strlen(OrgMembros[org_id][m]) == 0) continue;
        new online = 0;
        foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, OrgMembros[org_id][m], true) == 0) { online = 1; break; } }
        format(linha, sizeof(linha), "%s | Rank %d | %s\n", OrgMembros[org_id][m], OrgMembrosRank[org_id][m], online ? "Online" : "Offline");
        strcat(lista, linha, sizeof(lista));
    }
    if(lista[0] == EOS) strcopy(lista, "Nenhum membro.", sizeof(lista));
    ShowPlayerDialog(playerid, DIALOG_ORG_MENU, DIALOG_STYLE_MSGBOX, "Membros", lista, "Fechar", "");
    return 1;
}

CMD:operacao(playerid, params[])
{
    #pragma unused params
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1 || OrgData[org_id][org_tipo] != ORG_CRIMINAL) return SendClientMessage(playerid, COR_ERRO, "[ORG] Somente orgs criminosas."), 1;
    ShowPlayerDialog(playerid, DIALOG_ORG_MENU + 1, DIALOG_STYLE_LIST, "Operacoes", "Trafico\nRoubo Planejado\nLavagem", "Iniciar", "Cancelar");
    return 1;
}
