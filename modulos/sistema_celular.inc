// =============================================================================
//   sistema_celular.inc — Apps, SMS, GPS e VoIP privado
//   Aurora Roleplay 2026
//   Requirements: 11.1–11.11, 21.5
// =============================================================================

#define CEL_MAX_CONTATOS    50
#define CEL_MAX_SMS         20

enum E_Contato
{
    cont_nome[MAX_PLAYER_NAME],
    cont_numero[12]
}

new pContatos[MAX_PLAYERS][CEL_MAX_CONTATOS][E_Contato];
new pSMSPendentes[MAX_PLAYERS][CEL_MAX_SMS][128];
new pSMSCount[MAX_PLAYERS];

// Chamada ativa
new pChamadaAlvo[MAX_PLAYERS];
new SV_Stream:pStreamChamada[MAX_PLAYERS];

// ---------------------------------------------------------------------------
// Inicializar por jogador
// ---------------------------------------------------------------------------
stock Celular_InicializarJogador(playerid)
{
    pChamadaAlvo[playerid]   = -1;
    pStreamChamada[playerid] = SV_STREAM:SV_NULL;
    pSMSCount[playerid]      = 0;

    // Gera número de celular único
    if(pInfo[playerid][pi_numero_celular][0] == EOS || pInfo[playerid][pi_numero_celular][0] == '\0')
    {
        format(pInfo[playerid][pi_numero_celular], 12, "9%d%d%d%d-%d%d%d%d",
            random(9)+1, random(10), random(10), random(10),
            random(10), random(10), random(10), random(10));
    }
}

// ---------------------------------------------------------------------------
// Abrir menu principal — Req 11.1
// ---------------------------------------------------------------------------
stock Celular_AbrirMenu(playerid)
{
    // Req 11.11 — bloqueio se incapacitado ou algemado
    if(pInfo[playerid][pi_incapacitado] || pInfo[playerid][pi_algemado])
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] Voce nao pode usar o celular agora.");

    pInfo[playerid][pi_celular_aberto] = true;

    new apps[] = "Contatos\nMensagens\nLigacoes\nGPS\nBanco\nAnuncios\nServicos\nInformacoes";
    ShowPlayerDialog(playerid, DIALOG_CELULAR_MENU, DIALOG_STYLE_LIST,
        "Celular Aurora", apps, "Abrir", "Fechar");
}

// ---------------------------------------------------------------------------
// Iniciar chamada — Req 11.3, 11.4
// ---------------------------------------------------------------------------
stock Celular_IniciarChamada(playerid, alvo)
{
    if(pChamadaAlvo[alvo] != -1)
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] Numero ocupado.");

    if(pChamadaAlvo[playerid] != -1)
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] Voce ja esta em uma chamada.");

    pChamadaAlvo[playerid] = alvo;
    pChamadaAlvo[alvo]     = playerid;

    // Cria stream VoIP privado para a chamada
    new SV_Stream:stream = VoIP_CriarStreamChamada(playerid, alvo);
    pStreamChamada[playerid] = stream;
    pStreamChamada[alvo]     = stream;

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new msg[64];
    format(msg, sizeof(msg), "[CELULAR] Chamada recebida de %s. Use /atender ou /recusar.", nome);
    SendClientMessage(alvo, COR_DESTAQUE, msg);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[CELULAR] Chamada iniciada. Aguardando...");
}

// ---------------------------------------------------------------------------
// Encerrar chamada — Req 11.5
// ---------------------------------------------------------------------------
stock Celular_EncerrarChamada(playerid)
{
    new alvo = pChamadaAlvo[playerid];
    if(alvo == -1) return;

    if(pStreamChamada[playerid] != SV_STREAM:SV_NULL)
    {
        VoIP_DestruirStreamChamada(pStreamChamada[playerid], playerid, alvo);
        pStreamChamada[playerid] = SV_STREAM:SV_NULL;
        if(alvo != -1 && IsPlayerConnected(alvo))
            pStreamChamada[alvo] = SV_STREAM:SV_NULL;
    }

    pChamadaAlvo[playerid] = -1;
    if(alvo != -1 && IsPlayerConnected(alvo))
    {
        pChamadaAlvo[alvo] = -1;
        SendClientMessage(alvo, COR_SERVIDOR1, "[CELULAR] Chamada encerrada.");
    }

    SendClientMessage(playerid, COR_SERVIDOR1, "[CELULAR] Chamada encerrada.");
}

