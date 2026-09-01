// =============================================================================
// SISTEMA ADMIN — Aurora Roleplay 2026
// sistema_admin.inc
// =============================================================================

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if(dialogid == 1000) // Dialog de profissão
    {
        if(!response) return 1;
        
        pInfo[playerid][pi_profissao] = E_Profissao:listitem;
        
        new profnome[32];
        switch(listitem)
        {
            case PROF_DESEMPREGADO: strcpy(profnome, "Desempregado");
            case PROF_POLICIAL: strcpy(profnome, "Policial");
            case PROF_PARAMEDICO: strcpy(profnome, "Paramédico");
            case PROF_BOMBEIRO: strcpy(profnome, "Bombeiro");
            case PROF_MECANICO: strcpy(profnome, "Mecânico");
            case PROF_TAXISTA: strcpy(profnome, "Taxista");
            case PROF_MOTORISTA_ONIBUS: strcpy(profnome, "Motorista de Ônibus");
            case PROF_EMPRESARIO: strcpy(profnome, "Empresário");
            case PROF_ADVOGADO: strcpy(profnome, "Advogado");
            case PROF_JORNALISTA: strcpy(profnome, "Jornalista");
        }
        
        format(string, sizeof(string), "[PROFISSÃO] Você é agora um %s!", profnome);
        SendClientMessage(playerid, COR_CONFIRMACAO, string);
    }
    return 1;
}

CMD:help(playerid, params[])
{
    SendClientMessage(playerid, COR_DESTAQUE, "=== Comandos Aurora Roleplay ===");
    SendClientMessage(playerid, COR_BRANCO, "/profissao - Escolher profissão");
    SendClientMessage(playerid, COR_BRANCO, "/banco - Acessar banco");
    SendClientMessage(playerid, COR_BRANCO, "/casa - Gerenciar casa");
    SendClientMessage(playerid, COR_BRANCO, "/inventario - Ver inventário");
    SendClientMessage(playerid, COR_BRANCO, "/dar [playerid] [valor] - Dar dinheiro (Admin)");
    SendClientMessage(playerid, COR_BRANCO, "/prender [playerid] [minutos] - Prender (Policial)");
    SendClientMessage(playerid, COR_BRANCO, "/multar [playerid] [valor] [motivo] - Multar (Policial)");
    return 1;
}

CMD:profissao(playerid, params[])
{
    new profissoes[10][32] = {
        "Desempregado (Sem ganhos)",
        "Policial (R$5000)",
        "Paramédico (R$4500)",
        "Bombeiro (R$4500)",
        "Mecânico (R$3500)",
        "Taxista (R$3000)",
        "Motorista de Ônibus (R$2500)",
        "Empresário (R$6000)",
        "Advogado (R$5500)",
        "Jornalista (R$4000)"
    };
    
    new dialog_list[512];
    for(new i = 0; i < 10; i++)
    {
        strcat(dialog_list, profissoes[i]);
        if(i < 9) strcat(dialog_list, "\n");
    }
    
    ShowPlayerDialog(playerid, 1000, DIALOG_STYLE_LIST, "Escolher Profissão", dialog_list, "Selecionar", "Cancelar");
    return 1;
}

CMD:dar(playerid, params[])
{
    if(pInfo[playerid][pi_nivel_admin] < 1)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Você não é administrador.");
        return 1;
    }
    
    new target, valor;
    if(sscanf(params, "ui", target, valor) != 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Use: /dar [playerid] [valor]");
        return 1;
    }
    
    if(!IsPlayerConnected(target))
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Jogador não conectado.");
        return 1;
    }
    
    Economia_DarDinheiro(target, valor);
    format(string, sizeof(string), "[ADMIN] %s te deu R$%d", GetPlayerNameEx(playerid), valor);
    SendClientMessage(target, COR_CONFIRMACAO, string);
    
    return 1;
}

CMD:prender(playerid, params[])
{
    new target, minutos;
    if(sscanf(params, "ui", target, minutos) != 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Use: /prender [playerid] [minutos]");
        return 1;
    }
    
    if(!IsPlayerConnected(target))
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Jogador não conectado.");
        return 1;
    }
    
    Policial_PrenderJogador(playerid, target, minutos);
    return 1;
}

CMD:multar(playerid, params[])
{
    new target, valor;
    new motivo[128];
    if(sscanf(params, "uis[128]", target, valor, motivo) != 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Use: /multar [playerid] [valor] [motivo]");
        return 1;
    }
    
    if(!IsPlayerConnected(target))
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Jogador não conectado.");
        return 1;
    }
    
    Policial_MultarJogador(playerid, target, valor, motivo);
    return 1;
}

CMD:spawn(playerid, params[])
{
    if(pInfo[playerid][pi_nivel_admin] < 1)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Você não é administrador.");
        return 1;
    }
    
    Veiculo_InicializarSpawns();
    SendClientMessage(playerid, COR_CONFIRMACAO, "[ADMIN] Veículos spawned!");
    return 1;
}

stock Admin_VerificarVIPExpirado(playerid)
{
    return 1;
}

stock Admin_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    return 1;
}