// =============================================================================
// SISTEMA DE CASA — Aurora Roleplay 2026
// sistema_casa.inc
// =============================================================================

#define MAX_CASAS_SPAWN 100

enum E_Casa
{
    casa_id,
    Float:casa_x,
    Float:casa_y,
    Float:casa_z,
    casa_preco,
    casa_dono
}

new Casas[MAX_CASAS][E_Casa];
new TotalCasas = 0;

stock Casa_CarregarTodas()
{
    // Adicionar algumas casas padrão
    Casa_CriarSpawn(0, 223.6604, 1287.5541, 1082.1406, 50000, -1);
    Casa_CriarSpawn(1, 228.0000, 1294.2000, 1082.0000, 60000, -1);
    Casa_CriarSpawn(2, 299.5068, 1473.3766, 1084.3281, 75000, -1);
    
    printf("[CASAS] %d casas carregadas.", TotalCasas);
    return 1;
}

stock Casa_CriarSpawn(id, Float:x, Float:y, Float:z, preco, dono)
{
    Casas[TotalCasas][casa_id] = id;
    Casas[TotalCasas][casa_x] = x;
    Casas[TotalCasas][casa_y] = y;
    Casas[TotalCasas][casa_z] = z;
    Casas[TotalCasas][casa_preco] = preco;
    Casas[TotalCasas][casa_dono] = dono;
    
    // Criar pickup
    CreateDynamicPickup(1273, 23, x, y, z, -1, -1, -1, 100.0);
    
    TotalCasas++;
    return 1;
}

stock Casa_Salvar(id)
{
    // Salvar em arquivo
    return 1;
}

stock Casa_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    return 1;
}