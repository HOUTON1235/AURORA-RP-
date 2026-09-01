// =============================================================================
//   sistema_casa.inc — Imóveis, tranca e regeneração
//   Aurora Roleplay 2026
//   Requirements: 8.1–8.9, 20.5
// =============================================================================

#define MAX_CASAS   1000

enum E_Casa
{
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
new gTotalCasas = 0;

// ---------------------------------------------------------------------------
// Carregar todas as casas — chamado em OnGameModeInit
// ---------------------------------------------------------------------------
stock Casa_CarregarTodas()
{
    gTotalCasas = 0;
    for(new id = 0; id < MAX_CASAS; id++)
    {
        new caminho[64];
        format(caminho, sizeof(caminho), "Casas/%d.ini", id);
        if(!DOF2_FileExists(caminho)) continue;

        CasaData[id][casa_id]      = id;
        CasaData[id][casa_preco]   = DOF2_GetInt(caminho, "Preco");
        CasaData[id][casa_trancada] = bool:DOF2_GetInt(caminho, "Trancada");
        CasaData[id][casa_interior] = DOF2_GetInt(caminho, "Interior");
        CasaData[id][casa_comodos]  = DOF2_GetInt(caminho, "Comodos");

        DOF2_GetString(caminho, "Dono", CasaData[id][casa_dono], MAX_PLAYER_NAME);

        CasaData[id][casa_entrada][0] = DOF2_GetFloat(caminho, "EntradaX");
        CasaData[id][casa_entrada][1] = DOF2_GetFloat(caminho, "EntradaY");
        CasaData[id][casa_entrada][2] = DOF2_GetFloat(caminho, "EntradaZ");

        CasaData[id][casa_interior_pos][0] = DOF2_GetFloat(caminho, "IntX");
        CasaData[id][casa_interior_pos][1] = DOF2_GetFloat(caminho, "IntY");
        CasaData[id][casa_interior_pos][2] = DOF2_GetFloat(caminho, "IntZ");

        // Cria pickup e label
        CasaData[id][casa_pickup] = CreatePickup(1273, 1,
            CasaData[id][casa_entrada][0],
            CasaData[id][casa_entrada][1],
            CasaData[id][casa_entrada][2]);

        Casa_AtualizarLabel(id);
        gTotalCasas++;
    }
    printf("[CASAS] %d casas carregadas.", gTotalCasas);
}

// ---------------------------------------------------------------------------
// Salvar casa — Req 20.5
// ---------------------------------------------------------------------------
stock Casa_Salvar(id)
{
    if(id < 0 || id >= MAX_CASAS) return;

    new caminho[64];
    format(caminho, sizeof(caminho), "Casas/%d.ini", id);

    DOF2_SetString(caminho, "Dono",     CasaData[id][casa_dono]);
    DOF2_SetInt   (caminho, "Preco",    CasaData[id][casa_preco]);
    DOF2_SetInt   (caminho, "Trancada", _:CasaData[id][casa_trancada]);
    DOF2_SetInt   (caminho, "Interior", CasaData[id][casa_interior]);
    DOF2_SetInt   (caminho, "Comodos",  CasaData[id][casa_comodos]);
    DOF2_SetFloat (caminho, "EntradaX", CasaData[id][casa_entrada][0]);
    DOF2_SetFloat (caminho, "EntradaY", CasaData[id][casa_entrada][1]);
    DOF2_SetFloat (caminho, "EntradaZ", CasaData[id][casa_entrada][2]);
    DOF2_SetFloat (caminho, "IntX",     CasaData[id][casa_interior_pos][0]);
    DOF2_SetFloat (caminho, "IntY",     CasaData[id][casa_interior_pos][1]);
    DOF2_SetFloat (caminho, "IntZ",     CasaData[id][casa_interior_pos][2]);
    DOF2_SaveFile(caminho);
}

// ---------------------------------------------------------------------------
// Atualizar Label_3D — Req 8.8
// ---------------------------------------------------------------------------
stock Casa_AtualizarLabel(id)
{
    if(CasaData[id][casa_label] != Text3D:INVALID_3DTEXT_ID)
        Delete3DTextLabel(CasaData[id][casa_label]);

    new texto[128];
    if(strcmp(CasaData[id][casa_dono], "Nenhum") == 0 || strlen(CasaData[id][casa_dono]) == 0)
        format(texto, sizeof(texto), "Casa #%d\n[A Venda] R$%d", id, CasaData[id][casa_preco]);
    else
    {
        new trancadoStr[16];
        if(CasaData[id][casa_trancada]) trancadoStr = "[TRANCADA]";
        else                            trancadoStr = "[ABERTA]";

        format(texto, sizeof(texto), "Casa #%d\nDono: %s\n%s",
            id, CasaData[id][casa_dono], trancadoStr);
    }

    CasaData[id][casa_label] = Create3DTextLabel(texto, 0x00C8FFFF,
        CasaData[id][casa_entrada][0],
        CasaData[id][casa_entrada][1],
        CasaData[id][casa_entrada][2] + 1.0,
        15.0, 0);
}

// ---------------------------------------------------------------------------
// Comprar casa — Req 8.2, 8.3
// ---------------------------------------------------------------------------
stock Casa_Comprar(playerid, id)
{
    if(id < 0 || id >= MAX_CASAS)
        return SendClientMessage(playerid, COR_ERRO, "[CASA] Casa invalida."), 0;

    if(strlen(CasaData[id][casa_dono]) > 0 && strcmp(CasaData[id][casa_dono], "Nenhum") != 0)
        return SendClientMessage(playerid, COR_ERRO, "[CASA] Esta casa ja tem dono."), 0;

    new preco = CasaData[id][casa_preco];

    if(!Economia_DebitarBanco(playerid, preco))
    {
        new msg[128];
        format(msg, sizeof(msg), "[CASA] Saldo insuficiente. Faltam R$%d.", preco - pInfo[playerid][pi_banco]);
        SendClientMessage(playerid, COR_ERRO, msg);
        return 0;
    }

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    strcopy(CasaData[id][casa_dono], nome, MAX_PLAYER_NAME);
    pInfo[playerid][pi_casa_id] = id;

    Casa_Salvar(id);
    Casa_AtualizarLabel(id);
    Progressao_RegistrarConquista(playerid, CONQUISTA_PRIMEIRO_IMOVEL);

    new msg[64];
    format(msg, sizeof(msg), "[CASA] Voce comprou a casa #%d por R$%d!", id, preco);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Vender/anunciar casa — Req 8.4
// ---------------------------------------------------------------------------
stock Casa_VenderAnunciar(playerid, id, preco)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    if(strcmp(CasaData[id][casa_dono], nome, true) != 0)
        return SendClientMessage(playerid, COR_ERRO, "[CASA] Voce nao e o dono desta casa."), 0;

    strcopy(CasaData[id][casa_dono], "Nenhum", MAX_PLAYER_NAME);
    CasaData[id][casa_preco]  = preco;
    pInfo[playerid][pi_casa_id] = -1;

    Casa_Salvar(id);
    Casa_AtualizarLabel(id);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[CASA] Casa colocada a venda com sucesso!");
    return 1;
}

// ---------------------------------------------------------------------------
// Alternar tranca — Req 8.5, 8.6
// ---------------------------------------------------------------------------
stock Casa_AlternarTranca(playerid, id)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    if(strcmp(CasaData[id][casa_dono], nome, true) != 0)
        return SendClientMessage(playerid, COR_ERRO, "[CASA] Voce nao e o dono desta casa.");

