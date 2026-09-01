// =============================================================================
//   sistema_inventario.inc — Itens, peso e categorias
//   Aurora Roleplay 2026
//   Requirements: 5.1–5.8
// =============================================================================

#define INV_MAX_ITENS       20
#define INV_PESO_PADRAO     15.0
#define INV_PESO_VIP2       25.0

enum E_CategoriaItem
{
    CAT_CONSUMIVEL,
    CAT_FERRAMENTA,
    CAT_ARMA,
    CAT_MUNICAO,
    CAT_DOCUMENTO,
    CAT_ELETRONICO,
    CAT_MISC
}

enum E_Item
{
    item_id,
    item_nome[32],
    item_quantidade,
    Float:item_peso_unitario,
    E_CategoriaItem:item_categoria
}

new pInventario[MAX_PLAYERS][INV_MAX_ITENS][E_Item];
new Float:pPesoTotal[MAX_PLAYERS];

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
stock Float:Inventario_CapacidadeMaxima(playerid)
{
    if(pInfo[playerid][pi_nivel_vip] >= 2) return INV_PESO_VIP2;
    return INV_PESO_PADRAO;
}

stock Float:Inventario_PesoAtual(playerid)
{
    return pPesoTotal[playerid];
}

stock Inventario_PodeCaber(playerid, Float:peso_item)
{
    return (pPesoTotal[playerid] + peso_item <= Inventario_CapacidadeMaxima(playerid)) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Adicionar item — Req 5.2
// ---------------------------------------------------------------------------
stock Inventario_AdicionarItem(playerid, id, quantidade, Float:peso_unitario, E_CategoriaItem:categoria, const nome[32] = "Item")
{
    new Float:pesoTotal = peso_unitario * float(quantidade);

    if(!Inventario_PodeCaber(playerid, pesoTotal))
    {
        new msg[128];
        format(msg, sizeof(msg), "[INV] Sem espaco. Peso atual: %.1fkg / %.1fkg. Item: %.1fkg.",
            pPesoTotal[playerid], Inventario_CapacidadeMaxima(playerid), pesoTotal);
        SendClientMessage(playerid, COR_ERRO, msg);
        return 0;
    }

    // Verifica se item já existe no inventário (stack)
    for(new s = 0; s < INV_MAX_ITENS; s++)
    {
        if(pInventario[playerid][s][item_id] == id)
        {
            pInventario[playerid][s][item_quantidade] += quantidade;
            pPesoTotal[playerid] += pesoTotal;
            return 1;
        }
    }

    // Slot vazio
    for(new s = 0; s < INV_MAX_ITENS; s++)
    {
        if(pInventario[playerid][s][item_id] == 0)
        {
            pInventario[playerid][s][item_id]           = id;
            pInventario[playerid][s][item_quantidade]   = quantidade;
            pInventario[playerid][s][item_peso_unitario] = peso_unitario;
            pInventario[playerid][s][item_categoria]    = categoria;
            strcopy(pInventario[playerid][s][item_nome], nome, 32);
            pPesoTotal[playerid] += pesoTotal;
            return 1;
        }
    }

    SendClientMessage(playerid, COR_ERRO, "[INV] Inventario cheio (20 itens).");
    return 0;
}

// ---------------------------------------------------------------------------
// Descartar item — Req 5.5
// ---------------------------------------------------------------------------
stock Inventario_DescartarItem(playerid, slot)
{
    if(slot < 0 || slot >= INV_MAX_ITENS || pInventario[playerid][slot][item_id] == 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[INV] Slot invalido.");
        return;
    }

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    // Cria pickup no chão
    CreatePickup(1210, 1, px, py, pz);

    // Remove do inventário
    pPesoTotal[playerid] -= pInventario[playerid][slot][item_peso_unitario] * float(pInventario[playerid][slot][item_quantidade]);
    if(pPesoTotal[playerid] < 0.0) pPesoTotal[playerid] = 0.0;

    pInventario[playerid][slot][item_id]        = 0;
    pInventario[playerid][slot][item_quantidade] = 0;
    pInventario[playerid][slot][item_peso_unitario] = 0.0;
    pInventario[playerid][slot][item_nome][0]   = EOS;

    SendClientMessage(playerid, COR_CONFIRMACAO, "[INV] Item descartado no chao.");
}

// ---------------------------------------------------------------------------
// Usar item consumível — Req 5.6
// ---------------------------------------------------------------------------
stock Inventario_UsarItem(playerid, slot)
{
    if(slot < 0 || slot >= INV_MAX_ITENS || pInventario[playerid][slot][item_id] == 0)
    {
        SendClientMessage(playerid, COR_ERRO, "[INV] Slot invalido.");
        return;
    }

    if(pInventario[playerid][slot][item_categoria] != CAT_CONSUMIVEL)
    {
        SendClientMessage(playerid, COR_ERRO, "[INV] Este item nao pode ser consumido.");
        return;
    }

    new id = pInventario[playerid][slot][item_id];

    switch(id)
    {
        case 1: // Lanche — fome +30
        {
            pInfo[playerid][pi_fome] += 30;
            if(pInfo[playerid][pi_fome] > 100) pInfo[playerid][pi_fome] = 100;
            SendClientMessage(playerid, COR_CONFIRMACAO, "[INV] Voce comeu um lanche. Fome +30.");
        }
        case 2: // Agua — sede +40
        {
            pInfo[playerid][pi_sede] += 40;
            if(pInfo[playerid][pi_sede] > 100) pInfo[playerid][pi_sede] = 100;
            SendClientMessage(playerid, COR_CONFIRMACAO, "[INV] Voce bebeu agua. Sede +40.");
        }
        case 3: // Kit medico — HP +40 (max 100)
        {
            new Float:hp;
            GetPlayerHealth(playerid, hp);
            hp += 40.0;
            if(hp > 100.0) hp = 100.0;
            SetPlayerHealth(playerid, hp);
            SendClientMessage(playerid, COR_CONFIRMACAO, "[INV] Kit medico usado. HP +40.");
        }
        default:
            SendClientMessage(playerid, COR_ERRO, "[INV] Efeito desconhecido para este item.");
    }

    // Consome 1 unidade
    pInventario[playerid][slot][item_quantidade]--;
    if(pInventario[playerid][slot][item_quantidade] <= 0)
    {
        pPesoTotal[playerid] -= pInventario[playerid][slot][item_peso_unitario];
        if(pPesoTotal[playerid] < 0.0) pPesoTotal[playerid] = 0.0;
        pInventario[playerid][slot][item_id]        = 0;
        pInventario[playerid][slot][item_nome][0]   = EOS;
    }
}

// ---------------------------------------------------------------------------
// Limpar inventário
// ---------------------------------------------------------------------------
stock Inventario_LimparInventario(playerid)
{
    for(new s = 0; s < INV_MAX_ITENS; s++)
    {
        pInventario[playerid][s][item_id]           = 0;
        pInventario[playerid][s][item_quantidade]   = 0;
        pInventario[playerid][s][item_peso_unitario] = 0.0;
        pInventario[playerid][s][item_nome][0]      = EOS;
    }
    pPesoTotal[playerid] = 0.0;
}

// ---------------------------------------------------------------------------
// Exibir dialog — Req 5.4
// ---------------------------------------------------------------------------
stock Inventario_ExibirDialog(playerid)
{
    new lista[1024], linha[128];
    lista[0] = EOS;

    for(new s = 0; s < INV_MAX_ITENS; s++)
    {
        if(pInventario[playerid][s][item_id] == 0) continue;
        format(linha, sizeof(linha), "[%d] %s x%d (%.1fkg cada)\n",
            s,
            pInventario[playerid][s][item_nome],
            pInventario[playerid][s][item_quantidade],
            pInventario[playerid][s][item_peso_unitario]);
        strcat(lista, linha, sizeof(lista));
    }

    if(lista[0] == EOS) strcopy(lista, "Inventario vazio.", sizeof(lista));

    new titulo[64];
    format(titulo, sizeof(titulo), "Inventario — %.1f / %.1f kg",
        pPesoTotal[playerid], Inventario_CapacidadeMaxima(playerid));

    ShowPlayerDialog(playerid, DIALOG_INVENTARIO, DIALOG_STYLE_LIST,
        titulo, lista, "Usar/Descartar", "Fechar");
}

stock Inventario_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused dialogid, inputtext
    if(!response) return 1;

    // listitem = slot selecionado
    // Por simplicidade: usa o item no slot
    if(listitem >= 0 && listitem < INV_MAX_ITENS)
        Inventario_UsarItem(playerid, listitem);

    return 1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:inventario(playerid, params[])
{
    #pragma unused params
    Inventario_ExibirDialog(playerid);
    return 1;
}

CMD:descartar(playerid, params[])
{
    new slot;
    if(sscanf(params, "i", slot))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /descartar [slot]"), 1;
    Inventario_DescartarItem(playerid, slot);
    return 1;
}

CMD:usar(playerid, params[])
{
    new slot;
    if(sscanf(params, "i", slot))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /usar [slot]"), 1;
    Inventario_UsarItem(playerid, slot);
    return 1;
}
