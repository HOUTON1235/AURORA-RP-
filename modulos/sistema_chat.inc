// =============================================================================
// SISTEMA DE CHAT — Aurora Roleplay 2026
// sistema_chat.inc
// =============================================================================

// Função auxiliar para calcular distância
stock Float:GetDistance(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2)
{
    new Float:dx = x1 - x2;
    new Float:dy = y1 - y2;
    new Float:dz = z1 - z2;
    return floatsqroot((dx*dx) + (dy*dy) + (dz*dz));
}

// ============ FUNÇÕES DE CHAT ============
stock Chat_EnviarMensagem(playerid, tipo, mensagem[])
{
    new cor = COR_BRANCO;
    new prefixo[32];
    
    switch(tipo)
    {
        case 0: // Mensagem normal
        {
            cor = COR_BRANCO;
            strcpy(prefixo, "[CHAT]");
        }
        case 1: // Mensagem local
        {
            cor = 0x90EE90FF; // Verde claro
            strcpy(prefixo, "[LOCAL]");
        }
        case 2: // Grito
        {
            cor = 0xFF6347FF; // Vermelho
            strcpy(prefixo, "[GRITO]");
        }
        case 3: // Admin
        {
            cor = COR_STAFF;
            strcpy(prefixo, "[ADMIN]");
        }
    }
    
    format(string, sizeof(string), "%s %s: %s", prefixo, GetPlayerNameEx(playerid), mensagem);
    SendClientMessageToAll(cor, string);
    return 1;
}

stock GetPlayerNameEx(playerid)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, sizeof(nome));
    return nome;
}

stock Chat_MensagemLocal(playerid, mensagem[])
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    foreach(new i : Player)
    {
        if(!IsPlayerConnected(i)) continue;
        new Float:dist;
        new Float:ix, Float:iy, Float:iz;
        GetPlayerPos(i, ix, iy, iz);
        dist = GetDistance(x, y, z, ix, iy, iz);
        if(dist <= 30.0)
        {
            format(string, sizeof(string), "%s diz: %s", GetPlayerNameEx(playerid), mensagem);
            SendClientMessage(i, 0x90EE90FF, string);
        }
    }
    return 1;
}