    CasaData[id][casa_trancada] = !CasaData[id][casa_trancada];
    Casa_Salvar(id);
    Casa_AtualizarLabel(id);

    new msg[64];
    format(msg, sizeof(msg), "[CASA] Casa #%d %s.", id,
        CasaData[id][casa_trancada] ? "trancada" : "destrancada");
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Regeneração de HP dentro da casa — Req 8.7
// ---------------------------------------------------------------------------
stock Casa_AplicarRegen(playerid)
{
    new id = pInfo[playerid][pi_casa_id];
    if(id == -1) return;

    // Verifica se está dentro da casa (interior correto)
    if(GetPlayerInterior(playerid) != CasaData[id][casa_interior]) return;

    Personagem_RegenCasa(playerid);
}

// ---------------------------------------------------------------------------
// Verificar entrada/saída
// ---------------------------------------------------------------------------
stock Casa_VerificarEntrada(playerid, id)
{
    if(id < 0 || id >= MAX_CASAS) return 0;

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    if(CasaData[id][casa_trancada] && strcmp(CasaData[id][casa_dono], nome, true) != 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[CASA] Esta casa esta trancada.");
        return 0;
    }

    // Teleporta para interior
    SetPlayerPos(playerid,
        CasaData[id][casa_interior_pos][0],
        CasaData[id][casa_interior_pos][1],
        CasaData[id][casa_interior_pos][2]);
    SetPlayerInterior(playerid, CasaData[id][casa_interior]);

    // Timer de regen se for o dono
    if(strcmp(CasaData[id][casa_dono], nome, true) == 0)
    {
        if(pInfo[playerid][pi_timer_regen_casa] != 0)
            KillTimer(pInfo[playerid][pi_timer_regen_casa]);
        pInfo[playerid][pi_timer_regen_casa] = SetTimerEx("Casa_TimerRegen", 60000, true, "i", playerid);
    }
    return 1;
}

forward Casa_TimerRegen(playerid);
public Casa_TimerRegen(playerid)
{
    if(IsPlayerConnected(playerid))
        Casa_AplicarRegen(playerid);
}

// ---------------------------------------------------------------------------
// Dialog
// ---------------------------------------------------------------------------
stock Casa_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused dialogid, listitem, inputtext
    if(!response) return 1;
    return 1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:comprarcasa(playerid, params[])
{
    #pragma unused params
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    for(new id = 0; id < MAX_CASAS; id++)
    {
        if(CasaData[id][casa_preco] == 0) continue;
        if(strlen(CasaData[id][casa_dono]) > 0 && strcmp(CasaData[id][casa_dono], "Nenhum") != 0) continue;
        if(!IsPlayerInRangeOfPoint(playerid, 3.0,
            CasaData[id][casa_entrada][0],
            CasaData[id][casa_entrada][1],
            CasaData[id][casa_entrada][2])) continue;

        Casa_Comprar(playerid, id);
        return 1;
    }
    SendClientMessage(playerid, COR_ERRO, "[CASA] Nenhuma casa a venda por perto.");
    return 1;
}

CMD:vendercasa(playerid, params[])
{
    new id = pInfo[playerid][pi_casa_id];
    if(id == -1) return SendClientMessage(playerid, COR_ERRO, "[CASA] Voce nao tem casa."), 1;

    new preco;
    if(sscanf(params, "i", preco) || preco <= 0)
        return SendClientMessage(playerid, COR_ERRO, "Uso: /vendercasa [preco]"), 1;

    Casa_VenderAnunciar(playerid, id, preco);
    return 1;
}

CMD:trancarcasa(playerid, params[])
{
    #pragma unused params
    new id = pInfo[playerid][pi_casa_id];
    if(id == -1) return SendClientMessage(playerid, COR_ERRO, "[CASA] Voce nao tem casa."), 1;
    Casa_AlternarTranca(playerid, id);
    return 1;
}

CMD:destrancarcasa(playerid, params[])
{
    #pragma unused params
    return cmd_trancarcasa(playerid, params);
}

CMD:informacoes(playerid, params[])
{
    #pragma unused params
    new id = pInfo[playerid][pi_casa_id];
    if(id == -1) return SendClientMessage(playerid, COR_ERRO, "[CASA] Voce nao tem casa."), 1;

    new info[256];
    format(info, sizeof(info),
        "Casa #%d\nDono: %s\nPreco de Mercado: R$%d\nComodos: %d\nTrancada: %s",
        id, CasaData[id][casa_dono], CasaData[id][casa_preco], CasaData[id][casa_comodos],
        CasaData[id][casa_trancada] ? "Sim" : "Nao");

    ShowPlayerDialog(playerid, DIALOG_CASA_MENU, DIALOG_STYLE_MSGBOX, "Informacoes da Casa", info, "Fechar", "");
    return 1;
}
