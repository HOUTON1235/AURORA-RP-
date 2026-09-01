// =============================================================================
//   sistema_banco.inc — ATM, extrato e PIN
//   Aurora Roleplay 2026
//   Requirements: 4.2–4.7, 11.7
// =============================================================================

// ---------------------------------------------------------------------------
// Verificar e definir PIN — Req 11.7
// ---------------------------------------------------------------------------
stock Banco_VerificarPIN(playerid, pin)
{
    new pinStr[5];
    format(pinStr, sizeof(pinStr), "%04d", pin);
    return (strcmp(pInfo[playerid][pi_pin], pinStr) == 0) ? 1 : 0;
}

stock Banco_DefinirPIN(playerid, pin)
{
    format(pInfo[playerid][pi_pin], 5, "%04d", pin);
}

// ---------------------------------------------------------------------------
// Exibir menu bancário — Req 4.2
// ---------------------------------------------------------------------------
stock Banco_ExibirMenu(playerid)
{
    new opcoes[] = "Depositar\nSacar\nTransferir\nExtrato\nDefinir PIN\nFechar";
    ShowPlayerDialog(playerid, DIALOG_BANCO_MENU, DIALOG_STYLE_LIST,
        "Banco Aurora", opcoes, "Selecionar", "Fechar");
}

// ---------------------------------------------------------------------------
// Exibir extrato — Req 4.7
// ---------------------------------------------------------------------------
stock Banco_ExibirExtrato(playerid)
{
    new lista[2048], linha[128];
    lista[0] = EOS;

    new idx = pExtratoIdx[playerid];
    new count = 0;

    for(new i = 0; i < ECO_MAX_EXTRATO; i++)
    {
        new pos = (idx - 1 - i + ECO_MAX_EXTRATO) % ECO_MAX_EXTRATO;
        if(pExtrato[playerid][pos][ext_timestamp] == 0) continue;

        format(linha, sizeof(linha), "%s | R$%d | %s\n",
            pExtrato[playerid][pos][ext_tipo],
            pExtrato[playerid][pos][ext_valor],
            pExtrato[playerid][pos][ext_envolvido]);
        strcat(lista, linha, sizeof(lista));

        count++;
        if(count >= 50) break;
    }

    if(lista[0] == EOS)
        strcopy(lista, "Nenhuma transacao encontrada.", sizeof(lista));

    ShowPlayerDialog(playerid, DIALOG_BANCO_EXTRATO, DIALOG_STYLE_MSGBOX,
        "Extrato", lista, "Fechar", "");
}

// ---------------------------------------------------------------------------
// Processar dialogs bancários
// ---------------------------------------------------------------------------
stock Banco_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    if(!response)
    {
        if(dialogid != DIALOG_BANCO_MENU) Banco_ExibirMenu(playerid);
        return 1;
    }

    if(dialogid == DIALOG_BANCO_MENU)
    {
        if(listitem == 0)
        {
            ShowPlayerDialog(playerid, DIALOG_BANCO_DEPOSITAR, DIALOG_STYLE_INPUT,
                "Depositar", "Digite o valor a depositar:", "Confirmar", "Voltar");
        }
        else if(listitem == 1)
        {
            new titulo[64];
            format(titulo, sizeof(titulo), "Sacar (Saldo: R$%d)", pInfo[playerid][pi_banco]);
            ShowPlayerDialog(playerid, DIALOG_BANCO_SACAR, DIALOG_STYLE_INPUT,
                titulo, "Digite o valor a sacar:", "Confirmar", "Voltar");
        }
        else if(listitem == 2)
        {
            ShowPlayerDialog(playerid, DIALOG_BANCO_TRANSFERIR, DIALOG_STYLE_INPUT,
                "Transferir", "Digite: nome|valor", "Confirmar", "Voltar");
        }
        else if(listitem == 3)
        {
            Banco_ExibirExtrato(playerid);
        }
        else if(listitem == 4)
        {
            ShowPlayerDialog(playerid, DIALOG_BANCO_PIN, DIALOG_STYLE_PASSWORD,
                "Definir PIN", "Digite seu novo PIN (4 digitos):", "Salvar", "Cancelar");
        }
        return 1;
    }

    if(dialogid == DIALOG_BANCO_DEPOSITAR)
    {
        new valor = strval(inputtext);
        if(valor <= 0)
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Valor invalido.");
        else if(!Economia_Depositar(playerid, valor))
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Dinheiro em mao insuficiente.");
        else
        {
            new msg[64];
            format(msg, sizeof(msg), "[BANCO] R$%d depositado.", valor);
            SendClientMessage(playerid, COR_CONFIRMACAO, msg);
        }
        Banco_ExibirMenu(playerid);
        return 1;
    }

    if(dialogid == DIALOG_BANCO_SACAR)
    {
        new valor = strval(inputtext);
        if(valor <= 0)
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Valor invalido.");
        else if(!Economia_Sacar(playerid, valor))
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Saldo insuficiente ou teto de caixa atingido.");
        else
        {
            new msg[64];
            format(msg, sizeof(msg), "[BANCO] R$%d sacado.", valor);
            SendClientMessage(playerid, COR_CONFIRMACAO, msg);
        }
        Banco_ExibirMenu(playerid);
        return 1;
    }

    if(dialogid == DIALOG_BANCO_TRANSFERIR)
    {
        new destNome[24], valor;
        if(sscanf(inputtext, "p<|>s[24]i", destNome, valor))
        {
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Formato: nome|valor");
            Banco_ExibirMenu(playerid);
            return 1;
        }
        new dest = -1;
        foreach(new i : Player)
        {
            new n[MAX_PLAYER_NAME];
            GetPlayerName(i, n, MAX_PLAYER_NAME);
            if(strcmp(n, destNome, true) == 0) { dest = i; break; }
        }
        if(dest == -1)
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Jogador nao encontrado.");
        else if(!Economia_Transferir(playerid, dest, valor))
            SendClientMessage(playerid, COR_ERRO, "[BANCO] Saldo bancario insuficiente.");
        else
        {
            new msg[64];
            format(msg, sizeof(msg), "[BANCO] R$%d transferido para %s.", valor, destNome);
            SendClientMessage(playerid, COR_CONFIRMACAO, msg);
        }
        Banco_ExibirMenu(playerid);
        return 1;
    }

    if(dialogid == DIALOG_BANCO_PIN)
    {
        if(strlen(inputtext) != 4)
            SendClientMessage(playerid, COR_ERRO, "[BANCO] PIN deve ter 4 digitos.");
        else
        {
            Banco_DefinirPIN(playerid, strval(inputtext));
            SendClientMessage(playerid, COR_CONFIRMACAO, "[BANCO] PIN definido.");
        }
        return 1;
    }

    return 1;
}
