// =============================================================================
//   sistema_admin.inc — Administração, VIP e logs
//   Aurora Roleplay 2026
//   Requirements: 18.1–18.10, 19.1–19.6
// =============================================================================

// ---------------------------------------------------------------------------
// Verificar permissão — Req 18.1
// ---------------------------------------------------------------------------
stock Admin_TemPermissao(playerid, nivel_requerido)
{
    return (pInfo[playerid][pi_nivel_admin] >= nivel_requerido) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Registrar log — Req 18.7
// ---------------------------------------------------------------------------
stock Admin_RegistrarLog(playerid, const acao[], const alvo[], const params[])
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new entrada[256], chave[32];
    format(entrada, sizeof(entrada), "[%d] Admin %s (Nv%d) | %s | Alvo: %s | Params: %s",
        gettime(), nome, pInfo[playerid][pi_nivel_admin], acao, alvo, params);

    format(chave, sizeof(chave), "Log_%d", gettime());
    DOF2_SetString("Logs/Admin.ini", chave, entrada);
    DOF2_SaveFile("Logs/Admin.ini");
    printf("[ADMIN-LOG] %s", entrada);
}

// ---------------------------------------------------------------------------
// Banir — Req 18.2
// ---------------------------------------------------------------------------
stock Admin_Banir(playerid, alvo_nome[], const motivo[])
{
    if(!Admin_TemPermissao(playerid, 2))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0)
        {
            new ip[16];
            GetPlayerIp(i, ip, sizeof(ip));

            // Salva ban
            new caminho[64];
            format(caminho, sizeof(caminho), "Banidos/%s.ini", alvo_nome);
            DOF2_SetString(caminho, "Nome",    alvo_nome);
            DOF2_SetString(caminho, "IP",      ip);
            DOF2_SetString(caminho, "Motivo",  motivo);
            DOF2_SetInt   (caminho, "Data",    gettime());
            DOF2_SaveFile(caminho);

            new msg[128];
            format(msg, sizeof(msg), "Voce foi banido. Motivo: %s", motivo);
            SendClientMessage(i, COR_ERRO, msg);
            BanEx(i, motivo);

            Admin_RegistrarLog(playerid, "BAN", alvo_nome, motivo);

            // Discord log
            new discMsg[256];
            format(discMsg, sizeof(discMsg), "🔨 BAN | Admin: %s | Alvo: %s | IP: %s | Motivo: %s", "", alvo_nome, ip, motivo);
            Discord_LogAdmin("BAN", "", alvo_nome, discMsg);
            return;
        }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
}

// ---------------------------------------------------------------------------
// Expulsar — Req 18.3
// ---------------------------------------------------------------------------
stock Admin_Expulsar(playerid, alvo_nome[], const motivo[])
{
    if(!Admin_TemPermissao(playerid, 1))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0)
        {
            new msg[128];
            format(msg, sizeof(msg), "Voce foi expulso. Motivo: %s", motivo);
            SendClientMessage(i, COR_ERRO, msg);
            Kick(i);

            Admin_RegistrarLog(playerid, "KICK", alvo_nome, motivo);
            return;
        }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
}

// ---------------------------------------------------------------------------
// Mutar — Req 18.4
// ---------------------------------------------------------------------------
stock Admin_Mutar(playerid, alvo, minutos)
{
    if(!Admin_TemPermissao(playerid, 1))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    pInfo[alvo][pi_mutado]     = true;
    pInfo[alvo][pi_mute_expira] = gettime() + (minutos * 60);

    new nome[MAX_PLAYER_NAME], msg[64];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[ADMIN] %s silenciado por %d minutos.", nome, minutos);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    SendClientMessage(alvo, COR_ERRO, "[ADMIN] Voce foi silenciado pelo staff.");
    Admin_RegistrarLog(playerid, "MUTE", nome, "");
}

