// =============================================================================
// SISTEMA DE LOGIN — Aurora Roleplay 2026
// sistema_login.inc
// =============================================================================

stock Login_ExibirRegistroOuLogin(playerid)
{
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, sizeof(nome));
    
    // Aqui você verificaria se existe arquivo do jogador
    // Por enquanto, mostra registro
    ShowPlayerDialog(playerid, DIALOG_REGISTRO, DIALOG_STYLE_PASSWORD,
        "Bem-vindo ao Aurora Roleplay",
        "Digite uma senha para seu personagem:\n(Mín. 6 caracteres)",
        "Registrar", "Cancelar");
    return 1;
}

stock Login_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    if(!response) return Kick(playerid);
    
    if(strlen(inputtext) < 6)
    {
        SendClientMessage(playerid, COR_ERRO, "[ERRO] Senha muito curta (mín. 6 caracteres)");
        return Login_ExibirRegistroOuLogin(playerid);
    }
    
    strcpy(pInfo[playerid][pi_senha], inputtext);
    pInfo[playerid][pi_logado] = true;
    pInfo[playerid][pi_nivel] = 1;
    pInfo[playerid][pi_dinheiro] = 1000;
    pInfo[playerid][pi_banco] = 5000;
    pInfo[playerid][pi_sexo] = 0;
    pInfo[playerid][pi_idade] = 18;
    pInfo[playerid][pi_profissao] = PROF_DESEMPREGADO;
    
    SendClientMessage(playerid, COR_CONFIRMACAO, "[SUCESSO] Bem-vindo ao Aurora Roleplay!");
    SetPlayerPos(playerid, 1958.3783, 1343.1572, 15.3746); // Centro LS
    
    return 1;
}