// =============================================================================
//   sistema_chat.inc — Canais de comunicação textual
//   Aurora Roleplay 2026
//   Requirements: 12.1–12.10, 16.7, 22.1–22.3
// =============================================================================

// ---------------------------------------------------------------------------
// Constantes de raio
// ---------------------------------------------------------------------------
#define CHAT_RAIO_LOCAL     15.0
#define CHAT_RAIO_GRITO     40.0
#define CHAT_RAIO_ME_DO     10.0
#define CHAT_RAIO_AME       20.0

// ---------------------------------------------------------------------------
// Cooldowns por jogador
// ---------------------------------------------------------------------------
new pCooldownGlobal[MAX_PLAYERS];
new pCooldownReportagem[MAX_PLAYERS];

// ---------------------------------------------------------------------------
// Lista de palavras proibidas
// ---------------------------------------------------------------------------
static const gPalavrasProibidas[][16] = {
    "palavrao1","palavrao2","palavrao3","palavrao4","palavrao5",
    "hack","cheat","exploit","racist","spam"
};

// ---------------------------------------------------------------------------
// Helper: converte string para maiúsculas
// ---------------------------------------------------------------------------
static stock Pawn_ToUpper(str[])
{
    for(new i = 0; str[i] != EOS; i++)
        if(str[i] >= 'a' && str[i] <= 'z')
            str[i] -= 32;
}

// ---------------------------------------------------------------------------
// Chat_Filtrar — Req 12.9
// ---------------------------------------------------------------------------
stock Chat_Filtrar(playerid, const msg[], saida[256])
{
    strcopy(saida, msg, 256);
    new bool:encontrou = false;

    for(new w = 0; w < sizeof(gPalavrasProibidas); w++)
    {
        new pos = 0;
        new palavra[16];
        strcopy(palavra, gPalavrasProibidas[w], 16);
        pos = strfind(saida, palavra, true);
        while(pos != -1)
        {
            new len = strlen(palavra);
            for(new c = pos; c < pos + len && c < 255; c++)
                saida[c] = '*';
            encontrou = true;
            pos = strfind(saida, palavra, true, pos + len);
        }
    }

    if(encontrou)
    {
        new nome[MAX_PLAYER_NAME], log[256];
        GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
        format(log, sizeof(log), "[MODERACAO] %s usou palavra proibida: %s", nome, msg);
        new ts[32];
        format(ts, sizeof(ts), "%d", gettime());
        DOF2_SetString("Logs/Moderacao.ini", ts, log);
        DOF2_SaveFile("Logs/Moderacao.ini");
    }
}

// ---------------------------------------------------------------------------
// Chat_Formatar — Req 22.1–22.3
// ---------------------------------------------------------------------------
// tipos: 0=DESTAQUE, 1=ERRO, 2=CONFIRMACAO, 3=STAFF, 4=RADIO, 5=SERVIDOR
stock Chat_Formatar(tipo, const msg[], saida[256])
{
    switch(tipo)
    {
        case 0: format(saida, 256, "{00C8FF}%s", msg);
        case 1: format(saida, 256, "{FF3B30}%s", msg);
        case 2: format(saida, 256, "{32D74B}%s", msg);
        case 3: format(saida, 256, "{A855F7}%s", msg);
        case 4: format(saida, 256, "{FF9500}%s", msg);
        default: format(saida, 256, "{D0D0D0}%s", msg);
    }
}

// ---------------------------------------------------------------------------
// Chat_Local — Req 12.1
// ---------------------------------------------------------------------------
stock Chat_Local(playerid, const msg[])
{
    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    format(saida, sizeof(saida), "%s diz: %s", nome, filtrado);

    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        new Float:dist = GetPlayerDistanceFromPoint(i, px, py, pz);
        if(dist > CHAT_RAIO_LOCAL) continue;

        new cor;
        if(dist <= 5.0)       cor = 0xFFFFFFFF;
        else if(dist <= 10.0) cor = 0xC8C8C8FF;
        else                  cor = 0x969696FF;

        SendClientMessage(i, cor, saida);
    }
}

// ---------------------------------------------------------------------------
// Chat_Grito — Req 12.2
// ---------------------------------------------------------------------------
stock Chat_Grito(playerid, const msg[])
{
    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256], upper[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);
    strcopy(upper, filtrado, 256);
    Pawn_ToUpper(upper);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    format(saida, sizeof(saida), "**GRITO** %s grita: %s", nome, upper);

    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        if(IsPlayerInRangeOfPoint(i, CHAT_RAIO_GRITO, px, py, pz))
            SendClientMessage(i, 0xFFFF00FF, saida);
    }
}

// ---------------------------------------------------------------------------
// Chat_AcaoRP — Req 12.3
// ---------------------------------------------------------------------------
stock Chat_AcaoRP(playerid, const msg[], tipo)
{
    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);

    if(tipo == 0)
        format(saida, sizeof(saida), "* %s %s", nome, filtrado);
    else
        format(saida, sizeof(saida), "(( %s: %s ))", nome, filtrado);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        if(IsPlayerInRangeOfPoint(i, CHAT_RAIO_ME_DO, px, py, pz))
            SendClientMessage(i, 0xC8A0FFFF, saida);
    }
}

