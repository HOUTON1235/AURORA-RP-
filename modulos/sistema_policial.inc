// =============================================================================
// SISTEMA POLICIAL — Aurora Roleplay 2026
// sistema_policial.inc
// =============================================================================

stock Policial_PrenderJogador(playerid, targetid, minutos)
{
    if(pInfo[playerid][pi_profissao] != PROF_POLICIAL)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Você não é policial.");
        return 0;
    }
    
    pInfo[targetid][pi_preso] = true;
    SetPlayerPos(targetid, 206.4842, 173.7704, 1003.0391); // Interior presídio
    
    format(string, sizeof(string), "[POLÍCIA] %s foi preso por %s.", GetPlayerNameEx(targetid), GetPlayerNameEx(playerid));
    SendClientMessageToAll(0xFF6347FF, string);
    
    return 1;
}

stock Policial_MultarJogador(playerid, targetid, quantia, motivo[])
{
    if(pInfo[playerid][pi_profissao] != PROF_POLICIAL)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Você não é policial.");
        return 0;
    }
    
    Economia_RemoverDinheiro(targetid, quantia);
    
    format(string, sizeof(string), "[MULTA] Você foi multado em R$%d. Motivo: %s", quantia, motivo);
    SendClientMessage(targetid, 0xFF6347FF, string);
    
    format(string, sizeof(string), "[MULTA] Você multou %s em R$%d", GetPlayerNameEx(targetid), quantia);
    SendClientMessage(playerid, COR_CONFIRMACAO, string);
    
    return 1;
}

stock Policial_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    return 1;
}