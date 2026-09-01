// =============================================================================
// SISTEMA DE PERSONAGEM — Aurora Roleplay 2026
// sistema_personagem.inc
// =============================================================================

stock Personagem_Salvar(playerid)
{
    if(!pInfo[playerid][pi_logado]) return 0;
    // Aqui você salvaria em arquivo (DOF2)
    printf("[SAVE] Personagem %d salvo.", playerid);
    return 1;
}

stock Personagem_Carregar(playerid)
{
    // Aqui você carregaria do arquivo (DOF2)
    return 1;
}

stock Personagem_Incapacitar(playerid, attackerid)
{
    pInfo[playerid][pi_incapacitado] = true;
    pInfo[playerid][pi_hp] = 0.0;
    SetPlayerHealth(playerid, 0.1);
    TogglePlayerControllable(playerid, false);
    
    format(string, sizeof(string), "%s foi incapacitado!", GetPlayerNameEx(playerid));
    SendClientMessageToAll(0xFF3B30FF, string);
    
    pInfo[playerid][pi_timer_incapacitado] = SetTimerEx("Timer_RessuscitarJogador", 30000, false, "i", playerid);
    return 1;
}

forward Timer_RessuscitarJogador(playerid);
public Timer_RessuscitarJogador(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    pInfo[playerid][pi_incapacitado] = false;
    SetPlayerHealth(playerid, 100.0);
    TogglePlayerControllable(playerid, true);
    SetPlayerPos(playerid, 2034.0896, 1657.9932, 10.8203); // Hospital LS
    
    SendClientMessage(playerid, COR_CONFIRMACAO, "[HOSPITAL] Você foi ressuscitado!");
    return 1;
}

stock Personagem_AtualizarHUD(playerid)
{
    // Aqui você atualizaria a HUD do jogador (TextDraw)
    return 1;
}