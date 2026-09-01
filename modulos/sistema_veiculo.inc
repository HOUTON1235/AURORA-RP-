// =============================================================================
//   sistema_veiculo.inc — Veículos pessoais, combustível, garagem e seguro
//   Aurora Roleplay 2026
//   Requirements: 10.1–10.13, 20.4, 21.2, 21.6
// =============================================================================

#define MAX_VEICULOS_PESSOAIS   (MAX_PLAYERS * 3)

enum E_Veiculo
{
    veh_id_samp,
    veh_modelo,
    veh_placa[8],
    veh_dono[MAX_PLAYER_NAME],
    veh_cor1,
    veh_cor2,
    Float:veh_combustivel,
    Float:veh_km,
    Float:veh_dano,
    bool:veh_trancado,
    bool:veh_motor_ligado,
    bool:veh_guardado,
    bool:veh_segurado,
    veh_seguro_expira,
    Float:veh_seguro_valor_mercado,
    Float:veh_pos[3],
    Float:veh_angulo
}

new VeiculoData[MAX_VEICULOS_PESSOAIS][E_Veiculo];
new gTotalVeiculos = 0;

// Tabela de preços de veículos (modelo → preço base)
static stock Veiculo_PrecoBase(modelo)
{
    // Preços simplificados por categoria de modelo
    if(modelo >= 400 && modelo <= 450) return 15000;
    if(modelo >= 451 && modelo <= 500) return 25000;
    if(modelo >= 501 && modelo <= 550) return 40000;
    if(modelo >= 551 && modelo <= 600) return 60000;
    return 20000;
}

// ---------------------------------------------------------------------------
// Gerar placa única — Req 10.2
// ---------------------------------------------------------------------------
stock Veiculo_GerarPlaca(placa[8])
{
    static const chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const nums[]  = "0123456789";

    new tentativas = 0;
    new caminho[32];

    do {
        placa[0] = chars[random(26)];
        placa[1] = chars[random(26)];
        placa[2] = chars[random(26)];
        placa[3] = '-';
        placa[4] = nums[random(10)];
        placa[5] = nums[random(10)];
        placa[6] = nums[random(10)];
        placa[7] = EOS;

        format(caminho, sizeof(caminho), "Veiculos/placa_%s.ini", placa);
        tentativas++;
    } while(DOF2_FileExists(caminho) && tentativas < 100);

    // Marca a placa como usada
    DOF2_SetInt(caminho, "usado", 1);
    DOF2_SaveFile(caminho);
}

// ---------------------------------------------------------------------------
// Encontrar slot livre
// ---------------------------------------------------------------------------
static stock Veiculo_SlotLivre()
{
    for(new i = 0; i < MAX_VEICULOS_PESSOAIS; i++)
        if(VeiculoData[i][veh_id_samp] == 0 && strlen(VeiculoData[i][veh_dono]) == 0)
            return i;
    return -1;
}