// ---------------------------------------------------------------------------
// Enviar SMS — Req 11.6
// ---------------------------------------------------------------------------
stock Celular_EnviarSMS(playerid, alvo, const mensagem[])
{
    new remetente[MAX_PLAYER_NAME], msg[192];
    GetPlayerName(playerid, remetente, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[SMS de %s]: %s", remetente, mensagem);

    if(IsPlayerConnected(alvo) && pInfo[alvo][pi_logado])
    {
        SendClientMessage(alvo, COR_DESTAQUE, msg);
    }
    else
    {
        // Persiste para próximo login
        if(pSMSCount[alvo] < CEL_MAX_SMS)
        {
            strcopy(pSMSPendentes[alvo][pSMSCount[alvo]], msg, 128);
            pSMSCount[alvo]++;
        }
    }
    SendClientMessage(playerid, COR_CONFIRMACAO, "[CELULAR] Mensagem enviada.");
}

// ---------------------------------------------------------------------------
// Entregar SMS pendentes ao logar
// ---------------------------------------------------------------------------
stock Celular_EntregarSMSPendentes(playerid)
{
    for(new i = 0; i < pSMSCount[playerid]; i++)
        SendClientMessage(playerid, COR_DESTAQUE, pSMSPendentes[playerid][i]);
    pSMSCount[playerid] = 0;
}

// ---------------------------------------------------------------------------
// GPS — Req 11.8
// ---------------------------------------------------------------------------
stock Celular_AbrirGPS(playerid)
{
    new destinos[] = "Hospital\nBanco (ATM)\nDelegacia\nPosto de Gasolina\nCentro de Empregos\nPrefeitura";
    ShowPlayerDialog(playerid, DIALOG_CELULAR_GPS, DIALOG_STYLE_LIST,
        "GPS Aurora", destinos, "Ir!", "Cancelar");
}

static const gGPSDestinos[][3] = {
    {1174, -1323, 14},   // Hospital
    {1026, -1020, 32},   // Banco
    {459,  -616,  16},   // Delegacia
    {620,  -1292, 18},   // Posto
    {372,  -208,  1001}, // Centro empregos
    {1545, -1675, 13}    // Prefeitura
};

stock Celular_ProcessarGPS(playerid, destino)
{
    if(destino < 0 || destino >= 6) return;
    SetPlayerCheckpoint(playerid,
        float(gGPSDestinos[destino][0]),
        float(gGPSDestinos[destino][1]),
        float(gGPSDestinos[destino][2]),
        5.0);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[GPS] Checkpoint criado no mapa.");
}

// ---------------------------------------------------------------------------
// Transferência bancária via celular — Req 11.7
// ---------------------------------------------------------------------------
stock Celular_TransferenciaBancaria(playerid, alvo, valor, pin)
{
    if(!Banco_VerificarPIN(playerid, pin))
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] PIN incorreto.");

    if(!Economia_Transferir(playerid, alvo, valor))
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] Saldo insuficiente.");

    new nomeAlvo[MAX_PLAYER_NAME], msg[64];
    GetPlayerName(alvo, nomeAlvo, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[CELULAR] R$%d transferido para %s.", valor, nomeAlvo);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Publicar anúncio — Req 11.9
// ---------------------------------------------------------------------------
stock Celular_PublicarAnuncio(playerid, const texto[])
{
    if(!Economia_RetirarDinheiro(playerid, ECO_TAXA_ANUNCIO))
        return SendClientMessage(playerid, COR_ERRO, "[CELULAR] Dinheiro insuficiente para anuncio (R$100).");

    new nome[MAX_PLAYER_NAME], msg[256];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(msg, sizeof(msg), "[ANUNCIO] %s: %s", nome, texto);
    SendClientMessageToAll(COR_DESTAQUE, msg);
}

// ---------------------------------------------------------------------------
// Solicitar serviço — Req 11.10, 21.5
// ---------------------------------------------------------------------------
stock Celular_SolicitarServico(playerid, tipo_servico)
{
    // 0=Taxi, 1=Mecanico, 2=SAMU, 3=Pizza
    static const gServicos[][16] = {"Taxista","Mecanico","Paramedico","Jornalista"};
    static const E_Profissao:gProfs[] = {PROF_TAXISTA, PROF_MECANICO, PROF_PARAMEDICO, PROF_JORNALISTA};

    if(tipo_servico < 0 || tipo_servico > 3) return;

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new msg[128], encontrou = 0;
    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        if(pInfo[i][pi_profissao] != gProfs[tipo_servico]) continue;

        format(msg, sizeof(msg), "[SERVICO] %s solicita %s. Use /aceitar para atender.", nome, gServicos[tipo_servico]);
        SendClientMessage(i, COR_DESTAQUE, msg);
        encontrou++;
    }

    if(!encontrou)
    {
        format(msg, sizeof(msg), "[CELULAR] Nenhum %s disponivel no momento.", gServicos[tipo_servico]);
        SendClientMessage(playerid, COR_ERRO, msg);
    }
    else
        SendClientMessage(playerid, COR_CONFIRMACAO, "[CELULAR] Solicitacao enviada aos prestadores disponiveis.");
}

