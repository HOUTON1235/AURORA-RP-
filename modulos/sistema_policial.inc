// =============================================================================
//   sistema_policial.inc — Ocorrências, investigação e prisão
//   Aurora Roleplay 2026
//   Requirements: 14.1–14.11, 15.1–15.5
// =============================================================================

#define POL_MAX_OCORRENCIAS     200
#define POL_SECS_POR_ESTRELA    30

enum E_StatusOcorrencia { OC_EM_ANDAMENTO, OC_ENCERRADA, OC_ARQUIVADA }

enum E_Ocorrencia
{
    oc_numero,
    oc_descricao[128],
    oc_policial[MAX_PLAYER_NAME],
    oc_suspeito1[MAX_PLAYER_NAME],
    oc_suspeito2[MAX_PLAYER_NAME],
    oc_evidencia1[64],
    oc_evidencia2[64],
    oc_evidencia3[64],
    E_StatusOcorrencia:oc_status,
    oc_timestamp
}

new OcorrenciaData[POL_MAX_OCORRENCIAS][E_Ocorrencia];
new OcorrenciaAtual[MAX_PLAYERS];
new gProximaOcorrencia = 1;

new pTimerPrisao[MAX_PLAYERS];
new pTempoPrisao[MAX_PLAYERS];

stock Policial_CriarOcorrencia(playerid, const descricao[])
{
    if(pInfo[playerid][pi_profissao] != PROF_POLICIAL)
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Somente policiais podem criar ocorrencias.");
        return;
    }
    new idx = gProximaOcorrencia % POL_MAX_OCORRENCIAS;
    OcorrenciaData[idx][oc_numero]    = gProximaOcorrencia++;
    OcorrenciaData[idx][oc_status]    = OC_EM_ANDAMENTO;
    OcorrenciaData[idx][oc_timestamp] = gettime();
    strcopy(OcorrenciaData[idx][oc_descricao], descricao, 128);
    GetPlayerName(playerid, OcorrenciaData[idx][oc_policial], MAX_PLAYER_NAME);
    OcorrenciaAtual[playerid] = idx;

    new msg[64];
    format(msg, sizeof(msg), "[POL] Ocorrencia #%d criada.", OcorrenciaData[idx][oc_numero]);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

stock Policial_ColetarEvidencia(playerid, const desc[])
{
    new idx = OcorrenciaAtual[playerid];
    if(OcorrenciaData[idx][oc_status] != OC_EM_ANDAMENTO)
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Sem ocorrencia ativa.");
        return;
    }
    if(OcorrenciaData[idx][oc_evidencia1][0] == EOS)
        strcopy(OcorrenciaData[idx][oc_evidencia1], desc, 64);
    else if(OcorrenciaData[idx][oc_evidencia2][0] == EOS)
        strcopy(OcorrenciaData[idx][oc_evidencia2], desc, 64);
    else if(OcorrenciaData[idx][oc_evidencia3][0] == EOS)
        strcopy(OcorrenciaData[idx][oc_evidencia3], desc, 64);
    else
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Limite de evidencias atingido.");
        return;
    }
    SendClientMessage(playerid, COR_CONFIRMACAO, "[POL] Evidencia coletada.");
}

stock Policial_AlternarAlgemas(playerid, alvo)
{
    if(pInfo[playerid][pi_profissao] != PROF_POLICIAL)
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Somente policiais podem algemar.");
        return;
    }
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    if(!IsPlayerInRangeOfPoint(alvo, 2.0, px, py, pz))
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] O jogador esta longe demais.");
        return;
    }
    pInfo[alvo][pi_algemado] = !pInfo[alvo][pi_algemado];
    new nome[MAX_PLAYER_NAME], msg[64];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    if(pInfo[alvo][pi_algemado])
    {
        ApplyAnimation(alvo, "SWORD", "sword_block", 4.0, true, false, false, false, 0);
        format(msg, sizeof(msg), "[POL] %s foi algemado.", nome);
        SendClientMessage(playerid, COR_CONFIRMACAO, msg);
        SendClientMessage(alvo, COR_ERRO, "[POL] Voce foi algemado!");
    }
    else
    {
        ClearAnimations(alvo);
        format(msg, sizeof(msg), "[POL] %s foi desalgemado.", nome);
        SendClientMessage(playerid, COR_CONFIRMACAO, msg);
        SendClientMessage(alvo, COR_CONFIRMACAO, "[POL] Voce foi desalgemado.");
    }
}