// ---------------------------------------------------------------------------
// Comprar veículo — Req 10.2, 21.6
// ---------------------------------------------------------------------------
stock Veiculo_Comprar(playerid, modelo)
{
    new Float:desconto = Progressao_DescontoConcess(playerid);
    new preco_base = Veiculo_PrecoBase(modelo);
    new preco = floatround(float(preco_base) * (1.0 - desconto), floatround_floor);

    if(!Economia_DebitarBanco(playerid, preco))
    {
        new msg[64];
        format(msg, sizeof(msg), "[VEICULO] Saldo insuficiente. Necessario: R$%d", preco);
        SendClientMessage(playerid, COR_ERRO, msg);
        return;
    }

    new slot = Veiculo_SlotLivre();
    if(slot == -1)
    {
        SendClientMessage(playerid, COR_ERRO, "[VEICULO] Limite de veiculos do servidor atingido.");
        Economia_CreditarBanco(playerid, preco); // Estorno
        return;
    }

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    new placa[8];
    Veiculo_GerarPlaca(placa);

    VeiculoData[slot][veh_modelo]       = modelo;
    strcopy(VeiculoData[slot][veh_placa], placa, 8);
    strcopy(VeiculoData[slot][veh_dono],  nome, MAX_PLAYER_NAME);
    VeiculoData[slot][veh_cor1]         = random(128);
    VeiculoData[slot][veh_cor2]         = random(128);
    VeiculoData[slot][veh_combustivel]  = 100.0;
    VeiculoData[slot][veh_km]           = 0.0;
    VeiculoData[slot][veh_dano]         = 0.0;
    VeiculoData[slot][veh_trancado]     = false;
    VeiculoData[slot][veh_motor_ligado] = false;
    VeiculoData[slot][veh_guardado]     = false;
    VeiculoData[slot][veh_segurado]     = false;
    VeiculoData[slot][veh_seguro_valor_mercado] = float(preco);
    VeiculoData[slot][veh_pos][0]       = px + 3.0;
    VeiculoData[slot][veh_pos][1]       = py;
    VeiculoData[slot][veh_pos][2]       = pz;
    VeiculoData[slot][veh_angulo]       = 0.0;

    new vid = CreateVehicle(modelo, px + 3.0, py, pz, 0.0,
        VeiculoData[slot][veh_cor1], VeiculoData[slot][veh_cor2], -1);
    SetVehicleNumberPlate(vid, placa);
    VeiculoData[slot][veh_id_samp] = vid;

    Progressao_RegistrarConquista(playerid, CONQUISTA_PRIMEIRO_VEICULO);
    Veiculo_Salvar(slot);

    new msg[128];
    format(msg, sizeof(msg), "[VEICULO] Veiculo comprado! Placa: %s | Preco: R$%d (desconto %.0f%%)",
        placa, preco, desconto * 100.0);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    gTotalVeiculos++;
}

