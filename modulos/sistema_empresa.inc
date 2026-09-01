// =============================================================================
//   sistema_empresa.inc — Comércio, estoque e funcionários
//   Aurora Roleplay 2026
//   Requirements: 9.1–9.10, 20.6, 21.2, 21.7
// =============================================================================

#define MAX_EMPRESAS        1000
#define EMP_MAX_HISTORICO   100
#define EMP_MAX_PRODUTOS    20
#define EMP_MAX_FUNCIONARIOS 30

enum E_Empresa
{
    emp_id,
    emp_nome[64],
    emp_tipo[32],
    emp_dono[MAX_PLAYER_NAME],
    emp_caixa,
    emp_pickup,
    Text3D:emp_label,
    Float:emp_pos[3],
    Float:emp_interior_pos[3],
    emp_interior,
    emp_ultimo_ativo   // timestamp — para verificar inatividade
}

enum E_EstoqueItem
{
    estoque_item_id,
    estoque_nome[32],
    estoque_quantidade,
    estoque_max,
    estoque_preco
}

new EmpData[MAX_EMPRESAS][E_Empresa];
new EmpEstoque[MAX_EMPRESAS][EMP_MAX_PRODUTOS][E_EstoqueItem];
new EmpFuncionarios[MAX_EMPRESAS][EMP_MAX_FUNCIONARIOS][MAX_PLAYER_NAME];
new EmpFuncionarioSalario[MAX_EMPRESAS][EMP_MAX_FUNCIONARIOS];
new gTotalEmpresas = 0;

// ---------------------------------------------------------------------------
// Salvar empresa — Req 20.6
// ---------------------------------------------------------------------------
stock Empresa_Salvar(id)
{
    if(id < 0 || id >= MAX_EMPRESAS) return;
    if(strlen(EmpData[id][emp_nome]) == 0) return;

    new caminho[64];
    format(caminho, sizeof(caminho), "Empresas/%d.ini", id);

    DOF2_SetInt   (caminho, "ID",          id);
    DOF2_SetString(caminho, "Nome",        EmpData[id][emp_nome]);
    DOF2_SetString(caminho, "Tipo",        EmpData[id][emp_tipo]);
    DOF2_SetString(caminho, "Dono",        EmpData[id][emp_dono]);
    DOF2_SetInt   (caminho, "Caixa",       EmpData[id][emp_caixa]);
    DOF2_SetInt   (caminho, "Interior",    EmpData[id][emp_interior]);
    DOF2_SetFloat (caminho, "PosX",        EmpData[id][emp_pos][0]);
    DOF2_SetFloat (caminho, "PosY",        EmpData[id][emp_pos][1]);
    DOF2_SetFloat (caminho, "PosZ",        EmpData[id][emp_pos][2]);
    DOF2_SetFloat (caminho, "IntX",        EmpData[id][emp_interior_pos][0]);
    DOF2_SetFloat (caminho, "IntY",        EmpData[id][emp_interior_pos][1]);
    DOF2_SetFloat (caminho, "IntZ",        EmpData[id][emp_interior_pos][2]);
    DOF2_SetInt   (caminho, "UltimoAtivo", EmpData[id][emp_ultimo_ativo]);

    // Salva estoque
    for(new p = 0; p < EMP_MAX_PRODUTOS; p++)
    {
        if(EmpEstoque[id][p][estoque_item_id] == 0) continue;
        new chave[32];
        format(chave, sizeof(chave), "Produto%d_Nome",  p); DOF2_SetString(caminho, chave, EmpEstoque[id][p][estoque_nome]);
        format(chave, sizeof(chave), "Produto%d_Qtd",   p); DOF2_SetInt   (caminho, chave, EmpEstoque[id][p][estoque_quantidade]);
        format(chave, sizeof(chave), "Produto%d_Max",   p); DOF2_SetInt   (caminho, chave, EmpEstoque[id][p][estoque_max]);
        format(chave, sizeof(chave), "Produto%d_Preco", p); DOF2_SetInt   (caminho, chave, EmpEstoque[id][p][estoque_preco]);
        format(chave, sizeof(chave), "Produto%d_ID",    p); DOF2_SetInt   (caminho, chave, EmpEstoque[id][p][estoque_item_id]);
    }

    // Salva funcionários
    for(new f = 0; f < EMP_MAX_FUNCIONARIOS; f++)
    {
        if(strlen(EmpFuncionarios[id][f]) == 0) continue;
        new chave[32];
        format(chave, sizeof(chave), "Func%d_Nome",    f); DOF2_SetString(caminho, chave, EmpFuncionarios[id][f]);
        format(chave, sizeof(chave), "Func%d_Salario", f); DOF2_SetInt   (caminho, chave, EmpFuncionarioSalario[id][f]);
    }

    DOF2_SaveFile(caminho);
}