// ---------------------------------------------------------------------------
// Processar dialogs
// ---------------------------------------------------------------------------
stock Celular_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused inputtext

    if(!response)
    {
        pInfo[playerid][pi_celular_aberto] = false;
        return 1;
    }

    switch(dialogid)
    {
        case DIALOG_CELULAR_MENU:
        {
            switch(listitem)
            {
                case 0: // Contatos
                {
                    new lista[1024], linha[64];
                    for(new c = 0; c < CEL_MAX_CONTATOS; c++)
                    {
                        if(pContatos[playerid][c][cont_nome][0] == EOS) continue;
                        format(linha, sizeof(linha), "%s | %s\n", pContatos[playerid][c][cont_nome], pContatos[playerid][c][cont_numero]);
                        strcat(lista, linha, sizeof(lista));
                    }
                    if(lista[0] == EOS) strcopy(lista, "Nenhum contato.", sizeof(lista));
                    ShowPlayerDialog(playerid, DIALOG_CELULAR_CONTATOS, DIALOG_STYLE_MSGBOX, "Contatos", lista, "Fechar", "");
                }
                case 1: // Mensagens — simplificado
                    SendClientMessage(playerid, COR_SERVIDOR1, "[CELULAR] Use /sms [nome] [mensagem] para enviar.");
                case 2: // Ligações
                    SendClientMessage(playerid, COR_SERVIDOR1, "[CELULAR] Use /ligar [nome] para ligar.");
                case 3: Celular_AbrirGPS(playerid);
                case 4: Banco_ExibirMenu(playerid);
                case 5: // Anúncios
                    ShowPlayerDialog(playerid, DIALOG_CELULAR_SMS, DIALOG_STYLE_INPUT,
                        "Publicar Anuncio (R$100)", "Digite seu anuncio:", "Publicar", "Cancelar");
                case 6: // Serviços
                {
                    new servicos[] = "Taxi\nMecanico\nSAMU\nPizza";
                    ShowPlayerDialog(playerid, DIALOG_CELULAR_CONTATOS + 1, DIALOG_STYLE_LIST, "Servicos", servicos, "Solicitar", "Cancelar");
                }
                case 7: // Info
                {
                    new info[64];
                    format(info, sizeof(info), "Numero: %s", pInfo[playerid][pi_numero_celular]);
                    SendClientMessage(playerid, COR_DESTAQUE, info);
                }
            }
        }
        case DIALOG_CELULAR_GPS:
            Celular_ProcessarGPS(playerid, listitem);

        case DIALOG_CELULAR_SMS: // Anúncio
            Celular_PublicarAnuncio(playerid, inputtext);
    }

    pInfo[playerid][pi_celular_aberto] = false;
    return 1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:celular(playerid, params[])
{
    #pragma unused params
    Celular_AbrirMenu(playerid);
    return 1;
}

CMD:ligar(playerid, params[])
{
    new destNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", destNome))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /ligar [nome]"), 1;

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, destNome, true) == 0) { Celular_IniciarChamada(playerid, i); return 1; }
    }
    SendClientMessage(playerid, COR_ERRO, "[CELULAR] Jogador nao encontrado.");
    return 1;
}

CMD:desligar(playerid, params[])
{
    #pragma unused params
    Celular_EncerrarChamada(playerid);
    return 1;
}

CMD:sms(playerid, params[])
{
    new destNome[MAX_PLAYER_NAME], mensagem[128];
    if(sscanf(params, "s[24]s[128]", destNome, mensagem))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /sms [nome] [mensagem]"), 1;

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, destNome, true) == 0) { Celular_EnviarSMS(playerid, i, mensagem); return 1; }
    }
    SendClientMessage(playerid, COR_ERRO, "[CELULAR] Jogador nao encontrado.");
    return 1;
}