// ---------------------------------------------------------------------------
// Chat_AmeLabel — Req 12.4
// ---------------------------------------------------------------------------
forward Chat_AmeDestruir(Text3D:labelid);
public Chat_AmeDestruir(Text3D:labelid)
{
    Delete3DTextLabel(labelid);
}

stock Chat_AmeLabel(playerid, const msg[])
{
    new saida[64];
    strcopy(saida, msg, 64);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    pz += 1.2; // acima da cabeça

    new Text3D:label = Create3DTextLabel(saida, 0xC8A0FFFF, px, py, pz, CHAT_RAIO_AME, 0);
    SetTimerEx("Chat_AmeDestruir", 5000, false, "i", _:label);
}

// ---------------------------------------------------------------------------
// Chat_Global — Req 12.10
// ---------------------------------------------------------------------------
stock Chat_Global(playerid, const msg[])
{
    new ts = gettime();
    if(ts - pCooldownGlobal[playerid] < 60)
    {
        new restante = 60 - (ts - pCooldownGlobal[playerid]);
        new aviso[64];
        format(aviso, sizeof(aviso), "[CHAT] Aguarde %ds para usar /g novamente.", restante);
        SendClientMessage(playerid, COR_ERRO, aviso);
        return;
    }
    pCooldownGlobal[playerid] = ts;

    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);
    format(saida, sizeof(saida), "[GLOBAL] %s: %s", nome, filtrado);
    SendClientMessageToAll(0xFFFFFFFF, saida);
}

// ---------------------------------------------------------------------------
// Chat_Fracao — Req 12.5
// ---------------------------------------------------------------------------
stock Chat_Fracao(playerid, const msg[])
{
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1)
    {
        SendClientMessage(playerid, COR_ERRO, "[CHAT] Voce nao pertence a nenhuma organizacao.");
        return;
    }

    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);
    format(saida, sizeof(saida), "[FRACAO] %s: %s", nome, filtrado);

    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado] && pInfo[i][pi_org_id] == org_id)
            SendClientMessage(i, COR_RADIO, saida);
    }
}

// ---------------------------------------------------------------------------
// Chat_Radio — Req 12.8
// ---------------------------------------------------------------------------
stock Chat_Radio(playerid, const msg[])
{
    new org_id = pInfo[playerid][pi_org_id];
    if(org_id == -1)
    {
        SendClientMessage(playerid, COR_ERRO, "[RADIO] Voce nao pertence a nenhuma organizacao.");
        return;
    }

    new nome[MAX_PLAYER_NAME], saida[256], filtrado[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    Chat_Filtrar(playerid, msg, filtrado);
    format(saida, sizeof(saida), "[RADIO] %s: %s", nome, filtrado);

    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado] && pInfo[i][pi_org_id] == org_id)
            SendClientMessage(i, COR_RADIO, saida);
    }
}

// ---------------------------------------------------------------------------
// Chat_Reportagem — Req 16.7
// ---------------------------------------------------------------------------
stock Chat_Reportagem(playerid, const texto[])
{
    if(pInfo[playerid][pi_profissao] != PROF_JORNALISTA)
    {
        SendClientMessage(playerid, COR_ERRO, "[REPORTAGEM] Somente jornalistas podem usar este comando.");
        return;
    }

    new ts = gettime();
    if(ts - pCooldownReportagem[playerid] < 600)
    {
        new aviso[64];
        format(aviso, sizeof(aviso), "[REPORTAGEM] Aguarde %ds.", 600 - (ts - pCooldownReportagem[playerid]));
        SendClientMessage(playerid, COR_ERRO, aviso);
        return;
    }
    pCooldownReportagem[playerid] = ts;

    new nome[MAX_PLAYER_NAME], saida[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(saida, sizeof(saida), "[REPORTAGEM] %s: %s", nome, texto);
    SendClientMessageToAll(COR_DESTAQUE, saida);
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:s(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /s [mensagem]"), 1;
    Chat_Grito(playerid, params);
    return 1;
}

CMD:me(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /me [acao]"), 1;
    Chat_AcaoRP(playerid, params, 0);
    return 1;
}

CMD:do(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /do [descricao]"), 1;
    Chat_AcaoRP(playerid, params, 1);
    return 1;
}

CMD:ame(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /ame [acao]"), 1;
    Chat_AmeLabel(playerid, params);
    return 1;
}

CMD:f(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /f [mensagem]"), 1;
    Chat_Fracao(playerid, params);
    return 1;
}

CMD:r(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /r [mensagem]"), 1;
    Chat_Radio(playerid, params);
    return 1;
}

CMD:g(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /g [mensagem]"), 1;
    Chat_Global(playerid, params);
    return 1;
}

CMD:reportagem(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /reportagem [texto]"), 1;
    Chat_Reportagem(playerid, params);
    return 1;
}