// ---------------------------------------------------------------------------
// Carregar empresa
// ---------------------------------------------------------------------------
stock Empresa_Carregar(id)
{
    new caminho[64];
    format(caminho, sizeof(caminho), "Empresas/%d.ini", id);
    if(!DOF2_FileExists(caminho)) return;

    EmpData[id][emp_id] = id;
    DOF2_GetString(caminho, "Nome",  EmpData[id][emp_nome], 64);
    DOF2_GetString(caminho, "Tipo",  EmpData[id][emp_tipo], 32);
    DOF2_GetString(caminho, "Dono",  EmpData[id][emp_dono], MAX_PLAYER_NAME);
    EmpData[id][emp_caixa]       = DOF2_GetInt(caminho, "Caixa");
    EmpData[id][emp_interior]    = DOF2_GetInt(caminho, "Interior");
    EmpData[id][emp_pos][0]      = DOF2_GetFloat(caminho, "PosX");
    EmpData[id][emp_pos][1]      = DOF2_GetFloat(caminho, "PosY");
    EmpData[id][emp_pos][2]      = DOF2_GetFloat(caminho, "PosZ");
    EmpData[id][emp_interior_pos][0] = DOF2_GetFloat(caminho, "IntX");
    EmpData[id][emp_interior_pos][1] = DOF2_GetFloat(caminho, "IntY");
    EmpData[id][emp_interior_pos][2] = DOF2_GetFloat(caminho, "IntZ");
    EmpData[id][emp_ultimo_ativo] = DOF2_GetInt(caminho, "UltimoAtivo");

    for(new p = 0; p < EMP_MAX_PRODUTOS; p++)
    {
        new chave[32];
        format(chave, sizeof(chave), "Produto%d_ID", p);
        EmpEstoque[id][p][estoque_item_id] = DOF2_GetInt(caminho, chave);
        if(EmpEstoque[id][p][estoque_item_id] == 0) continue;
        format(chave, sizeof(chave), "Produto%d_Nome",  p); DOF2_GetString(caminho, chave, EmpEstoque[id][p][estoque_nome], 32);
        format(chave, sizeof(chave), "Produto%d_Qtd",   p); EmpEstoque[id][p][estoque_quantidade] = DOF2_GetInt(caminho, chave);
        format(chave, sizeof(chave), "Produto%d_Max",   p); EmpEstoque[id][p][estoque_max]        = DOF2_GetInt(caminho, chave);
        format(chave, sizeof(chave), "Produto%d_Preco", p); EmpEstoque[id][p][estoque_preco]      = DOF2_GetInt(caminho, chave);
    }

    for(new f = 0; f < EMP_MAX_FUNCIONARIOS; f++)
    {
        new chave[32];
        format(chave, sizeof(chave), "Func%d_Nome", f);
        DOF2_GetString(caminho, chave, EmpFuncionarios[id][f], MAX_PLAYER_NAME);
        format(chave, sizeof(chave), "Func%d_Salario", f);
        EmpFuncionarioSalario[id][f] = DOF2_GetInt(caminho, chave);
    }

    // Cria pickup e label
    EmpData[id][emp_pickup] = CreatePickup(1239, 1, EmpData[id][emp_pos][0], EmpData[id][emp_pos][1], EmpData[id][emp_pos][2]);
    new texto[128];
    format(texto, sizeof(texto), "%s\nTipo: %s\nDono: %s", EmpData[id][emp_nome], EmpData[id][emp_tipo], EmpData[id][emp_dono]);
    EmpData[id][emp_label] = Create3DTextLabel(texto, COR_DESTAQUE,
        EmpData[id][emp_pos][0], EmpData[id][emp_pos][1], EmpData[id][emp_pos][2] + 1.0, 15.0, 0);

    gTotalEmpresas++;
}

stock Empresa_CarregarTodas()
{
    gTotalEmpresas = 0;
    for(new id = 0; id < MAX_EMPRESAS; id++)
        Empresa_Carregar(id);
    printf("[EMPRESAS] %d empresas carregadas.", gTotalEmpresas);
}

// ---------------------------------------------------------------------------
// Registrar histórico financeiro — Req 9.8
// ---------------------------------------------------------------------------
stock Empresa_RegistrarHistorico(id, tipo[], valor, envolvido[], timestamp)
{
    new caminho[64], chave[64];
    format(caminho, sizeof(caminho), "Empresas/%d.ini", id);
    format(chave, sizeof(chave), "Hist_%d_%s", timestamp, tipo);
    new entrada[128];
    format(entrada, sizeof(entrada), "%s|%d|%s", tipo, valor, envolvido);
    DOF2_SetString(caminho, chave, entrada);
    DOF2_SaveFile(caminho);
}