stock Policial_CalcularTempoPrisao(estrelas)
{
    return estrelas * POL_SECS_POR_ESTRELA;
}

stock Policial_Prender(playerid, alvo)
{
    if(pInfo[playerid][pi_profissao] != PROF_POLICIAL)
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Somente policiais podem prender.");
        return;
    }
    if(!pInfo[alvo][pi_algemado])
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Algeme o suspeito primeiro.");
        return;
    }
    if(pInfo[alvo][pi_wanted] <= 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[POL] Suspeito sem nivel de procurado.");
        return;
    }
    new tempo = Policial_CalcularTempoPrisao(pInfo[alvo][pi_wanted]);
    pInfo[alvo][pi_preso]    = true;
    pInfo[alvo][pi_wanted]   = 0;
    pInfo[alvo][pi_algemado] = false;
    pTempoPrisao[alvo] = tempo;
    ClearAnimations(alvo);
    SetPlayerPos(alvo, 264.9, 76.6, 1001.0);
    SetPlayerInterior(alvo, 6);
    SetPlayerVirtualWorld(alvo, 0);
    pTimerPrisao[alvo] = SetTimerEx("Policial_TimerPrisao", 1000, true, "i", alvo);
    pInfo[alvo][pi_timer_prisao] = pTimerPrisao[alvo];
    new nome[MAX_PLAYER_NAME], msg[128];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[POL] %s preso por %d segundos.", nome, tempo);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    format(msg, sizeof(msg), "[PRISAO] Voce foi preso por %d segundos.", tempo);
    SendClientMessage(alvo, COR_ERRO, msg);
    Progressao_AdicionarXP(playerid, XP_TRABALHO);
}

forward Policial_TimerPrisao(playerid);
public Policial_TimerPrisao(playerid)
{
    if(!IsPlayerConnected(playerid)) { KillTimer(pTimerPrisao[playerid]); return; }
    pTempoPrisao[playerid]--;
    new msg[64];
    format(msg, sizeof(msg), "~r~PRESO~n~~w~%ds", pTempoPrisao[playerid]);
    GameTextForPlayer(playerid, msg, 1500, 3);
    if(pTempoPrisao[playerid] <= 0)
    {
        KillTimer(pTimerPrisao[playerid]);
        pInfo[playerid][pi_preso] = false;
        pInfo[playerid][pi_timer_prisao] = 0;
        SetPlayerPos(playerid, 459.1, -616.8, 16.2);
        SetPlayerInterior(playerid, 0);
        SendClientMessage(playerid, COR_CONFIRMACAO, "[PRISAO] Pena cumprida. Voce foi liberado.");
    }
}

stock Policial_Multar(playerid, alvo, infracao_id)
{
    static const gInfracoes[][32] = {
        "Excesso de velocidade","Fuga policial","Porte ilegal","Agressao",
        "Furto","Trafico","Desacato","Dano patrimonio"
    };
    static const gValores[] = {500, 1000, 2000, 1500, 3000, 5000, 300, 800};
    if(infracao_id < 0 || infracao_id >= 8) { SendClientMessage(playerid, COR_ERRO, "[POL] Infracao invalida."); return; }
    new valor = gValores[infracao_id];
    if(!Economia_RetirarDinheiro(alvo, valor)) Economia_DebitarBanco(alvo, valor);
    new nome[MAX_PLAYER_NAME], msg[128];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[POL] %s multado: %s (R$%d).", nome, gInfracoes[infracao_id], valor);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    format(msg, sizeof(msg), "[POL] Voce foi multado: %s (R$%d).", gInfracoes[infracao_id], valor);
    SendClientMessage(alvo, COR_ERRO, msg);
}

stock Policial_ExibirFicha(playerid, alvo_nome[])
{
    new info[256];
    format(info, sizeof(info), "Ficha Criminal de: %s\nWanted: 0 estrelas\nHistorico: Nenhum", alvo_nome);
    ShowPlayerDialog(playerid, DIALOG_POLICIAL_MENU, DIALOG_STYLE_MSGBOX, "Ficha Criminal", info, "Fechar", "");
}

