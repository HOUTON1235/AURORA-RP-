// =============================================================================
//   sistema_cidade.inc — Transporte, clima, ciclo e eventos
//   Aurora Roleplay 2026
//   Requirements: 17.1–17.8, 16.6
// =============================================================================

// ---------------------------------------------------------------------------
// Estados de clima
// ---------------------------------------------------------------------------
#define CLIMA_ENSOLARADO    0
#define CLIMA_NUBLADO       1
#define CLIMA_CHUVOSO       2
#define CLIMA_TEMPESTADE    3

new gClimaAtual = CLIMA_ENSOLARADO;
new gHoraServidor = 12;

// IDs de clima SA-MP correspondentes
static const gClimaWeather[] = {10, 8, 16, 9};

// Pontos de parada de ônibus (posições simplificadas)
static const Float:gParadasOnibus[][3] = {
    {1487.0, -1768.0, 13.0},
    {1958.0, -1343.0, 15.0},
    {459.0,  -616.0,  16.0},
    {1026.0, -1020.0, 32.0}
};

// ---------------------------------------------------------------------------
// Inicializar transporte público — Req 17.1
// ---------------------------------------------------------------------------
stock Cidade_IniciarTransportePublico()
{
    // Wraps sistema_botsrck — os bots já inicializam via ConectarBotsSV()
    // chamado em OnGameModeInit no ARP.pwn
    printf("[CIDADE] Transporte publico inicializado.");
}

// ---------------------------------------------------------------------------
// Atualizar clima — Req 17.2
// ---------------------------------------------------------------------------
stock Cidade_AtualizarClima()
{
    // Transição gradual: avança 1 estado por ciclo, cicla entre os 4
    gClimaAtual = (gClimaAtual + 1) % 4;
    SetWeather(gClimaWeather[gClimaAtual]);

    new nomeClima[16];
    switch(gClimaAtual)
    {
        case CLIMA_ENSOLARADO:  nomeClima = "Ensolarado";
        case CLIMA_NUBLADO:     nomeClima = "Nublado";
        case CLIMA_CHUVOSO:     nomeClima = "Chuvoso";
        case CLIMA_TEMPESTADE:  nomeClima = "Tempestade";
    }

    new msg[64];
    format(msg, sizeof(msg), "[CLIMA] Tempo mudou para: %s.", nomeClima);
    SendClientMessageToAll(COR_SERVIDOR1, msg);

    Cidade_AplicarEfeitosClima();
}

// ---------------------------------------------------------------------------
// Aplicar efeitos do clima — Req 17.3
// ---------------------------------------------------------------------------
stock Cidade_AplicarEfeitosClima()
{
    if(gClimaAtual == CLIMA_TEMPESTADE)
    {
        SetPlayerFog(0, 50); // visibilidade reduzida para todos
        SendClientMessageToAll(COR_SERVIDOR1,
            "[CLIMA] Tempestade! Visibilidade reduzida. Consumo de combustivel +20%%.");
    }
}

// ---------------------------------------------------------------------------
// Atualizar hora — Req 17.4
// ---------------------------------------------------------------------------
stock Cidade_AtualizarHora()
{
    gHoraServidor = (gHoraServidor + 1) % 24;
    SetWorldTime(gHoraServidor);
}

// ---------------------------------------------------------------------------
// Atualizar blips — Req 17.5
// ---------------------------------------------------------------------------
stock Cidade_AtualizarBlips()
{
    // Blips fixos de estabelecimentos — simplificado
    // Na implementação completa, filtrar por horário de funcionamento
}