// ---------------------------------------------------------------------------
// Ligar/desligar motor — Req 10.3
// ---------------------------------------------------------------------------
stock Veiculo_AlternarMotor(playerid, slot)
{
    if(VeiculoData[slot][veh_dano] >= 90.0)
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Veiculo com dano critico. Conserte primeiro.");

    if(VeiculoData[slot][veh_combustivel] <= 0.0)
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Sem combustivel. Abasteca o veiculo.");

    VeiculoData[slot][veh_motor_ligado] = !VeiculoData[slot][veh_motor_ligado];

    new msg[64];
    format(msg, sizeof(msg), "[VEICULO] Motor %s.", VeiculoData[slot][veh_motor_ligado] ? "ligado" : "desligado");
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Consumir combustível — Req 10.4
// ---------------------------------------------------------------------------
stock Veiculo_ConsumirCombustivel(slot, Float:distancia)
{
    if(!VeiculoData[slot][veh_motor_ligado]) return;

    new consumo = floatround(distancia / 500.0, floatround_floor);
    if(consumo <= 0) return;

    VeiculoData[slot][veh_combustivel] -= float(consumo);
    VeiculoData[slot][veh_km]          += distancia / 1000.0;

    if(VeiculoData[slot][veh_combustivel] <= 0.0)
    {
        VeiculoData[slot][veh_combustivel]  = 0.0;
        VeiculoData[slot][veh_motor_ligado] = false;

        // Notifica o dono se online
        foreach(new i : Player)
        {
            new nome[MAX_PLAYER_NAME];
            GetPlayerName(i, nome, MAX_PLAYER_NAME);
            if(strcmp(nome, VeiculoData[slot][veh_dono], true) == 0)
            {
                SendClientMessage(i, COR_ERRO, "[VEICULO] Combustivel zerado! Motor desligado.");
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Abastecer — Req 10.6
// ---------------------------------------------------------------------------
stock Veiculo_Abastecer(playerid, slot, litros)
{
    new Float:preco_litro = 4.5;
    new Float:total = preco_litro * float(litros);
    new custo = floatround(total, floatround_ceil);

    if(!Economia_RetirarDinheiro(playerid, custo))
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Dinheiro em mao insuficiente para abastecer.");

    VeiculoData[slot][veh_combustivel] += float(litros);
    if(VeiculoData[slot][veh_combustivel] > 100.0) VeiculoData[slot][veh_combustivel] = 100.0;

    new msg[64];
    format(msg, sizeof(msg), "[VEICULO] Abastecido %d litros por R$%d.", litros, custo);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Guardar/Retirar da garagem — Req 10.9, 10.10
// ---------------------------------------------------------------------------
stock Veiculo_Guardar(playerid, slot)
{
    if(VeiculoData[slot][veh_guardado])
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Veiculo ja esta guardado.");

    if(VeiculoData[slot][veh_id_samp] != 0)
    {
        new Float:vx, Float:vy, Float:vz, Float:va;
        GetVehiclePos(VeiculoData[slot][veh_id_samp], vx, vy, vz);
        GetVehicleZAngle(VeiculoData[slot][veh_id_samp], va);
        VeiculoData[slot][veh_pos][0] = vx;
        VeiculoData[slot][veh_pos][1] = vy;
        VeiculoData[slot][veh_pos][2] = vz;
        VeiculoData[slot][veh_angulo] = va;
        DestroyVehicle(VeiculoData[slot][veh_id_samp]);
        VeiculoData[slot][veh_id_samp] = 0;
    }

    VeiculoData[slot][veh_guardado] = true;
    Veiculo_Salvar(slot);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[VEICULO] Veiculo guardado na garagem.");
}

stock Veiculo_Retirar(playerid, slot)
{
    if(!VeiculoData[slot][veh_guardado])
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Veiculo nao esta guardado.");

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    new vid = CreateVehicle(VeiculoData[slot][veh_modelo],
        px + 3.0, py, pz, VeiculoData[slot][veh_angulo],
        VeiculoData[slot][veh_cor1], VeiculoData[slot][veh_cor2], -1);
    SetVehicleNumberPlate(vid, VeiculoData[slot][veh_placa]);
    VeiculoData[slot][veh_id_samp] = vid;
    VeiculoData[slot][veh_guardado] = false;

    Veiculo_Salvar(slot);
    SendClientMessage(playerid, COR_CONFIRMACAO, "[VEICULO] Veiculo retirado da garagem.");
}

// ---------------------------------------------------------------------------
// Transferir — Req 10.11
// ---------------------------------------------------------------------------
stock Veiculo_Transferir(playerid, alvo, slot)
{
    new nomeAlvo[MAX_PLAYER_NAME], nomeRemetente[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nomeRemetente, MAX_PLAYER_NAME);
    GetPlayerName(alvo, nomeAlvo, MAX_PLAYER_NAME);

    strcopy(VeiculoData[slot][veh_dono], nomeAlvo, MAX_PLAYER_NAME);
    Veiculo_Salvar(slot);

    new msg[128];
    format(msg, sizeof(msg), "[VEICULO] Veiculo (placa %s) transferido para %s.", VeiculoData[slot][veh_placa], nomeAlvo);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    format(msg, sizeof(msg), "[VEICULO] Voce recebeu um veiculo (placa %s) de %s.", VeiculoData[slot][veh_placa], nomeRemetente);
    SendClientMessage(alvo, COR_CONFIRMACAO, msg);
}

// ---------------------------------------------------------------------------
// Seguro — Req 10.12, 10.13
// ---------------------------------------------------------------------------
stock Veiculo_ContratarSeguro(playerid, slot)
{
    new premio = floatround(VeiculoData[slot][veh_seguro_valor_mercado] * 0.05, floatround_ceil);

    if(!Economia_DebitarBanco(playerid, premio))
        return SendClientMessage(playerid, COR_ERRO, "[SEGURO] Saldo bancario insuficiente.");

    VeiculoData[slot][veh_segurado]     = true;
    VeiculoData[slot][veh_seguro_expira] = gettime() + (30 * 86400); // 30 dias
    Veiculo_Salvar(slot);

    new msg[64];
    format(msg, sizeof(msg), "[SEGURO] Seguro contratado por R$%d. Validade: 30 dias.", premio);
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
}

stock Veiculo_ProcessarDestruicao(slot)
{
    if(slot < 0 || slot >= MAX_VEICULOS_PESSOAIS) return;

    if(VeiculoData[slot][veh_segurado])
    {
        new credito = floatround(VeiculoData[slot][veh_seguro_valor_mercado] * ECO_SEGURO_COBERTURA, floatround_floor);

        foreach(new i : Player)
        {
            new nome[MAX_PLAYER_NAME];
            GetPlayerName(i, nome, MAX_PLAYER_NAME);
            if(strcmp(nome, VeiculoData[slot][veh_dono], true) == 0)
            {
                Economia_CreditarBanco(i, credito);
                new msg[128];
                format(msg, sizeof(msg), "[SEGURO] Veiculo destruido. Credito de R$%d (80%%) no seu banco.", credito);
                SendClientMessage(i, COR_CONFIRMACAO, msg);
                break;
            }
        }
        VeiculoData[slot][veh_segurado] = false;
    }

    VeiculoData[slot][veh_id_samp]     = 0;
    VeiculoData[slot][veh_guardado]    = true;
    VeiculoData[slot][veh_dano]        = 0.0;
    VeiculoData[slot][veh_combustivel] = 50.0;
    Veiculo_Salvar(slot);
}

// Wrapper por SAMP vehicle ID (chamado de OnVehicleDeath)
stock Veiculo_ProcessarDestruicao_PorSAMPID(vehicleid)
{
    for(new s = 0; s < MAX_VEICULOS_PESSOAIS; s++)
        if(VeiculoData[s][veh_id_samp] == vehicleid) { Veiculo_ProcessarDestruicao(s); return; }
}

// ---------------------------------------------------------------------------
// Salvar — Req 20.4
// ---------------------------------------------------------------------------
stock Veiculo_Salvar(slot)
{
    if(slot < 0 || slot >= MAX_VEICULOS_PESSOAIS) return;
    if(strlen(VeiculoData[slot][veh_dono]) == 0) return;

    new caminho[64];
    format(caminho, sizeof(caminho), "Veiculos/%s_%d.ini", VeiculoData[slot][veh_dono], slot);

    DOF2_SetString(caminho, "Dono",          VeiculoData[slot][veh_dono]);
    DOF2_SetString(caminho, "Placa",         VeiculoData[slot][veh_placa]);
    DOF2_SetInt   (caminho, "Modelo",        VeiculoData[slot][veh_modelo]);
    DOF2_SetInt   (caminho, "Cor1",          VeiculoData[slot][veh_cor1]);
    DOF2_SetInt   (caminho, "Cor2",          VeiculoData[slot][veh_cor2]);
    DOF2_SetFloat (caminho, "Combustivel",   VeiculoData[slot][veh_combustivel]);
    DOF2_SetFloat (caminho, "KM",            VeiculoData[slot][veh_km]);
    DOF2_SetFloat (caminho, "Dano",          VeiculoData[slot][veh_dano]);
    DOF2_SetInt   (caminho, "Guardado",      _:VeiculoData[slot][veh_guardado]);
    DOF2_SetInt   (caminho, "Segurado",      _:VeiculoData[slot][veh_segurado]);
    DOF2_SetInt   (caminho, "SeguroExpira",  VeiculoData[slot][veh_seguro_expira]);
    DOF2_SetFloat (caminho, "SeguroValor",   VeiculoData[slot][veh_seguro_valor_mercado]);
    DOF2_SetFloat (caminho, "PosX",          VeiculoData[slot][veh_pos][0]);
    DOF2_SetFloat (caminho, "PosY",          VeiculoData[slot][veh_pos][1]);
    DOF2_SetFloat (caminho, "PosZ",          VeiculoData[slot][veh_pos][2]);
    DOF2_SetFloat (caminho, "Angulo",        VeiculoData[slot][veh_angulo]);
    DOF2_SaveFile(caminho);
}

// ---------------------------------------------------------------------------
// Encontrar slot do veículo pelo jogador e SAMP ID
// ---------------------------------------------------------------------------
stock Veiculo_SlotPorSAMPID(vehicleid)
{
    for(new s = 0; s < MAX_VEICULOS_PESSOAIS; s++)
        if(VeiculoData[s][veh_id_samp] == vehicleid) return s;
    return -1;
}

stock Veiculo_SlotPorDono(const nome[])
{
    for(new s = 0; s < MAX_VEICULOS_PESSOAIS; s++)
        if(strcmp(VeiculoData[s][veh_dono], nome, true) == 0 && !VeiculoData[s][veh_guardado])
            return s;
    return -1;
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------
CMD:ligarcarro(playerid, params[])
{
    #pragma unused params
    new vid = GetPlayerVehicleID(playerid);
    if(!vid) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Voce nao esta em um veiculo."), 1;

    new slot = Veiculo_SlotPorSAMPID(vid);
    if(slot == -1) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Este nao e seu veiculo."), 1;

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    if(strcmp(VeiculoData[slot][veh_dono], nome, true) != 0)
        return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Este nao e seu veiculo."), 1;

    Veiculo_AlternarMotor(playerid, slot);
    return 1;
}

CMD:trancarveiculo(playerid, params[])
{
    #pragma unused params
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    new slot = Veiculo_SlotPorDono(nome);
    if(slot == -1) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Nenhum veiculo encontrado."), 1;

    VeiculoData[slot][veh_trancado] = !VeiculoData[slot][veh_trancado];
    new msg[64];
    format(msg, sizeof(msg), "[VEICULO] Veiculo %s.", VeiculoData[slot][veh_trancado] ? "trancado" : "destrancado");
    SendClientMessage(playerid, COR_CONFIRMACAO, msg);
    return 1;
}

CMD:guardarveiculo(playerid, params[])
{
    #pragma unused params
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    new slot = Veiculo_SlotPorDono(nome);
    if(slot == -1) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Nenhum veiculo encontrado."), 1;
    Veiculo_Guardar(playerid, slot);
    return 1;
}

CMD:pegarveiculo(playerid, params[])
{
    #pragma unused params
    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);

    new lista[512], linha[64];
    new count = 0;
    for(new s = 0; s < MAX_VEICULOS_PESSOAIS; s++)
    {
        if(strcmp(VeiculoData[s][veh_dono], nome, true) != 0) continue;
        if(!VeiculoData[s][veh_guardado]) continue;
        format(linha, sizeof(linha), "[%d] Modelo %d | Placa %s\n", s, VeiculoData[s][veh_modelo], VeiculoData[s][veh_placa]);
        strcat(lista, linha, sizeof(lista));
        count++;
    }

    if(count == 0) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Nenhum veiculo guardado."), 1;

    ShowPlayerDialog(playerid, DIALOG_VEICULO_MENU, DIALOG_STYLE_LIST, "Garagem", lista, "Retirar", "Cancelar");
    return 1;
}

CMD:transferirveiculo(playerid, params[])
{
    new destNome[MAX_PLAYER_NAME];
    if(sscanf(params, "s[24]", destNome))
        return SendClientMessage(playerid, COR_ERRO, "Uso: /transferirveiculo [nome]"), 1;

    new dest = -1;
    foreach(new i : Player)
    {
        new n[MAX_PLAYER_NAME]; GetPlayerName(i, n, MAX_PLAYER_NAME);
        if(strcmp(n, destNome, true) == 0) { dest = i; break; }
    }
    if(dest == -1) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Jogador nao encontrado."), 1;

    new nome[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nome, MAX_PLAYER_NAME);
    new slot = Veiculo_SlotPorDono(nome);
    if(slot == -1) return SendClientMessage(playerid, COR_ERRO, "[VEICULO] Nenhum veiculo encontrado."), 1;

    Veiculo_Transferir(playerid, dest, slot);
    return 1;
}
