// =============================================================================
// SISTEMA DE PROGRESSÃO — Aurora Roleplay 2026
// sistema_progressao.inc
// =============================================================================

stock Progressao_AdicionarXP(playerid, quantidade)
{
    pInfo[playerid][pi_xp] += quantidade;
    new nivelanterior = pInfo[playerid][pi_nivel];
    pInfo[playerid][pi_nivel] = (pInfo[playerid][pi_xp] / 100) + 1;
    
    if(pInfo[playerid][pi_nivel] > nivelanterior)
    {
        format(string, sizeof(string), "[LEVEL UP] Você subiu para o nível %d!", pInfo[playerid][pi_nivel]);
        SendClientMessage(playerid, COR_CONFIRMACAO, string);
    }
    return 1;
}

stock Progressao_AdicionarReputacao(playerid, quantidade)
{
    pInfo[playerid][pi_reputacao] += quantidade;
    if(pInfo[playerid][pi_reputacao] < 0) pInfo[playerid][pi_reputacao] = 0;
    return 1;
}