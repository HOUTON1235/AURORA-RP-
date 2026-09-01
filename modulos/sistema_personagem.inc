// =============================================================================
//   sistema_personagem.inc — Perfil, saúde, fome/sede e HUD
//   Aurora Roleplay 2026
//   Requirements: 2.1–2.6, 6.1–6.8, 7.1–7.7, 20.1–20.2, 20.8, 22.4, 22.6
// =============================================================================

// ---------------------------------------------------------------------------
// IDs de textdraws do HUD (por jogador)
// ---------------------------------------------------------------------------
new PlayerText:tdHP[MAX_PLAYERS]        = {PlayerText:INVALID_TEXT_DRAW, ...};
new PlayerText:tdFome[MAX_PLAYERS]      = {PlayerText:INVALID_TEXT_DRAW, ...};
new PlayerText:tdSede[MAX_PLAYERS]      = {PlayerText:INVALID_TEXT_DRAW, ...};
new PlayerText:tdDinheiro[MAX_PLAYERS]  = {PlayerText:INVALID_TEXT_DRAW, ...};
new PlayerText:tdNivel[MAX_PLAYERS]     = {PlayerText:INVALID_TEXT_DRAW, ...};

// ---------------------------------------------------------------------------
// Validar idade — Req 2.6
// ---------------------------------------------------------------------------
stock Personagem_ValidarIdade(idade)
{
    return (idade >= 18 && idade <= 70) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Iniciar criação de personagem (novo jogador) — Req 2.1
// ---------------------------------------------------------------------------
stock Personagem_IniciarCriacao(playerid)
{
    // Posiciona em câmera de criação
    SetPlayerPos(playerid, 1958.33, -1343.07, 15.36);
    SetPlayerCameraPos(playerid, 1958.33, -1350.0, 18.0);
    SetPlayerCameraLookAt(playerid, 1958.33, -1343.07, 15.36);
    TogglePlayerControllable(playerid, 0);

    ShowPlayerDialog(playerid, DIALOG_CRIACAO_SEXO, DIALOG_STYLE_LIST,
        "Criacao de Personagem — Sexo",
        "Masculino\nFeminino",
        "Selecionar", "");
}

// ---------------------------------------------------------------------------
// Processar dialogs de criação
// ---------------------------------------------------------------------------
stock Personagem_ProcessarCriacao(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_CRIACAO_SEXO:
        {
            pInfo[playerid][pi_sexo] = listitem; // 0=masc, 1=fem
            ShowPlayerDialog(playerid, DIALOG_CRIACAO_IDADE, DIALOG_STYLE_INPUT,
                "Criacao de Personagem — Idade",
                "Digite sua idade (18 a 70 anos):",
                "Confirmar", "");
        }
        case DIALOG_CRIACAO_IDADE:
        {
            new idade = strval(inputtext);
            if(!Personagem_ValidarIdade(idade))
            {
                SendClientMessage(playerid, COR_ERRO, "[PERSONAGEM] Idade invalida. Deve ser entre 18 e 70 anos.");
                ShowPlayerDialog(playerid, DIALOG_CRIACAO_IDADE, DIALOG_STYLE_INPUT,
                    "Criacao de Personagem — Idade",
                    "Idade invalida! Digite sua idade (18 a 70 anos):",
                    "Confirmar", "");
                return 1;
            }
            pInfo[playerid][pi_idade] = idade;

            new origens[] = "Sao Paulo\nRio de Janeiro\nBelo Horizonte\nSalvador\nCuritiba\nFortaleza\nManaus\nRecife\nPorto Alegre\nOutra";
            ShowPlayerDialog(playerid, DIALOG_CRIACAO_ORIGEM, DIALOG_STYLE_LIST,
                "Criacao de Personagem — Origem", origens, "Selecionar", "");
        }
        case DIALOG_CRIACAO_ORIGEM:
        {
            static const origens[][20] = {
                "Sao Paulo","Rio de Janeiro","Belo Horizonte","Salvador",
                "Curitiba","Fortaleza","Manaus","Recife","Porto Alegre","Outra"
            };
            if(listitem >= 0 && listitem < sizeof(origens))
                strcopy(pInfo[playerid][pi_origem], origens[listitem], 32);

            // Criação concluída
            TogglePlayerControllable(playerid, 1);
            SetCameraBehindPlayer(playerid);

            // Posiciona na spawn inicial
            SetPlayerPos(playerid, 1958.33, -1343.07, 15.36);
            SetPlayerInterior(playerid, 0);
            SetPlayerVirtualWorld(playerid, 0);

            // Inicializa valores físicos
            pInfo[playerid][pi_fome] = 100;
            pInfo[playerid][pi_sede] = 100;
            SetPlayerHealth(playerid, 100.0);

            // Salva imediatamente
            Personagem_Salvar(playerid);
            SendClientMessage(playerid, COR_CONFIRMACAO, "[PERSONAGEM] Personagem criado com sucesso! Bem-vindo ao servidor.");

            Personagem_CriarHUD(playerid);
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Carregar personagem (jogador retornante) — Req 1.6
// ---------------------------------------------------------------------------
stock Personagem_Carregar(playerid)
{
    // Dados já carregados por Login_CarregarDados
    // Aqui aplica os valores no mundo
    SetPlayerHealth(playerid, pInfo[playerid][pi_hp] > 0.0 ? pInfo[playerid][pi_hp] : 100.0);
    SetPlayerArmour(playerid, pInfo[playerid][pi_colete]);
    SetPlayerSkin(playerid, pInfo[playerid][pi_skin]);

    Personagem_CriarHUD(playerid);
    Progressao_Carregar(playerid);
}

// ---------------------------------------------------------------------------
// Salvar — Req 20.1, 20.2
// ---------------------------------------------------------------------------
stock Personagem_Salvar(playerid)
{
    if(!pInfo[playerid][pi_logado]) return 0;

    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    // Captura posição atual
    GetPlayerPos(playerid, pInfo[playerid][pi_pos_x], pInfo[playerid][pi_pos_y], pInfo[playerid][pi_pos_z]);
    GetPlayerFacingAngle(playerid, pInfo[playerid][pi_angulo]);
    pInfo[playerid][pi_interior]     = GetPlayerInterior(playerid);
    pInfo[playerid][pi_virtual_world] = GetPlayerVirtualWorld(playerid);

    new Float:hp, Float:armour;
    GetPlayerHealth(playerid, hp);
    GetPlayerArmour(playerid, armour);
    pInfo[playerid][pi_hp]     = hp;
    pInfo[playerid][pi_colete] = armour;

    DOF2_SetString(caminho, "Senha",        pInfo[playerid][pi_senha]);
    DOF2_SetString(caminho, "Email",        pInfo[playerid][pi_email]);
    DOF2_SetString(caminho, "Origem",       pInfo[playerid][pi_origem]);
    DOF2_SetInt   (caminho, "Nivel",        pInfo[playerid][pi_nivel]);
    DOF2_SetInt   (caminho, "XP",           pInfo[playerid][pi_xp]);
    DOF2_SetInt   (caminho, "Reputacao",    pInfo[playerid][pi_reputacao]);
    DOF2_SetInt   (caminho, "Skin",         pInfo[playerid][pi_skin]);
    DOF2_SetInt   (caminho, "Sexo",         pInfo[playerid][pi_sexo]);
    DOF2_SetInt   (caminho, "Idade",        pInfo[playerid][pi_idade]);
    DOF2_SetInt   (caminho, "Dinheiro",     pInfo[playerid][pi_dinheiro]);
    DOF2_SetInt   (caminho, "Banco",        pInfo[playerid][pi_banco]);
    DOF2_SetInt   (caminho, "DividaHospital", pInfo[playerid][pi_divida_hospital]);
    DOF2_SetFloat (caminho, "HP",           hp);
    DOF2_SetFloat (caminho, "Colete",       armour);
    DOF2_SetInt   (caminho, "Fome",         pInfo[playerid][pi_fome]);
    DOF2_SetInt   (caminho, "Sede",         pInfo[playerid][pi_sede]);
    DOF2_SetFloat (caminho, "PosX",         pInfo[playerid][pi_pos_x]);
    DOF2_SetFloat (caminho, "PosY",         pInfo[playerid][pi_pos_y]);
    DOF2_SetFloat (caminho, "PosZ",         pInfo[playerid][pi_pos_z]);
    DOF2_SetFloat (caminho, "Angulo",       pInfo[playerid][pi_angulo]);
    DOF2_SetInt   (caminho, "Interior",     pInfo[playerid][pi_interior]);
    DOF2_SetInt   (caminho, "VirtualWorld", pInfo[playerid][pi_virtual_world]);
    DOF2_SetInt   (caminho, "Profissao",    _:pInfo[playerid][pi_profissao]);
    DOF2_SetInt   (caminho, "OrgId",        pInfo[playerid][pi_org_id]);
    DOF2_SetInt   (caminho, "OrgRank",      pInfo[playerid][pi_org_rank]);
    DOF2_SetInt   (caminho, "EmpId",        pInfo[playerid][pi_emp_id]);
    DOF2_SetInt   (caminho, "CasaId",       pInfo[playerid][pi_casa_id]);
    DOF2_SetInt   (caminho, "NivelAdmin",   pInfo[playerid][pi_nivel_admin]);
    DOF2_SetInt   (caminho, "NivelVIP",     pInfo[playerid][pi_nivel_vip]);
    DOF2_SetInt   (caminho, "VipExpira",    pInfo[playerid][pi_vip_expira]);
    DOF2_SetInt   (caminho, "Mortes",       pInfo[playerid][pi_mortes]);
    DOF2_SetInt   (caminho, "Kills",        pInfo[playerid][pi_kills]);
    DOF2_SetInt   (caminho, "HorasJogadas", pInfo[playerid][pi_horas_jogadas]);
    DOF2_SetInt   (caminho, "Conquistas",   pConquistas[playerid]);
    DOF2_SetInt   (caminho, "Wanted",       pInfo[playerid][pi_wanted]);

    if(!DOF2_SaveFile(caminho))
    {
        printf("[ERRO-SAVE] Falha ao salvar %s. Retry em 30s.", nome);
        SetTimerEx("Personagem_RetrySave", 30000, false, "i", playerid);
        return 0;
    }
    return 1;
}

forward Personagem_RetrySave(playerid);
public Personagem_RetrySave(playerid)
{
    if(IsPlayerConnected(playerid))
        Personagem_Salvar(playerid);
}

// ---------------------------------------------------------------------------
// Incapacitar — Req 6.1, 6.2
// ---------------------------------------------------------------------------
stock Personagem_Incapacitar(playerid, killer)
{
    #pragma unused killer
    pInfo[playerid][pi_incapacitado] = true;
    SetPlayerHealth(playerid, 0.1); // mantém "vivo" para SA-MP
    ApplyAnimation(playerid, "CRACK", "crckidle01", 4.0, true, false, false, false, 0);

    SendClientMessage(playerid, COR_ERRO,
        "[SISTEMA] Voce esta incapacitado! Use /socorro ou aguarde 5 minutos.");

    // Timer de 5 min para morte definitiva
    pInfo[playerid][pi_timer_incapacitado] = SetTimerEx("Personagem_TimerIncapacitado", 300000, false, "i", playerid);
}

forward Personagem_TimerIncapacitado(playerid);
public Personagem_TimerIncapacitado(playerid)
{
    if(IsPlayerConnected(playerid) && pInfo[playerid][pi_incapacitado])
        Personagem_MorteDefinitiva(playerid);
}

// ---------------------------------------------------------------------------
// Morte definitiva — Req 6.4, 6.5, 6.6
// ---------------------------------------------------------------------------
stock Personagem_MorteDefinitiva(playerid)
{
    pInfo[playerid][pi_incapacitado] = false;
    pInfo[playerid][pi_mortes]++;

    // Teleporta para hospital
    SetPlayerPos(playerid, 1174.1357, -1323.6500, 14.1484);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerHealth(playerid, 30.0);
    ClearAnimations(playerid);

    // Cobra taxa hospitalar R$500
    if(!Economia_DebitarBanco(playerid, ECO_TAXA_HOSPITAL))
    {
        // Banco insuficiente: cobra do dinheiro em mão
        if(!Economia_RetirarDinheiro(playerid, ECO_TAXA_HOSPITAL))
        {
            // Sem fundos: registra dívida
            pInfo[playerid][pi_divida_hospital] += ECO_TAXA_HOSPITAL;
            SendClientMessage(playerid, COR_ERRO,
                "[HOSPITAL] Taxa de internacao de R$500 nao paga. Divida registrada. Acesso ao banco bloqueado.");
        }
        else
            SendClientMessage(playerid, COR_CONFIRMACAO, "[HOSPITAL] Taxa de internacao de R$500 debitada do seu dinheiro em mao.");
    }
    else
        SendClientMessage(playerid, COR_CONFIRMACAO, "[HOSPITAL] Taxa de internacao de R$500 debitada do seu banco.");

    SendClientMessage(playerid, COR_SERVIDOR1, "[HOSPITAL] Voce foi internado. Descanse para se recuperar.");
    Personagem_Salvar(playerid);
}

// ---------------------------------------------------------------------------
// Regeneração em casa — Req 6.7, 8.7
// ---------------------------------------------------------------------------
stock Personagem_RegenCasa(playerid)
{
    new Float:hp;
    GetPlayerHealth(playerid, hp);
    hp += 10.0;
    if(hp > 100.0) hp = 100.0;
    SetPlayerHealth(playerid, hp);
}

// ---------------------------------------------------------------------------
// Patrimônio total — Req 3.5
// ---------------------------------------------------------------------------
stock Float:Personagem_Patrimonio(playerid)
{
    return float(pInfo[playerid][pi_dinheiro] + pInfo[playerid][pi_banco]);
    // Nota: veículos e imóveis somados quando sistema_veiculo e sistema_casa estiverem integrados
}

// ---------------------------------------------------------------------------
// HUD — Req 22.4
// ---------------------------------------------------------------------------
stock Personagem_CriarHUD(playerid)
{
    // HP
    tdHP[playerid] = CreatePlayerTextDraw(playerid, 10.0, 380.0, "HP: 100");
    PlayerTextDrawFont(playerid, tdHP[playerid], 1);
    PlayerTextDrawColor(playerid, tdHP[playerid], COR_CONFIRMACAO);
    PlayerTextDrawSetShadow(playerid, tdHP[playerid], 1);
    PlayerTextDrawShow(playerid, tdHP[playerid]);

    // Fome
    tdFome[playerid] = CreatePlayerTextDraw(playerid, 10.0, 395.0, "Fome: 100");
    PlayerTextDrawFont(playerid, tdFome[playerid], 1);
    PlayerTextDrawColor(playerid, tdFome[playerid], 0xFFAA00FF);
    PlayerTextDrawSetShadow(playerid, tdFome[playerid], 1);
    PlayerTextDrawShow(playerid, tdFome[playerid]);

    // Sede
    tdSede[playerid] = CreatePlayerTextDraw(playerid, 10.0, 410.0, "Sede: 100");
    PlayerTextDrawFont(playerid, tdSede[playerid], 1);
    PlayerTextDrawColor(playerid, tdSede[playerid], 0x00AAFFFF);
    PlayerTextDrawSetShadow(playerid, tdSede[playerid], 1);
    PlayerTextDrawShow(playerid, tdSede[playerid]);

    // Dinheiro
    tdDinheiro[playerid] = CreatePlayerTextDraw(playerid, 10.0, 425.0, "R$ 0");
    PlayerTextDrawFont(playerid, tdDinheiro[playerid], 1);
    PlayerTextDrawColor(playerid, tdDinheiro[playerid], 0x32D74BFF);
    PlayerTextDrawSetShadow(playerid, tdDinheiro[playerid], 1);
    PlayerTextDrawShow(playerid, tdDinheiro[playerid]);

    // Nível
    tdNivel[playerid] = CreatePlayerTextDraw(playerid, 10.0, 440.0, "Nv. 1");
    PlayerTextDrawFont(playerid, tdNivel[playerid], 1);
    PlayerTextDrawColor(playerid, tdNivel[playerid], COR_DESTAQUE);
    PlayerTextDrawSetShadow(playerid, tdNivel[playerid], 1);
    PlayerTextDrawShow(playerid, tdNivel[playerid]);
}

stock Personagem_AtualizarHUD(playerid)
{
    if(!pInfo[playerid][pi_logado]) return;
    if(tdHP[playerid] == PlayerText:INVALID_TEXT_DRAW) return;

    new Float:hp, str[32];
    GetPlayerHealth(playerid, hp);

    format(str, sizeof(str), "HP: %d", floatround(hp));
    PlayerTextDrawSetString(playerid, tdHP[playerid], str);

    format(str, sizeof(str), "Fome: %d", pInfo[playerid][pi_fome]);
    PlayerTextDrawSetString(playerid, tdFome[playerid], str);

    format(str, sizeof(str), "Sede: %d", pInfo[playerid][pi_sede]);
    PlayerTextDrawSetString(playerid, tdSede[playerid], str);

    format(str, sizeof(str), "R$ %d", pInfo[playerid][pi_dinheiro]);
    PlayerTextDrawSetString(playerid, tdDinheiro[playerid], str);

    format(str, sizeof(str), "Nv. %d", pInfo[playerid][pi_nivel]);
    PlayerTextDrawSetString(playerid, tdNivel[playerid], str);
}

stock Personagem_DestruirHUD(playerid)
{
    if(tdHP[playerid] != PlayerText:INVALID_TEXT_DRAW)       { PlayerTextDrawDestroy(playerid, tdHP[playerid]);       tdHP[playerid]       = PlayerText:INVALID_TEXT_DRAW; }
    if(tdFome[playerid] != PlayerText:INVALID_TEXT_DRAW)     { PlayerTextDrawDestroy(playerid, tdFome[playerid]);     tdFome[playerid]     = PlayerText:INVALID_TEXT_DRAW; }
    if(tdSede[playerid] != PlayerText:INVALID_TEXT_DRAW)     { PlayerTextDrawDestroy(playerid, tdSede[playerid]);     tdSede[playerid]     = PlayerText:INVALID_TEXT_DRAW; }
    if(tdDinheiro[playerid] != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, tdDinheiro[playerid]); tdDinheiro[playerid] = PlayerText:INVALID_TEXT_DRAW; }
    if(tdNivel[playerid] != PlayerText:INVALID_TEXT_DRAW)    { PlayerTextDrawDestroy(playerid, tdNivel[playerid]);    tdNivel[playerid]    = PlayerText:INVALID_TEXT_DRAW; }
}

// ---------------------------------------------------------------------------
// Exibir perfil — Req 2.3, 2.4
// ---------------------------------------------------------------------------
stock Personagem_ExibirPerfil(playerid, alvo)
{
    new nome[MAX_PLAYER_NAME], info[512];
    GetPlayerName(alvo, nome, MAX_PLAYER_NAME);

    new sexoStr[12];
    if(pInfo[alvo][pi_sexo] == 0) sexoStr = "Masculino";
    else                          sexoStr = "Feminino";

    if(alvo == playerid)
    {
        // Próprio perfil com patrimônio
        format(info, sizeof(info),
            "Nome: %s\nSexo: %s\nIdade: %d\nOrigem: %s\nNivel: %d\nProfissao: %d\nPatrimonio: R$%.0f\nMortes: %d | Kills: %d",
            nome, sexoStr, pInfo[alvo][pi_idade], pInfo[alvo][pi_origem],
            pInfo[alvo][pi_nivel], _:pInfo[alvo][pi_profissao],
            Personagem_Patrimonio(alvo),
            pInfo[alvo][pi_mortes], pInfo[alvo][pi_kills]);
    }
    else
    {
        // Perfil de outro jogador (apenas dados públicos)
        format(info, sizeof(info),
            "Nome: %s\nSexo: %s\nIdade: %d\nOrigem: %s\nNivel: %d",
            nome, sexoStr, pInfo[alvo][pi_idade], pInfo[alvo][pi_origem],
            pInfo[alvo][pi_nivel]);
    }

    ShowPlayerDialog(playerid, DIALOG_PERFIL, DIALOG_STYLE_MSGBOX,
        "Perfil do Personagem", info, "Fechar", "");
}

// ---------------------------------------------------------------------------
// Comando /socorro (paramédico) — Req 6.3
// ---------------------------------------------------------------------------
CMD:socorrer(playerid, params[])
{
    #pragma unused params
    if(pInfo[playerid][pi_profissao] != PROF_PARAMEDICO)
        return SendClientMessage(playerid, COR_ERRO, "[SOCORRO] Somente paramédicos podem socorrer."), 1;

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    foreach(new i : Player)
    {
        if(i == playerid) continue;
        if(!pInfo[i][pi_incapacitado]) continue;
        if(!IsPlayerInRangeOfPoint(i, 3.0, px, py, pz)) continue;

        // Resgata o jogador
        pInfo[i][pi_incapacitado] = false;
        if(pInfo[i][pi_timer_incapacitado] != 0)
        {
            KillTimer(pInfo[i][pi_timer_incapacitado]);
            pInfo[i][pi_timer_incapacitado] = 0;
        }
        SetPlayerHealth(i, 50.0);
        ClearAnimations(i);

        SendClientMessage(i, COR_CONFIRMACAO, "[SOCORRO] Voce foi socorrido!");
        SendClientMessage(playerid, COR_CONFIRMACAO, "[SOCORRO] Jogador socorrido com sucesso. +75 XP");
        Progressao_AdicionarXP(playerid, XP_SOCORRER);
        return 1;
    }

    SendClientMessage(playerid, COR_ERRO, "[SOCORRO] Nenhum jogador incapacitado no raio de 3 metros.");
    return 1;
}

// ---------------------------------------------------------------------------
// Comando /perfil — Req 2.3, 2.4
// ---------------------------------------------------------------------------
CMD:perfil(playerid, params[])
{
    new nome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", nome))
    {
        // Sem argumento: exibe o próprio perfil
        Personagem_ExibirPerfil(playerid, playerid);
        return 1;
    }

    // Procura jogador por nome próximo (5m)
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME];
        GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, nome, true) == 0)
        {
            if(!IsPlayerInRangeOfPoint(i, 5.0, px, py, pz))
                return SendClientMessage(playerid, COR_ERRO, "[PERFIL] O jogador esta longe demais."), 1;
            Personagem_ExibirPerfil(playerid, i);
            return 1;
        }
    }

    SendClientMessage(playerid, COR_ERRO, "[PERFIL] Jogador nao encontrado online.");
    return 1;
}

// ---------------------------------------------------------------------------
// Comando /aparencia — Req 2.5
// ---------------------------------------------------------------------------
CMD:aparencia(playerid, params[])
{
    #pragma unused params
    // Lista simples de skins
    new lista[512];
    format(lista, sizeof(lista), "Skin 0 (Padrão)\nSkin 7\nSkin 9\nSkin 10\nSkin 11\nSkin 12");
    if(pInfo[playerid][pi_nivel_vip] >= 1)
        strcat(lista, "\n[VIP] Skin 150\n[VIP] Skin 151\n[VIP] Skin 152\n[VIP] Skin 153\n[VIP] Skin 154", sizeof(lista));

    ShowPlayerDialog(playerid, DIALOG_PERFIL + 1, DIALOG_STYLE_LIST, "Aparencia", lista, "Aplicar", "Cancelar");
    return 1;
}
