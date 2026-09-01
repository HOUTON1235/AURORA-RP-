// =============================================================================
//   sistema_progressao.inc — XP, nível, profissões e conquistas
//   Aurora Roleplay 2026
//   Requirements: 3.1–3.7, 16.1–16.2, 21.6
// =============================================================================

// ---------------------------------------------------------------------------
// Constantes de XP por ação
// ---------------------------------------------------------------------------
#define XP_TRABALHO         50
#define XP_SOCORRER         75
#define XP_REPARO           60
#define XP_CORRIDA_TAXI     40
#define XP_CONQUISTA        200

// ---------------------------------------------------------------------------
// Conquistas (bitmask)
// ---------------------------------------------------------------------------
#define CONQUISTA_PRIMEIRO_VEICULO  (1)
#define CONQUISTA_PRIMEIRO_IMOVEL   (2)
#define CONQUISTA_100H              (4)

// Array de conquistas por jogador (bitmask)
new pConquistas[MAX_PLAYERS];

// ---------------------------------------------------------------------------
// Progressao_XPParaProximoNivel — Req 3.2
// ---------------------------------------------------------------------------
stock Progressao_XPParaProximoNivel(nivel)
{
    return nivel * 1000 + 500;
}

// ---------------------------------------------------------------------------
// Progressao_VerificarLevelUp — Req 3.2
// ---------------------------------------------------------------------------
stock Progressao_VerificarLevelUp(playerid)
{
    new limiar = Progressao_XPParaProximoNivel(pInfo[playerid][pi_nivel]);

    while(pInfo[playerid][pi_xp] >= limiar)
    {
        pInfo[playerid][pi_xp] -= limiar;
        pInfo[playerid][pi_nivel]++;

        new msg[128];
        format(msg, sizeof(msg),
            "[LEVEL UP] Parabens! Voce chegou ao nivel %d! Continue progredindo.",
            pInfo[playerid][pi_nivel]);
        SendClientMessage(playerid, COR_DESTAQUE, msg);

        Progressao_Salvar(playerid);

        limiar = Progressao_XPParaProximoNivel(pInfo[playerid][pi_nivel]);
    }
}

// ---------------------------------------------------------------------------
// Progressao_AdicionarXP — Req 3.1, 19.4
// ---------------------------------------------------------------------------
stock Progressao_AdicionarXP(playerid, quantidade)
{
    new xp = quantidade;

    // Multiplicador VIP Ouro (nível 3) = 1.5x
    if(pInfo[playerid][pi_nivel_vip] == 3)
        xp = floatround(float(quantidade) * 1.5, floatround_floor);

    pInfo[playerid][pi_xp] += xp;
    Progressao_VerificarLevelUp(playerid);

    // Verificar conquista de 100h
    if(pInfo[playerid][pi_horas_jogadas] >= 100)
        Progressao_RegistrarConquista(playerid, CONQUISTA_100H);
}

// ---------------------------------------------------------------------------
// Progressao_DefinirProfissao — Req 3.4, 16.2
// ---------------------------------------------------------------------------
stock Progressao_DefinirProfissao(playerid, E_Profissao:profissao)
{
    pInfo[playerid][pi_profissao] = profissao;

    // Salva histórico em arquivo separado
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s_profissoes.ini", nome);

    new chave[32], ts[16];
    format(ts, sizeof(ts), "%d", gettime());
    format(chave, sizeof(chave), "Prof_%s", ts);
    DOF2_SetInt(caminho, chave, _:profissao);
    DOF2_SaveFile(caminho);

    Progressao_Salvar(playerid);
}

// ---------------------------------------------------------------------------
// Progressao_RegistrarConquista — Req 3.6
// ---------------------------------------------------------------------------
stock Progressao_RegistrarConquista(playerid, tipo)
{
    if(pConquistas[playerid] & tipo) return; // já tem esta conquista

    pConquistas[playerid] |= tipo;
    Progressao_AdicionarXP(playerid, XP_CONQUISTA);

    new msg[128];
    switch(tipo)
    {
        case CONQUISTA_PRIMEIRO_VEICULO:
            format(msg, sizeof(msg), "[CONQUISTA] Primeiro Veiculo desbloqueado!");
        case CONQUISTA_PRIMEIRO_IMOVEL:
            format(msg, sizeof(msg), "[CONQUISTA] Primeiro Imovel desbloqueado!");
        case CONQUISTA_100H:
            format(msg, sizeof(msg), "[CONQUISTA] 100 horas jogadas desbloqueado!");
        default:
            format(msg, sizeof(msg), "[CONQUISTA] Nova conquista desbloqueada!");
    }
    SendClientMessage(playerid, COR_DESTAQUE, msg);
    Progressao_Salvar(playerid);
}