// ---------------------------------------------------------------------------
// Mensagem automática — Req 17.6
// ---------------------------------------------------------------------------
static new gMsgAutoIdx = 0;
static const gMensagensAuto[][128] = {
    "[AURORA] Bem-vindo ao Aurora Roleplay! Use /ajuda para ver os comandos.",
    "[AURORA] Lembre-se: roleplay sempre em primeiro lugar!",
    "[AURORA] Precisa de ajuda? Use /socorro ou chame um admin.",
    "[AURORA] Visite a concessionaria para comprar seu veiculo!",
    "[AURORA] Acesse o celular com /celular para usar todos os apps.",
    "[AURORA] Junte-se a uma organizacao e conquiste territorios!",
    "[AURORA] Cuide da fome e sede do seu personagem!"
};

stock Cidade_EnviarMensagemAutomatica()
{
    SendClientMessageToAll(COR_SERVIDOR1, gMensagensAuto[gMsgAutoIdx]);
    gMsgAutoIdx = (gMsgAutoIdx + 1) % sizeof(gMensagensAuto);
}

// ---------------------------------------------------------------------------
// Gerar evento de território — Req 17.7
// ---------------------------------------------------------------------------
stock Cidade_GerarEventoTerritorio(territorio_id)
{
    new msg[128];
    format(msg, sizeof(msg),
        "[EVENTO] Incidente de seguranca no territorio #%d! Zona controlada ha mais de 24h.", territorio_id);
    SendClientMessageToAll(COR_ERRO, msg);
}

// ---------------------------------------------------------------------------
// Verificar passagem de ônibus — Req 17.1
// ---------------------------------------------------------------------------
stock Cidade_VerificarPassagemOnibus(playerid)
{
    foreach(new p : 0, sizeof(gParadasOnibus))
    {
        if(IsPlayerInRangeOfPoint(playerid,
            3.0,
            gParadasOnibus[p][0],
            gParadasOnibus[p][1],
            gParadasOnibus[p][2]))
        {
            // Cobra passagem R$5
            if(Economia_RetirarDinheiro(playerid, ECO_TAXA_PASSAGEM))
                SendClientMessage(playerid, COR_SERVIDOR1, "[ONIBUS] Passagem de R$5 cobrada.");
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// NPC estático resposta — Req 17.8
// ---------------------------------------------------------------------------
stock Cidade_RespostaNPCEstatico(playerid, npc_id)
{
    static const gRespostas[][128] = {
        "Bem-vindo ao Banco! Aqui voce pode depositar, sacar e transferir dinheiro.",
        "Este e o Hospital. Emergencias: /socorro. Taxa de internacao: R$500.",
        "Delegacia Aurora. Policias de plantao 24h. Denuncias: /ocorrencia novo."
    };

    if(npc_id < 0 || npc_id >= sizeof(gRespostas)) return;
    SendClientMessage(playerid, COR_DESTAQUE, gRespostas[npc_id]);
}

// ---------------------------------------------------------------------------
// Consumo extra de combustível em tempestade
// ---------------------------------------------------------------------------
stock Float:Cidade_MultiplicadorCombustivel()
{
    return (gClimaAtual == CLIMA_TEMPESTADE) ? 1.2 : 1.0;
}

// ---------------------------------------------------------------------------
// Comando /falar — Req 17.8
// ---------------------------------------------------------------------------
CMD:falar(playerid, params[])
{
    #pragma unused params
    // Detecta NPC mais próximo (simplificado — 3 NPCs fixos)
    static const Float:gPosNPCs[][3] = {
        {1026.0, -1020.0, 32.0},  // Banco
        {1174.0, -1323.0, 14.0},  // Hospital
        {459.0,  -616.0,  16.0}   // Delegacia
    };

    for(new n = 0; n < sizeof(gPosNPCs); n++)
    {
        if(IsPlayerInRangeOfPoint(playerid, 5.0, gPosNPCs[n][0], gPosNPCs[n][1], gPosNPCs[n][2]))
        {
            Cidade_RespostaNPCEstatico(playerid, n);
            return 1;
        }
    }
    SendClientMessage(playerid, COR_ERRO, "[CIDADE] Nenhum NPC por perto.");
    return 1;
}
