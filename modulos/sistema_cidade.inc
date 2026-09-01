// =============================================================================
// SISTEMA DE CIDADE — Aurora Roleplay 2026
// sistema_cidade.inc
// =============================================================================

stock Cidade_AtualizarClima()
{
    new clima = random(15);
    SetWeather(clima);
    return 1;
}

stock Cidade_AtualizarHora()
{
    new hora = random(24);
    SetWorldTime(hora);
    return 1;
}

stock Cidade_EnviarMensagemAutomatica()
{
    new msg[128];
    new msgs[3][128] = {
        "Bem-vindo ao Aurora Roleplay!",
        "Use /help para ver os comandos",
        "Respeite os demais jogadores"
    };
    
    strcpy(msg, msgs[random(3)]);
    SendClientMessageToAll(COR_DESTAQUE, msg);
    return 1;
}

stock Cidade_VerificarPassagemOnibus(playerid)
{
    // Sistema de transporte público
    return 1;
}

stock Cidade_IniciarTransportePublico()
{
    printf("[CIDADE] Transporte público iniciado.");
    return 1;
}