// ---------------------------------------------------------------------------
// Teleportar admin — Req 18.5
// ---------------------------------------------------------------------------
stock Admin_Goto(playerid, alvo)
{
    if(!Admin_TemPermissao(playerid, 1))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(alvo, x, y, z);
    SetPlayerPos(playerid, x + 1.0, y, z);
    SetPlayerInterior(playerid, GetPlayerInterior(alvo));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(alvo));

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);
    new msg[64];
    format(msg, sizeof(msg), "[ADMIN] Teleportado para %s.", nome);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Spectate — Req 18.6
// ---------------------------------------------------------------------------
stock Admin_Spectate(playerid, alvo)
{
    if(!Admin_TemPermissao(playerid, 2))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    TogglePlayerSpectating(playerid, 1);
    PlayerSpectatePlayer(playerid, alvo);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[ADMIN] Modo espectador ativado. /sair para sair.");
}

// ---------------------------------------------------------------------------
// Anúncio — Req 18.10
// ---------------------------------------------------------------------------
stock Admin_Anuncio(playerid, const texto[])
{
    if(!Admin_TemPermissao(playerid, 2))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    new nome[MAX_PLAYER_NAME], msg[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[ANUNCIO GLOBAL] %s: %s", nome, texto);
    SendClientMessageToAll(COR_STAFF, msg);
    GameTextForAll(texto, 5000, 3);

    Admin_RegistrarLog(playerid, "ANNOUNCE", "Todos", texto);
    Discord_LogAdmin("ANNOUNCE", nome, "Todos", msg);
}

// ---------------------------------------------------------------------------
// Definir VIP — Req 19.5
// ---------------------------------------------------------------------------
stock Admin_DefinirVIP(playerid, alvo_nome[], nivel_vip)
{
    if(!Admin_TemPermissao(playerid, 3))
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Sem permissao.");

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0)
        {
            pInfo[i][pi_nivel_vip]   = nivel_vip;
            pInfo[i][pi_vip_expira]  = gettime() + (30 * 86400); // 30 dias
            Personagem_Salvar(i);

            new msg[64];
            format(msg, sizeof(msg), "[VIP] Voce recebeu VIP nivel %d! Validade: 30 dias.", nivel_vip);
            SendClientMessage(i, COR_CONFIRMACAO, msg);
            format(msg, sizeof(msg), "[ADMIN] VIP nivel %d definido para %s.", nivel_vip, alvo_nome);
            SendClientMessage(playerid, COR_CONFIRMACAO, msg);
            Admin_RegistrarLog(playerid, "SETVIP", alvo_nome, "");
            return;
        }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
}

// ---------------------------------------------------------------------------
// Definir admin — Req 18.8
// ---------------------------------------------------------------------------
stock Admin_DefinirAdmin(playerid, alvo_nome[], nivel)
{
    if(pInfo[playerid][pi_nivel_admin] < 6) // Owner = nível 6
        return SendClientMessage(playerid, COR_ERRO, "[ADMIN] Apenas o Owner pode definir admins.");

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvo_nome, true) == 0)
        {
            pInfo[i][pi_nivel_admin] = nivel;
            Personagem_Salvar(i);
            new msg[64];
            format(msg, sizeof(msg), "[ADMIN] Admin nivel %d definido para %s.", nivel, alvo_nome);
            SendClientMessage(playerid, COR_CONFIRMACAO, msg);
            Admin_RegistrarLog(playerid, "SETADMIN", alvo_nome, "");
            return;
        }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
}

// ---------------------------------------------------------------------------
// Verificar VIP expirado — Req 19.6
// ---------------------------------------------------------------------------
stock Admin_VerificarVIPExpirado(playerid)
{
    if(pInfo[playerid][pi_nivel_vip] == 0) return;
    if(pInfo[playerid][pi_vip_expira] == 0) return;

    new ts = gettime();
    new restante = pInfo[playerid][pi_vip_expira] - ts;

    if(restante <= 0)
    {
        pInfo[playerid][pi_nivel_vip]  = 0;
        pInfo[playerid][pi_vip_expira] = 0;
        Personagem_Salvar(playerid);
        SendClientMessage(playerid, COR_ERRO, "[VIP] Seu VIP expirou.");
    }
    else if(restante <= 3 * 86400) // 3 dias
    {
        new msg[64];
        format(msg, sizeof(msg), "[VIP] Seu VIP expira em %d dias!", restante / 86400);
        SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    }
}