// ---------------------------------------------------------------------------
// Exibir painel de gerenciamento — Req 9.2
// ---------------------------------------------------------------------------
stock Empresa_ExibirPainel(playerid, id)
{
    new info[512];
    format(info, sizeof(info),
        "Empresa: %s\nTipo: %s\nDono: %s\nCaixa: R$%d\n\nSelecione uma aba:",
        EmpData[id][emp_nome], EmpData[id][emp_tipo],
        EmpData[id][emp_dono], EmpData[id][emp_caixa]);

    new abas[] = "Caixa\nEstoque\nFuncionarios\nHistorico\nConfiguracoes";
    ShowPlayerDialog(playerid, DIALOG_EMPRESA_PAINEL, DIALOG_STYLE_LIST,
        "Gerenciamento da Empresa", abas, "Selecionar", "Fechar");
}

// ---------------------------------------------------------------------------
// Vender produto — Req 9.6, 21.2
// ---------------------------------------------------------------------------
stock Empresa_VenderProduto(playerid, id, slot)
{
    if(slot < 0 || slot >= EMP_MAX_PRODUTOS) { SendClientMessage(playerid, COR_ERRO, "[EMPRESA] Slot invalido."); return; }
    if(EmpEstoque[id][slot][estoque_quantidade] <= 0) { SendClientMessage(playerid, COR_ERRO, "[EMPRESA] Produto sem estoque."); return; }

    new preco = EmpEstoque[id][slot][estoque_preco];
    if(!Economia_RetirarDinheiro(playerid, preco)) { SendClientMessage(playerid, COR_ERRO, "[EMPRESA] Dinheiro em mao insuficiente."); return; }

    // Atualiza estoque e caixa atomicamente
    EmpEstoque[id][slot][estoque_quantidade]--;
    EmpData[id][emp_caixa] += preco;

    // Entrega item ao comprador
    new nomeItem[32];
    strcopy(nomeItem, EmpEstoque[id][slot][estoque_nome], 32);
    Inventario_AdicionarItem(playerid,
        EmpEstoque[id][slot][estoque_item_id],
        1,
        0.5,
        CAT_MISC,
        nomeItem);

    new nome[MAX_PLAYER_NAME], ts = gettime();
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    new tipo[] = "Venda";
    Empresa_RegistrarHistorico(id, tipo, preco, nome, ts);

    // Verifica estoque baixo
    Empresa_VerificarEstoque(id);
    Empresa_Salvar(id);

    new msg[64];
    format(msg, sizeof(msg), "[EMPRESA] Comprou %s por R$%d.", EmpEstoque[id][slot][estoque_nome], preco);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Pagar salários — Req 9.4, 9.5, 21.7
// ---------------------------------------------------------------------------
stock Empresa_PagarSalarios(id)
{
    new totalSalarios = 0;
    for(new f = 0; f < EMP_MAX_FUNCIONARIOS; f++)
        if(strlen(EmpFuncionarios[id][f]) > 0)
            totalSalarios += EmpFuncionarioSalario[id][f];

    if(totalSalarios == 0) return 0;

    // Req 21.7: tributos primeiro (10% do caixa como imposto de folha)
    new tributo = floatround(float(EmpData[id][emp_caixa]) * 0.10, floatround_floor);
    if(tributo > EmpData[id][emp_caixa]) tributo = EmpData[id][emp_caixa];
    EmpData[id][emp_caixa] -= tributo;

    new disponivel = EmpData[id][emp_caixa];
    new Float:proporcao = (disponivel >= totalSalarios) ? 1.0 : float(disponivel) / float(totalSalarios);

    new totalPago = 0;
    for(new f = 0; f < EMP_MAX_FUNCIONARIOS; f++)
    {
        if(strlen(EmpFuncionarios[id][f]) == 0) continue;
        new salario = floatround(float(EmpFuncionarioSalario[id][f]) * proporcao, floatround_floor);
        if(salario <= 0) continue;

        // Credita no banco do funcionário se estiver online
        foreach(new i : Player)
        {
            new n[MAX_PLAYER_NAME];
            GetPlayerName(i, n, MAX_PLAYER_NAME);
            if(strcmp(n, EmpFuncionarios[id][f], true) == 0)
            {
                Economia_CreditarBanco(i, salario);
                new msg[64];
                format(msg, sizeof(msg), "[EMPRESA] Salario de R$%d recebido de %s.", salario, EmpData[id][emp_nome]);
                SendClientMessage(i, COR_CONFIRMACAO, msg);
                break;
            }
        }

        EmpData[id][emp_caixa] -= salario;
        totalPago += salario;
    }

    // Notifica dono se caixa insuficiente
    if(proporcao < 1.0)
    {
        foreach(new i : Player)
        {
            new n[MAX_PLAYER_NAME];
            GetPlayerName(i, n, MAX_PLAYER_NAME);
            if(strcmp(n, EmpData[id][emp_dono], true) == 0)
            {
                SendClientMessage(i, COR_ERRO, "[EMPRESA] Caixa insuficiente para pagar todos os salarios.");
                break;
            }
        }
    }

    Empresa_Salvar(id);
    return totalPago;
}

// ---------------------------------------------------------------------------
// Verificar estoque baixo — Req 9.7
// ---------------------------------------------------------------------------
stock Empresa_VerificarEstoque(id)
{
    for(new p = 0; p < EMP_MAX_PRODUTOS; p++)
    {
        if(EmpEstoque[id][p][estoque_max] <= 0) continue;
        new pct = (EmpEstoque[id][p][estoque_quantidade] * 100) / EmpEstoque[id][p][estoque_max];
        if(pct < 10)
        {
            foreach(new i : Player)
            {
                new n[MAX_PLAYER_NAME];
                GetPlayerName(i, n, MAX_PLAYER_NAME);
                if(strcmp(n, EmpData[id][emp_dono], true) == 0)
                {
                    new msg[128];
                    format(msg, sizeof(msg), "[EMPRESA] Estoque baixo: %s (%d/%d).",
                        EmpEstoque[id][p][estoque_nome],
                        EmpEstoque[id][p][estoque_quantidade],
                        EmpEstoque[id][p][estoque_max]);
                    SendClientMessage(i, COR_ERRO, msg);
                    break;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Verificar inatividade (30 dias) — Req 9.10
// ---------------------------------------------------------------------------
stock Empresa_VerificarInatividade(id)
{
    if(strlen(EmpData[id][emp_nome]) == 0) return;
    if(EmpData[id][emp_ultimo_ativo] == 0) return;

    new ts = gettime();
    if(ts - EmpData[id][emp_ultimo_ativo] > 30 * 86400)
    {
        // Coloca em leilão: remove o dono
        strcopy(EmpData[id][emp_dono], "Leilao", MAX_PLAYER_NAME);
        EmpData[id][emp_caixa] = 0;
        Empresa_Salvar(id);
        printf("[EMPRESA] Empresa #%d colocada em leilão por inatividade.", id);
    }
}

// ---------------------------------------------------------------------------
// Processar dialog
// ---------------------------------------------------------------------------
stock Empresa_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused dialogid, inputtext
    if(!response) return 1;
    // Aba selecionada: listitem 0=Caixa, 1=Estoque, 2=Funcionarios, 3=Historico
    new msg[64];
    format(msg, sizeof(msg), "[EMPRESA] Aba %d selecionada.", listitem);
    SendClientMessage(playerid, COR_DESTAQUE, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:empresa(playerid, params[])
{
    #pragma unused params
    // Verifica se o jogador está próximo de sua empresa
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    for(new id = 0; id < MAX_EMPRESAS; id++)
    {
        if(strlen(EmpData[id][emp_nome]) == 0) continue;
        if(strcmp(EmpData[id][emp_dono], nome, true) != 0) continue;
        if(!IsPlayerInRangeOfPoint(playerid, 5.0, EmpData[id][emp_pos][0], EmpData[id][emp_pos][1], EmpData[id][emp_pos][2])) continue;

        EmpData[id][emp_ultimo_ativo] = gettime();
        Empresa_ExibirPainel(playerid, id);
        return 1;
    }
    SendClientMessage(playerid, COR_ERRO, "[EMPRESA] Voce nao esta perto de sua empresa.");
    return 1;
}

CMD:comprar(playerid, params[])
{
    new slot;
    if(sscanf(params, "i", slot))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /comprar [slot]"), 1;

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    for(new id = 0; id < MAX_EMPRESAS; id++)
    {
        if(strlen(EmpData[id][emp_nome]) == 0) continue;
        if(!IsPlayerInRangeOfPoint(playerid, 5.0, EmpData[id][emp_pos][0], EmpData[id][emp_pos][1], EmpData[id][emp_pos][2])) continue;
        Empresa_VenderProduto(playerid, id, slot);
        return 1;
    }
    SendClientMessage(playerid, COR_ERRO, "[EMPRESA] Nenhuma empresa por perto.");
    return 1;
}
