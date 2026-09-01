// =============================================================================
//   sistema_login.inc — Registro e autenticação
//   Aurora Roleplay 2026
//   Requirements: 1.1–1.8
// =============================================================================

#define LOGIN_MAX_TENTATIVAS    5
#define LOGIN_SENHA_MIN         6
#define LOGIN_SENHA_MAX         20

// ---------------------------------------------------------------------------
// Hash simples de senha (djb2) — substituto de MD5 para persistência local
// ---------------------------------------------------------------------------
static stock Login_HashSenha(const senha[], saida[33])
{
    new hash = 5381;
    for(new i = 0; senha[i] != EOS; i++)
        hash = ((hash << 5) + hash) + senha[i];

    // Converte para string hexadecimal de 8 chars (padding com zeros)
    format(saida, 33, "%08x000000000000000000000000", hash);
}
stock Login_ContaExiste(playerid)
{
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);
    return DOF2_FileExists(caminho);
}

// ---------------------------------------------------------------------------
// Exibe registro ou login conforme existência da conta — Req 1.1, 1.5
// ---------------------------------------------------------------------------
stock Login_ExibirRegistroOuLogin(playerid)
{
    if(Login_ContaExiste(playerid))
        Login_ExibirLogin(playerid);
    else
        Login_ExibirRegistro(playerid);
}

// ---------------------------------------------------------------------------
// Dialog de registro — Req 1.1
// ---------------------------------------------------------------------------
stock Login_ExibirRegistro(playerid)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new titulo[64];
    format(titulo, sizeof(titulo), "Registrar — %s", nome);

    ShowPlayerDialog(playerid, DIALOG_REGISTRO, DIALOG_STYLE_INPUT,
        titulo,
        "Bem-vindo ao Aurora Roleplay!\nDigite sua senha (6-20 caracteres).\nFormato: senha|confirmar|email",
        "Registrar", "Sair");
}

// ---------------------------------------------------------------------------
// Dialog de login — Req 1.5
// ---------------------------------------------------------------------------
stock Login_ExibirLogin(playerid)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new titulo[64];
    format(titulo, sizeof(titulo), "Login — %s", nome);

    ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD,
        titulo,
        "Bem-vindo de volta ao Aurora Roleplay!\n\nDigite sua senha:",
        "Entrar", "Sair");
}

// ---------------------------------------------------------------------------
// Processar registro — Req 1.2, 1.3, 1.4
// ---------------------------------------------------------------------------
stock Login_ProcessarRegistro(playerid, const input[])
{
    // Formato esperado: "senha|confirmar|email"
    new senha[32], confirma[32], email[64];

    if(sscanf(input, "p<|>s[32]s[32]s[64]", senha, confirma, email))
    {
        SendClientMessage(playerid, COR_ERRO, "[REGISTRO] Formato invalido. Use: senha|confirmar|email");
        Login_ExibirRegistro(playerid);
        return;
    }

    // Valida comprimento da senha — Req 1.3
    new len = strlen(senha);
    if(len < LOGIN_SENHA_MIN || len > LOGIN_SENHA_MAX)
    {
        SendClientMessage(playerid, COR_ERRO, "[REGISTRO] Senha deve ter entre 6 e 20 caracteres.");
        Login_ExibirRegistro(playerid);
        return;
    }

    // Valida confirmação — Req 1.4
    if(strcmp(senha, confirma, true) != 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[REGISTRO] As senhas nao coincidem.");
        Login_ExibirRegistro(playerid);
        return;
    }

    // Valida e-mail mínimo (contém @)
    if(strfind(email, "@") == -1)
    {
        SendClientMessage(playerid, COR_ERRO, "[REGISTRO] E-mail invalido.");
        Login_ExibirRegistro(playerid);
        return;
    }

    // Hash MD5 e criação do arquivo — Req 1.2
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    new hash[33];
    Login_HashSenha(senha, hash);

    DOF2_SetString(caminho, "Senha",        hash);
    DOF2_SetString(caminho, "Email",        email);
    DOF2_SetInt   (caminho, "Nivel",        1);
    DOF2_SetInt   (caminho, "XP",           0);
    DOF2_SetInt   (caminho, "Reputacao",    0);
    DOF2_SetInt   (caminho, "Skin",         0);
    DOF2_SetInt   (caminho, "Sexo",         -1);
    DOF2_SetInt   (caminho, "Idade",        0);
    DOF2_SetString(caminho, "Origem",       "");
    DOF2_SetInt   (caminho, "Dinheiro",     500);
    DOF2_SetInt   (caminho, "Banco",        0);
    DOF2_SetInt   (caminho, "Fome",         100);
    DOF2_SetInt   (caminho, "Sede",         100);
    DOF2_SetInt   (caminho, "NivelAdmin",   0);
    DOF2_SetInt   (caminho, "NivelVIP",     0);
    DOF2_SetInt   (caminho, "VipExpira",    0);
    DOF2_SetInt   (caminho, "OrgId",        -1);
    DOF2_SetInt   (caminho, "EmpId",        -1);
    DOF2_SetInt   (caminho, "CasaId",       -1);
    DOF2_SetInt   (caminho, "Mortes",       0);
    DOF2_SetInt   (caminho, "Kills",        0);
    DOF2_SetInt   (caminho, "HorasJogadas", 0);
    DOF2_SetInt   (caminho, "Conquistas",   0);
    DOF2_SetFloat (caminho, "PosX",         1958.33);
    DOF2_SetFloat (caminho, "PosY",         -1343.07);
    DOF2_SetFloat (caminho, "PosZ",         15.36);
    DOF2_SetFloat (caminho, "Angulo",       270.0);
    DOF2_SetInt   (caminho, "Interior",     0);
    DOF2_SetInt   (caminho, "VirtualWorld", 0);
    DOF2_SaveFile(caminho);

    // Carrega os dados e inicia criação de personagem — Req 1.8
    Login_CarregarDados(playerid);
    pInfo[playerid][pi_logado] = true;
    Login_ResetarFalhas(playerid);

    SendClientMessage(playerid, COR_CONFIRMACAO, "[REGISTRO] Conta criada com sucesso! Bem-vindo ao Aurora Roleplay.");
    Personagem_IniciarCriacao(playerid);
}