// ---------------------------------------------------------------------------
// Progressao_DescontoConcess — Req 21.6
// ---------------------------------------------------------------------------
stock Float:Progressao_DescontoConcess(playerid)
{
    new nivel = pInfo[playerid][pi_nivel];
    if(nivel <= 10) return 0.0;

    new Float:desconto = float(nivel - 10) * 0.01;
    if(desconto > 0.20) desconto = 0.20;
    return desconto;
}

// ---------------------------------------------------------------------------
// Salvar e carregar
// ---------------------------------------------------------------------------
stock Progressao_Salvar(playerid)
{
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    DOF2_SetInt(caminho, "Nivel",        pInfo[playerid][pi_nivel]);
    DOF2_SetInt(caminho, "XP",           pInfo[playerid][pi_xp]);
    DOF2_SetInt(caminho, "Reputacao",    pInfo[playerid][pi_reputacao]);
    DOF2_SetInt(caminho, "Profissao",    _:pInfo[playerid][pi_profissao]);
    DOF2_SetInt(caminho, "HorasJogadas", pInfo[playerid][pi_horas_jogadas]);
    DOF2_SetInt(caminho, "Mortes",       pInfo[playerid][pi_mortes]);
    DOF2_SetInt(caminho, "Kills",        pInfo[playerid][pi_kills]);
    DOF2_SetInt(caminho, "Conquistas",   pConquistas[playerid]);
    DOF2_SaveFile(caminho);
}

stock Progressao_Carregar(playerid)
{
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    if(!DOF2_FileExists(caminho)) return;

    pInfo[playerid][pi_nivel]        = DOF2_GetInt(caminho, "Nivel");
    pInfo[playerid][pi_xp]           = DOF2_GetInt(caminho, "XP");
    pInfo[playerid][pi_reputacao]    = DOF2_GetInt(caminho, "Reputacao");
    pInfo[playerid][pi_profissao]    = E_Profissao:DOF2_GetInt(caminho, "Profissao");
    pInfo[playerid][pi_horas_jogadas] = DOF2_GetInt(caminho, "HorasJogadas");
    pInfo[playerid][pi_mortes]       = DOF2_GetInt(caminho, "Mortes");
    pInfo[playerid][pi_kills]        = DOF2_GetInt(caminho, "Kills");
    pConquistas[playerid]            = DOF2_GetInt(caminho, "Conquistas");
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:stats(playerid, params[])
{
    #pragma unused params
    new info[512], nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new profNome[32];
    switch(pInfo[playerid][pi_profissao])
    {
        case PROF_DESEMPREGADO:    profNome = "Desempregado";
        case PROF_POLICIAL:        profNome = "Policial";
        case PROF_PARAMEDICO:      profNome = "Paramedico";
        case PROF_BOMBEIRO:        profNome = "Bombeiro";
        case PROF_MECANICO:        profNome = "Mecanico";
        case PROF_TAXISTA:         profNome = "Taxista";
        case PROF_MOTORISTA_ONIBUS: profNome = "Motorista de Onibus";
        case PROF_EMPRESARIO:      profNome = "Empresario";
        case PROF_ADVOGADO:        profNome = "Advogado";
        case PROF_JORNALISTA:      profNome = "Jornalista";
        default:                   profNome = "Desconhecida";
    }

    format(info, sizeof(info),
        "Nome: %s\nNivel: %d\nXP: %d / %d\nReputacao: %d\nProfissao: %s\nHoras: %d\nMortes: %d\nKills: %d\nDinheiro: R$%d\nBanco: R$%d",
        nome,
        pInfo[playerid][pi_nivel],
        pInfo[playerid][pi_xp],
        Progressao_XPParaProximoNivel(pInfo[playerid][pi_nivel]),
        pInfo[playerid][pi_reputacao],
        profNome,
        pInfo[playerid][pi_horas_jogadas],
        pInfo[playerid][pi_mortes],
        pInfo[playerid][pi_kills],
        pInfo[playerid][pi_dinheiro],
        pInfo[playerid][pi_banco]);

    ShowPlayerDialog(playerid, DIALOG_STATS, DIALOG_STYLE_MSGBOX, "Estatisticas", info, "Fechar", "");
    return 1;
}

CMD:profissao(playerid, params[])
{
    #pragma unused params
    new lista[] =
        "Desempregado\nPolicial\nParamedico\nBombeiro\nMecanico\n"
        "Taxista\nMotorista de Onibus\nEmpresario\nAdvogado\nJornalista";

    ShowPlayerDialog(playerid, DIALOG_STATS + 1, DIALOG_STYLE_LIST,
        "Escolha uma Profissao", lista, "Selecionar", "Cancelar");
    return 1;
}
