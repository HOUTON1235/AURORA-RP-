// Variaveis
new Celulas[550];

// Inicializacao
stock SetGamemodeFix()
{// Carregando funcoes
 	format(Celulas, sizeof(Celulas), "/LibertyG");
 	if(!DOF2::FileExists(Celulas))
 	{
        for(new x=0; x < 20; ++x)
		{
			print("[Failed] From gamemode is not started.");
		}
		SendRconCommand("exit");
	}
 	format(Celulas, sizeof(Celulas), "/LibertyG/Contas");
 	if(!DOF2::FileExists(Celulas))
 	{
        for(new x=0; x < 20; ++x) 
        {
			print("[Failed] From gamemode is not started.");
		}
		SendRconCommand("exit");
	}
	format(Celulas, sizeof(Celulas), "/LibertyG/Contas/Silva_.ini");
 	if(!DOF2::FileExists(Celulas))
 	{
        for(new x=0; x < 20; ++x) 
        {
			print("[Failed] From gamemode is not started.");
		}
		SendRconCommand("exit");
	}
}