// ---------------------------------------------------------------------------
// Verificar senha — Req 1.6, 1.7
// ---------------------------------------------------------------------------
stock Login_VerificarSenha(playerid, const senha[])
{
    new hash[33];
    Login_HashSenha(senha, hash);
    return (strcmp(pInfo[playerid][pi_senha], hash) == 0) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Carregar dados do ini para pInfo
// ---------------------------------------------------------------------------
stock Login_CarregarDados(playerid)
{
    new nome[MAX_PLAYER_NAME], caminho[64];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    format(caminho, sizeof(caminho), "Players/%s.ini", nome);

    if(!DOF2_FileExists(caminho)) return;

    DOF2_GetString(caminho, "Senha",   pInfo[playerid][pi_senha],  24);
    DOF2_GetString(caminho, "Email",   pInfo[playerid][pi_email],  64);
    DOF2_GetString(caminho, "Origem",  pInfo[playerid][pi_origem], 32);

    pInfo[playerid][pi_nivel]        = DOF2_GetInt(caminho, "Nivel");
    pInfo[playerid][pi_xp]           = DOF2_GetInt(caminho, "XP");
    pInfo[playerid][pi_reputacao]    = DOF2_GetInt(caminho, "Reputacao");
    pInfo[playerid][pi_skin]         = DOF2_GetInt(caminho, "Skin");
    pInfo[playerid][pi_sexo]         = DOF2_GetInt(caminho, "Sexo");
    pInfo[playerid][pi_idade]        = DOF2_GetInt(caminho, "Idade");
    pInfo[playerid][pi_dinheiro]     = DOF2_GetInt(caminho, "Dinheiro");
    pInfo[playerid][pi_banco]        = DOF2_GetInt(caminho, "Banco");
    pInfo[playerid][pi_fome]         = DOF2_GetInt(caminho, "Fome");
    pInfo[playerid][pi_sede]         = DOF2_GetInt(caminho, "Sede");
    pInfo[playerid][pi_nivel_admin]  = DOF2_GetInt(caminho, "NivelAdmin");
    pInfo[playerid][pi_nivel_vip]    = DOF2_GetInt(caminho, "NivelVIP");
    pInfo[playerid][pi_vip_expira]   = DOF2_GetInt(caminho, "VipExpira");
    pInfo[playerid][pi_org_id]       = DOF2_GetInt(caminho, "OrgId");
    pInfo[playerid][pi_org_rank]     = DOF2_GetInt(caminho, "OrgRank");
    pInfo[playerid][pi_emp_id]       = DOF2_GetInt(caminho, "EmpId");
    pInfo[playerid][pi_casa_id]      = DOF2_GetInt(caminho, "CasaId");
    pInfo[playerid][pi_mortes]       = DOF2_GetInt(caminho, "Mortes");
    pInfo[playerid][pi_kills]        = DOF2_GetInt(caminho, "Kills");
    pInfo[playerid][pi_horas_jogadas] = DOF2_GetInt(caminho, "HorasJogadas");
    pInfo[playerid][pi_profissao]    = E_Profissao:DOF2_GetInt(caminho, "Profissao");

    pInfo[playerid][pi_pos_x]        = DOF2_GetFloat(caminho, "PosX");
    pInfo[playerid][pi_pos_y]        = DOF2_GetFloat(caminho, "PosY");
    pInfo[playerid][pi_pos_z]        = DOF2_GetFloat(caminho, "PosZ");
    pInfo[playerid][pi_angulo]       = DOF2_GetFloat(caminho, "Angulo");
    pInfo[playerid][pi_interior]     = DOF2_GetInt(caminho, "Interior");
    pInfo[playerid][pi_virtual_world] = DOF2_GetInt(caminho, "VirtualWorld");

    // Garante invariante de cash
    if(pInfo[playerid][pi_dinheiro] > ECO_CASH_MAXIMO)
        pInfo[playerid][pi_dinheiro] = ECO_CASH_MAXIMO;
}

// ---------------------------------------------------------------------------
// Falhas de login — Req 1.7
// ---------------------------------------------------------------------------
stock Login_RegistrarFalha(playerid)
{
    pInfo[playerid][pi_tentativas_login]++;
    if(pInfo[playerid][pi_tentativas_login] >= LOGIN_MAX_TENTATIVAS)
    {
        SendClientMessage(playerid, COR_ERRO, "[LOGIN] Muitas tentativas incorretas. Voce foi expulso.");
        Kick(playerid);
    }
    else
    {
        new msg[64];
        format(msg, sizeof(msg), "[LOGIN] Senha incorreta. Tentativa %d/%d.",
            pInfo[playerid][pi_tentativas_login], LOGIN_MAX_TENTATIVAS);
        SendClientMessage(playerid, COR_ERRO, msg);
        Login_ExibirLogin(playerid);
    }
}

stock Login_ResetarFalhas(playerid)
{
    pInfo[playerid][pi_tentativas_login] = 0;
}

// ---------------------------------------------------------------------------
// Roteamento de dialogs — chamado de OnDialogResponse em ARP.pwn
// ---------------------------------------------------------------------------
stock Login_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused listitem

    if(dialogid == DIALOG_REGISTRO)
    {
        if(!response) { Kick(playerid); return 1; }
        Login_ProcessarRegistro(playerid, inputtext);
        return 1;
    }

    if(dialogid == DIALOG_LOGIN)
    {
        if(!response) { Kick(playerid); return 1; }

        if(!Login_VerificarSenha(playerid, inputtext))
        {
            Login_RegistrarFalha(playerid);
            return 1;
        }

        // Login bem-sucedido — Req 1.6
        Login_ResetarFalhas(playerid);
        pInfo[playerid][pi_logado] = true;

        // Posiciona no último local salvo
        SetPlayerPos(playerid,
            pInfo[playerid][pi_pos_x],
            pInfo[playerid][pi_pos_y],
            pInfo[playerid][pi_pos_z]);
        SetPlayerFacingAngle(playerid, pInfo[playerid][pi_angulo]);
        SetPlayerInterior(playerid, pInfo[playerid][pi_interior]);
        SetPlayerVirtualWorld(playerid, pInfo[playerid][pi_virtual_world]);

        // Sincroniza dinheiro exibido pelo SA-MP
        ResetPlayerMoney(playerid);
        GivePlayerMoney(playerid, pInfo[playerid][pi_dinheiro]);

        SendClientMessage(playerid, COR_CONFIRMACAO, "[LOGIN] Bem-vindo de volta ao Aurora Roleplay!");
        Personagem_Carregar(playerid);
        return 1;
    }

    return 0;
}