// ---------------------------------------------------------------------------
// Discord log helpers
// ---------------------------------------------------------------------------
forward Discord_LogAdmin(const acao[], const executor[], const alvo[], const detalhes[]);
public  Discord_LogAdmin(const acao[], const executor[], const alvo[], const detalhes[])
{
    #pragma unused acao, executor, alvo
    // Envia para canal Discord de logs se o plugin estiver conectado
    DCC_SendChannelMessage(0, 0, detalhes);
    return 1;
}

forward Discord_LogEvento(const titulo[], const descricao[]);
public  Discord_LogEvento(const titulo[], const descricao[])
{
    new msg[256];
    format(msg, sizeof(msg), "📢 %s: %s", titulo, descricao);
    DCC_SendChannelMessage(0, 0, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Processar dialog
// ---------------------------------------------------------------------------
stock Admin_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused dialogid, response, listitem, inputtext
    return 1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:ban(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], motivo[64];
    if(sscanf(params, "s[24]S(sem motivo)[64]", alvoNome, motivo))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /ban [nome] [motivo]"), 1;
    Admin_Banir(playerid, alvoNome, motivo);
    return 1;
}

CMD:kick(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], motivo[64];
    if(sscanf(params, "s[24]S(sem motivo)[64]", alvoNome, motivo))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /kick [nome] [motivo]"), 1;
    Admin_Expulsar(playerid, alvoNome, motivo);
    return 1;
}

CMD:mute(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], minutos;
    if(sscanf(params, "s[24]i", alvoNome, minutos))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /mute [nome] [minutos]"), 1;

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvoNome, true) == 0) { Admin_Mutar(playerid, i, minutos); return 1; }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
    return 1;
}

CMD:goto(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /goto [nome]"), 1;

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvoNome, true) == 0) { Admin_Goto(playerid, i); return 1; }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
    return 1;
}

CMD:spectate(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", alvoNome))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /spectate [nome]"), 1;

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, alvoNome, true) == 0) { Admin_Spectate(playerid, i); return 1; }
    }
    SendClientMessage(playerid, COR_ERRO, "[ADMIN] Jogador nao encontrado.");
    return 1;
}

CMD:sair(playerid, params[])
{
    #pragma unused params
    TogglePlayerSpectating(playerid, 0);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[ADMIN] Modo espectador desativado.");
    return 1;
}

CMD:announce(playerid, params[])
{
    if(isnull(params)) return SendClientMessage(playerid, COR_ERRO, "Uso: /announce [texto]"), 1;
    Admin_Anuncio(playerid, params);
    return 1;
}

CMD:setadmin(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], nivel;
    if(sscanf(params, "s[24]i", alvoNome, nivel))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /setadmin [nome] [nivel]"), 1;
    Admin_DefinirAdmin(playerid, alvoNome, nivel);
    return 1;
}

CMD:setvip(playerid, params[])
{
    new alvoNome[MAX_PLAYER_NAME], nivel;
    if(sscanf(params, "s[24]i", alvoNome, nivel))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /setvip [nome] [nivel 1-3]"), 1;
    Admin_DefinirVIP(playerid, alvoNome, nivel);
    return 1;
}

CMD:admins(playerid, params[])
{
    #pragma unused params
    new lista[512], linha[64];
    foreach(new i : Player)
    {
        if(pInfo[i][pi_nivel_admin] <= 0) continue;
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        format(linha, sizeof(linha), "%s — Nivel %d\n", n, pInfo[i][pi_nivel_admin]);
        strcat(lista, linha, sizeof(lista));
    }
    if(lista[0] == EOS) strcopy(lista, "Nenhum admin online.", sizeof(lista));
    ShowPlayerDialog(playerid, DIALOG_ADMIN_MENU, DIALOG_STYLE_MSGBOX, "Admins Online", lista, "Fechar", "");
    return 1;
}
