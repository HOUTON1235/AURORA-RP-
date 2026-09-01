// =============================================================================
// SISTEMA DE INVENTÁRIO — Aurora Roleplay 2026
// sistema_inventario.inc
// =============================================================================

#define MAX_ITENS_PLAYER 25

enum E_Item
{
    item_id,
    item_quantidade,
    item_nome[32]
}

new pInventario[MAX_PLAYERS][MAX_ITENS_PLAYER][E_Item];

// ============ FUNÇÕES DE INVENTÁRIO ============
stock Inventario_InicializarJogador(playerid)
{
    for(new i = 0; i < MAX_ITENS_PLAYER; i++)
    {
        pInventario[playerid][i][item_id] = 0;
        pInventario[playerid][i][item_quantidade] = 0;
        pInventario[playerid][i][item_nome][0] = EOS;
    }
}

stock Inventario_AdicionarItem(playerid, itemid, quantidade, nomeitm[])
{
    for(new i = 0; i < MAX_ITENS_PLAYER; i++)
    {
        if(pInventario[playerid][i][item_id] == itemid)
        {
            pInventario[playerid][i][item_quantidade] += quantidade;
            return 1;
        }
    }
    
    for(new i = 0; i < MAX_ITENS_PLAYER; i++)
    {
        if(pInventario[playerid][i][item_id] == 0)
        {
            pInventario[playerid][i][item_id] = itemid;
            pInventario[playerid][i][item_quantidade] = quantidade;
            strcpy(pInventario[playerid][i][item_nome], nomeitm);
            return 1;
        }
    }
    return 0;
}

stock Inventario_RemoverItem(playerid, itemid, quantidade)
{
    for(new i = 0; i < MAX_ITENS_PLAYER; i++)
    {
        if(pInventario[playerid][i][item_id] == itemid)
        {
            if(pInventario[playerid][i][item_quantidade] >= quantidade)
            {
                pInventario[playerid][i][item_quantidade] -= quantidade;
                if(pInventario[playerid][i][item_quantidade] == 0)
                {
                    pInventario[playerid][i][item_id] = 0;
                    pInventario[playerid][i][item_nome][0] = EOS;
                }
                return 1;
            }
        }
    }
    return 0;
}

stock Inventario_VerificarItem(playerid, itemid)
{
    for(new i = 0; i < MAX_ITENS_PLAYER; i++)
    {
        if(pInventario[playerid][i][item_id] == itemid)
            return pInventario[playerid][i][item_quantidade];
    }
    return 0;
}

stock Inventario_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    return 1;
}