// Sistema de Bots
new OnibusNpc[2];
new Text3D:TextoOnibusBot[MAX_VEHICLES];

// Conexao dos bots
CallBack:: ConectarBotsSV()
{
    // Onibus
    OnibusNpc[0] = AddStaticVehicleEx(431,1683.5056, -2245.9348, -2.6817, 187.4125, 1, 3, false); // onibus para o npc
    TextoOnibusBot[OnibusNpc[0]]  = Create3DTextLabel("{FFFFFF}ROTAS {FF2400}PUBLICAS{FFFFFF}", -1, 1411.2141,-2332.9297,13.6802, 15.0, 0, 0);
    Attach3DTextLabelToVehicle(TextoOnibusBot[OnibusNpc[0]], OnibusNpc[0], 0.000, 0.530, 0.000+2);
    // Helicoptero
    OnibusNpc[1] = AddStaticVehicleEx(487,1683.5056, -2245.9348, -2.6817, 187.4125, 3, 1, false); // Helicoptero para o npc
    TextoOnibusBot[OnibusNpc[1]]  = Create3DTextLabel("{FFFFFF}ROTAS {FF2400}PUBLICAS - Aeronautica{FFFFFF}", -1, 1411.2141,-2332.9297,13.6802, 15.0, 0, 0);
    Attach3DTextLabelToVehicle(TextoOnibusBot[OnibusNpc[1]], OnibusNpc[1], 0.000, 0.530, 0.000+2);
    // Conexao
    ConnectNPC("Oficer_Helicoptero", "helils"); // Nome e Funcao.
    ConnectNPC("Oficer_Onibus", "onibusrck"); // Nome e Funcao.
    print("[BOT] O sistema de conexao por bot foi carregado com sucesso.");
    return 1;
}

// CallBacks
CallBack:: BotSpawn()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerNPC(i))
	    {
            if(!strcmp(PlayerName(i), "Oficer_Helicoptero", true))
	        {
	   			printf("[BOT] %s Verificacao de spawn.", PlayerName(i));
			}
	        if(!strcmp(PlayerName(i), "Oficer_Onibus", true))
	        {
	   			printf("[BOT] %s Verificacao de spawn.", PlayerName(i));
			}
            SpawnPlayer(i);
		}
	}
    return 1;
}
