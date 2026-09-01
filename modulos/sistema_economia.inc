// =============================================================================
// SISTEMA DE ECONOMIA — Aurora Roleplay 2026
// sistema_economia.inc
// =============================================================================

#define ECO_CASH_MAXIMO 50000
#define ECO_BANCO_MAXIMO 999999

// ============ INICIALIZAÇÃO ============
stock Economia_InicializarTodos()
{
    printf("[ECONOMIA] Sistema de economia inicializado.");
}

// ============ OPERAÇÕES DE DINHEIRO ============
stock Economia_DarDinheiro(playerid, quantia)
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new novaQuantia = pInfo[playerid][pi_dinheiro] + quantia;
    if(novaQuantia > ECO_CASH_MAXIMO) novaQuantia = ECO_CASH_MAXIMO;
    
    pInfo[playerid][pi_dinheiro] = novaQuantia;
    GivePlayerMoney(playerid, quantia);
    return 1;
}

stock Economia_RemoverDinheiro(playerid, quantia)
{
    if(!IsPlayerConnected(playerid)) return 0;
    if(pInfo[playerid][pi_dinheiro] < quantia) return 0;
    
    pInfo[playerid][pi_dinheiro] -= quantia;
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, pInfo[playerid][pi_dinheiro]);
    return 1;
}

stock Economia_ObterDinheiro(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    return pInfo[playerid][pi_dinheiro];
}

// ============ OPERAÇÕES DE BANCO ============
stock Economia_DepositarBanco(playerid, quantia)
{
    if(!IsPlayerConnected(playerid)) return 0;
    if(pInfo[playerid][pi_dinheiro] < quantia) return 0;
    
    pInfo[playerid][pi_dinheiro] -= quantia;
    pInfo[playerid][pi_banco] += quantia;
    
    if(pInfo[playerid][pi_banco] > ECO_BANCO_MAXIMO) 
        pInfo[playerid][pi_banco] = ECO_BANCO_MAXIMO;
    
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, pInfo[playerid][pi_dinheiro]);
    
    format(string, sizeof(string), "[BANCO] Depósito de R$%d realizado.", quantia);
    SendClientMessage(playerid, COR_CONFIRMACAO, string);
    return 1;
}

stock Economia_SacarBanco(playerid, quantia)
{
    if(!IsPlayerConnected(playerid)) return 0;
    if(pInfo[playerid][pi_banco] < quantia) return 0;
    
    new novaQuantia = pInfo[playerid][pi_dinheiro] + quantia;
    if(novaQuantia > ECO_CASH_MAXIMO) novaQuantia = ECO_CASH_MAXIMO;
    
    pInfo[playerid][pi_banco] -= quantia;
    pInfo[playerid][pi_dinheiro] = novaQuantia;
    
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, pInfo[playerid][pi_dinheiro]);
    
    format(string, sizeof(string), "[BANCO] Saque de R$%d realizado.", quantia);
    SendClientMessage(playerid, COR_CONFIRMACAO, string);
    return 1;
}

stock Economia_ObterBanco(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    return pInfo[playerid][pi_banco];
}

// ============ CICLOS DE ECONOMIA ============
stock Economia_CicloSalarios()
{
    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        
        new salario = 0;
        switch(pInfo[i][pi_profissao])
        {
            case PROF_POLICIAL: salario = 5000;
            case PROF_PARAMEDICO: salario = 4500;
            case PROF_BOMBEIRO: salario = 4500;
            case PROF_MECANICO: salario = 3500;
            case PROF_TAXISTA: salario = 3000;
            case PROF_MOTORISTA_ONIBUS: salario = 2500;
            case PROF_EMPRESARIO: salario = 6000;
            case PROF_ADVOGADO: salario = 5500;
            case PROF_JORNALISTA: salario = 4000;
        }
        
        if(salario > 0)
        {
            pInfo[i][pi_banco] += salario;
            format(string, sizeof(string), "[SALÁRIO] Você recebeu R$%d em sua conta.", salario);
            SendClientMessage(i, COR_CONFIRMACAO, string);
        }
    }
}

stock Economia_CicloImpostos()
{
    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;
        
        new imposto = pInfo[i][pi_banco] / 100; // 1% por ciclo
        if(imposto > 0)
        {
            pInfo[i][pi_banco] -= imposto;
            format(string, sizeof(string), "[IMPOSTO] R$%d foram descontados.", imposto);
            SendClientMessage(i, 0xFF3B30FF, string);
        }
    }
}