stock Policial_AdicionarWanted(playerid, alvo)
{
    if(pInfo[alvo][pi_wanted] < 6)
    {
        pInfo[alvo][pi_wanted]++;
        SetPlayerWantedLevel(alvo, pInfo[alvo][pi_wanted]);
        new nome[MAX_PLAYER_NAME], msg[64];
        GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
        format(msg, sizeof(msg), "[POL] %s: wanted nivel %d.", nome, pInfo[alvo][pi_wanted]);
        SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    }
}

stock Policial_DecairWanted(playerid)
{
    if(pInfo[playerid][pi_wanted] > 0)
    {
        pInfo[playerid][pi_wanted]--;
        SetPlayerWantedLevel(playerid, pInfo[playerid][pi_wanted]);
    }
}

stock Policial_Revistar(playerid, alvo)
{
    if(!pInfo[alvo][pi_algemado]) { SendClientMessage(playerid, COR_ERRO, "[POL] Algeme o suspeito primeiro."); return; }
    Inventario_ExibirDialog(alvo);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[POL] Inventario do suspeito exibido.");
}

stock Policial_EmitirMandado(playerid, alvo_nome[], tipo, const motivo[])
{
    new msg[256];
    format(msg, sizeof(msg), "[MANDADO] Tipo %d contra %s — Motivo: %s. Validade: 7 dias.", tipo, alvo_nome, motivo);
    SendClientMessageToAll(COR_ERRO, msg);
}

stock Policial_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused playerid, dialogid, response, listitem, inputtext
    return 1;
}

CMD:ocorrencia(playerid, params[])
{
    new acao[16], descricao[128];
    if(sscanf(params, "s[16]S(sem descricao)[128]", acao, descricao))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /ocorrencia novo [descricao]"), 1;
    if(strcmp(acao, "novo", true) == 0) Policial_CriarOcorrencia(playerid, descricao);
    return 1;
}

CMD:evidencia(playerid, params[])
{
    new acao[16], desc[64];
    if(sscanf(params, "s[16]S(item)[64]", acao, desc))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /evidencia coletar [descricao]"), 1;
    if(strcmp(acao, "coletar", true) == 0) Policial_ColetarEvidencia(playerid, desc);
    return 1;
}

CMD:algemar(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /algemar [nome]"), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, alvoNome, true) == 0) { Policial_AlternarAlgemas(playerid, i); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[POL] Jogador nao encontrado."), 1;
}

CMD:prender(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /prender [nome]"), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, alvoNome, true) == 0) { Policial_Prender(playerid, i); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[POL] Jogador nao encontrado."), 1;
}

CMD:multar(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], infracao;
    if(sscanf(params, "s[24]i", alvoNome, infracao)) return SendClientMessage(playerid, COR_ERRO, "Uso: /multar [nome] [0-7]"), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, alvoNome, true) == 0) { Policial_Multar(playerid, i, infracao); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[POL] Jogador nao encontrado."), 1;
}

CMD:ficha(playerid, params[])
{
    new nome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", nome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /ficha [nome]"), 1;
    Policial_ExibirFicha(playerid, nome);
    return 1;
}

CMD:wanted(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /wanted [nome]"), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, alvoNome, true) == 0) { Policial_AdicionarWanted(playerid, i); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[POL] Jogador nao encontrado."), 1;
}

CMD:revista(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome)) return SendClientMessage(playerid, COR_ERRO, "Uso: /revista [nome]"), 1;
    foreach(new i : Player) { new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME); if(strcmp(n, alvoNome, true) == 0) { Policial_Revistar(playerid, i); return 1; } }
    return SendClientMessage(playerid, COR_ERRO, "[POL] Jogador nao encontrado."), 1;
}

CMD:mandado(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], tipo, motivo[64];
    if(sscanf(params, "s[24]iS(motivo)[64]", alvoNome, tipo, motivo)) return SendClientMessage(playerid, COR_ERRO, "Uso: /mandado [nome] [tipo] [motivo]"), 1;
    Policial_EmitirMandado(playerid, alvoNome, tipo, motivo);
    return 1;
}
