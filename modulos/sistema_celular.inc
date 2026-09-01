// =============================================================================
// SISTEMA DE CELULAR — Aurora Roleplay 2026
// sistema_celular.inc
// =============================================================================

new pChamadaAlvo[MAX_PLAYERS] = {-1, ...};

stock Celular_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    return 1;
}

stock Celular_EncerrarChamada(playerid)
{
    pChamadaAlvo[playerid] = -1;
    SendClientMessage(playerid, COR_ERRO, "[CELULAR] Chamada encerrada.");
    return 1;
}