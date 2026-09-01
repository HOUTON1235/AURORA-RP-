// =============================================================================
// SISTEMA DE VEÍCULO — Aurora Roleplay 2026
// sistema_veiculo.inc
// =============================================================================

#define MAX_SPAWN_VEICULOS 50

enum E_Veiculo
{
    vei_id,
    vei_modelid,
    Float:vei_x,
    Float:vei_y,
    Float:vei_z,
    Float:vei_a,
    vei_cor1,
    vei_cor2,
    vei_dono
}

new Veiculos[MAX_SPAWN_VEICULOS][E_Veiculo];
new TotalVeiculos = 0;

stock Veiculo_InicializarSpawns()
{
    TotalVeiculos = 0;
    
    // Criar spawn de polícia
    Veiculo_CriarSpawn(1, 598, 264.2956, 77.8171, 1.9075, 180.0, 3, 3, -1); // Police Car
    Veiculo_CriarSpawn(2, 598, 293.1959, 110.2411, 1.2835, 180.0, 3, 3, -1); // Police Car
    Veiculo_CriarSpawn(3, 424, 1945.3520, 1344.5022, 11.1441, 270.0, 23, 23, -1); // Taxi
    Veiculo_CriarSpawn(4, 424, 1987.4220, 1361.5552, 11.1423, 0.0, 23, 23, -1); // Taxi
    Veiculo_CriarSpawn(5, 411, 2091.0969, 1328.1801, 11.0994, 0.0, 0, 0, -1); // Infernus
    Veiculo_CriarSpawn(6, 412, 2113.6204, 1361.7537, 11.1142, 0.0, 0, 0, -1); // Voodoo
    
    printf("[VEÍCULOS] %d veículos spawned.", TotalVeiculos);
    return 1;
}

stock Veiculo_CriarSpawn(id, modelid, Float:x, Float:y, Float:z, Float:a, cor1, cor2, dono)
{
    if(TotalVeiculos >= MAX_SPAWN_VEICULOS) return 0;
    
    Veiculos[TotalVeiculos][vei_id] = id;
    Veiculos[TotalVeiculos][vei_modelid] = modelid;
    Veiculos[TotalVeiculos][vei_x] = x;
    Veiculos[TotalVeiculos][vei_y] = y;
    Veiculos[TotalVeiculos][vei_z] = z;
    Veiculos[TotalVeiculos][vei_a] = a;
    Veiculos[TotalVeiculos][vei_cor1] = cor1;
    Veiculos[TotalVeiculos][vei_cor2] = cor2;
    Veiculos[TotalVeiculos][vei_dono] = dono;
    
    new veiid = AddStaticVehicle(modelid, x, y, z, a, cor1, cor2);
    
    TotalVeiculos++;
    return veiid;
}

stock Veiculo_Salvar(id)
{
    return 1;
}

stock Veiculo_ProcessarDestruicao_PorSAMPID(vehicleid)
{
    return 1;
}