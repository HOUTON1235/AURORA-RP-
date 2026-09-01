
#define				ANTI_CHEAT_NOTIFICACAO		(0)
#define				ANTI_CHEAT_EXPULSAO			(1)
#define				ANTI_CHEAT_BANIMENTO		(2)

	// Tipos:
		// - 0: Notificação
		// - 1: Expulsao
		// - 2: Banimento
// OnPlayerAntiCheater(playerid, tipo, const opcao[])
forward OnPlayerAntiCheater(playerid, tipo, const opcao[]);

					
					// Redefinições

#if defined _ALS_CreateVehicle
	#undef CreateVehicle
#else
	#define _ALS_CreateVehicle
#endif

#if defined _ALS_AddStaticVehicle
	#undef AddStaticVehicle
#else
	#define _ALS_AddStaticVehicle
#endif

#if defined _ALS_AddStaticVehicleEx
	#undef AddStaticVehicleEx
#else
	#define _ALS_AddStaticVehicleEx
#endif

#if defined _ALS_SetPlayerPos
	#undef SetPlayerPos
#else
	#define _ALS_SetPlayerPos
#endif

#if defined _ALS_PutPlayerInVehicle
	#undef PutPlayerInVehicle
#else
	#define _ALS_PutPlayerInVehicle
#endif

#if defined _ALS_TogglePlayerSpectating
	#undef TogglePlayerSpectating
#else
	#define _ALS_TogglePlayerSpectating
#endif

#if defined _ALS_DestroyVehicle
	#undef DestroyVehicle
#else
	#define _ALS_DestroyVehicle
#endif

#if defined _ALS_SetPlayerSkin
	#undef SetPlayerSkin
#else
	#define _ALS_SetPlayerSkin
#endif

#if defined _ALS_SetPlayerScore
	#undef SetPlayerScore
#else
	#define _ALS_SetPlayerScore
#endif

#if defined _ALS_GivePlayerMoney
	#undef GivePlayerMoney
#else
	#define _ALS_GivePlayerMoney
#endif

#if defined _ALS_ResetPlayerMoney
	#undef ResetPlayerMoney
#else
	#define _ALS_ResetPlayerMoney
#endif

#if defined _ALS_SetVehicleHealth
	#undef SetVehicleHealth
#else
	#define _ALS_SetVehicleHealth
#endif

#if defined _ALS_GivePlayerWeapon
	#undef GivePlayerWeapon
#else
	#define _ALS_GivePlayerWeapon
#endif

#if defined _ALS_ResetPlayerWeapons
    #undef ResetPlayerWeapons
#else
    #define _ALS_ResetPlayerWeapons
#endif

#if defined _ALS_SetPlayerAmmo
    #undef SetPlayerAmmo
#else
    #define _ALS_SetPlayerAmmo
#endif

#if defined _ALS_RemovePlayerFromVehicle
    #undef RemovePlayerFromVehicle
#else
    #define _ALS_RemovePlayerFromVehicle
#endif

#if defined _ALS_RepairVehicle
    #undef RepairVehicle
#else
    #define _ALS_RepairVehicle
#endif

#if defined _ALS_SpawnPlayer
    #undef SpawnPlayer
#else
    #define _ALS_SpawnPlayer
#endif

#if defined _ALS_SetVehiclePos
    #undef SetVehiclePos
#else
    #define _ALS_SetVehiclePos
#endif


					// Chamadas
forward AntiCheater_Admin(playerid); public AntiCheater_Admin(playerid) {return getPlayerAdmin(playerid);}
forward AntiCheater_Dinheiro(playerid); public AntiCheater_Dinheiro(playerid) {return GetPlayerCash(playerid);}


					// Chamadas enviadas para Bird
						// * Respirador do Passaro, favor não desligar...
stock birdEYE_TogglePlayerSpectating(playerid, toggle) {
	CallRemoteFunction("birdEYE_TogglePlayerSpectating", "dd", playerid, toggle);
	return TogglePlayerSpectating(playerid, toggle);
}
stock birdEYE_PutPlayerInVehicle(playerid, vehicleid, seatid) {
	CallRemoteFunction("birdEYE_PutPlayerInVehicle", "ddd", playerid, vehicleid, seatid);
	return PutPlayerInVehicle(playerid, vehicleid, seatid);
}

stock birdEYE_SetPlayerPos(playerid, Float:x, Float:y, Float:z) {
	CallRemoteFunction("birdEYE_SetPlayerPos", "dfff", playerid, x, y, z);
	return SetPlayerPos(playerid, x, y, z);
}

stock birdEYE_CreateVehicle(vehicletype, Float:x, Float:y, Float:z, Float:rotation, color1, color2, respawn_delay, addsiren=0) {
	new id = CreateVehicle(vehicletype, x, y, z, rotation, color1, color2, respawn_delay, addsiren);
	if(!CallRemoteFunction("birdEYE_CreateVehicle", "ddffffdddd", id, vehicletype, x, y, z, rotation, color1, color2, respawn_delay, addsiren)) return id;
	return id;
}

stock birdEYE_AddStaticVehicle(modelid, Float:spawn_x, Float:spawn_y, Float:spawn_z, Float:z_angle, color1, color2) {
	new id = AddStaticVehicle(modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2);
	if(!CallRemoteFunction("birdEYE_AddStaticVehicle", "ddffffdd", id, modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2)) return id;
	return id;
}

stock birdEYE_AddStaticVehicleEx(modelid, Float:spawn_x, Float:spawn_y, Float:spawn_z, Float:z_angle, color1, color2, respawn_delay, addsiren=0) {
	new id = AddStaticVehicleEx(modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2, respawn_delay, addsiren);
	if(!CallRemoteFunction("birdEYE_AddStaticVehicleEx", "ddffffddddd", id, modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2, respawn_delay, addsiren)) return id;
	return id;
}

stock birdEYE_SetPlayerSkin(playerid, skin) {
	if(!CallRemoteFunction("birdEYE_SetPlayerSkin", "dd", playerid, skin)) return true;
	return SetPlayerSkin(playerid, skin);
}

stock birdEYE_DestroyVehicle(vehicleid) {
	if(!CallRemoteFunction("birdEYE_DestroyVehicle", "d", vehicleid)) return true;
	return DestroyVehicle(vehicleid);
}

stock birdEYE_SetPlayerScore(playerid, score) {
	if(!CallRemoteFunction("birdEYE_SetPlayerScore", "dd", playerid, score)) return true;
	return SetPlayerScore(playerid, score);
}

stock birdEYE_SetVehicleHealth(vehicleid, Float:health) {
	if(!CallRemoteFunction("birdEYE_SetVehicleHealth", "df", vehicleid, health)) return true;
	return SetVehicleHealth(vehicleid, health);
}

stock birdEYE_GivePlayerWeapon(playerid, weaponid, ammo) {
	if(!CallRemoteFunction("birdEYE_GivePlayerWeapon", "ddd", playerid, weaponid, ammo)) return true;
	return GivePlayerWeapon(playerid, weaponid, ammo);
}

stock birdEYE_ResetPlayerWeapons(playerid) {
	if(!CallRemoteFunction("birdEYE_ResetPlayerWeapons", "d", playerid)) return true;
	return ResetPlayerWeapons(playerid);
}

stock birdEYE_SetPlayerAmmo(playerid, weaponslot, ammo) {
	if(!CallRemoteFunction("birdEYE_SetPlayerAmmo", "ddd", playerid, weaponslot, ammo)) return true;
	return SetPlayerAmmo(playerid, weaponslot, ammo);
}

stock birdEYE_RemovePlayerFromVehicle(playerid) {
	if(!CallRemoteFunction("birdEYE_RemovePlayerFromVehicle", "d", playerid)) return true;
	return RemovePlayerFromVehicle(playerid); 
}

stock birdEYE_RepairVehicle(vehicleid) {
	if(!CallRemoteFunction("birdEYE_SetVehicleHealth", "df", vehicleid, 1000.0)) return true;
	return RepairVehicle(vehicleid);
}

stock birdEYE_SpawnPlayer(playerid) {
	if(!CallRemoteFunction("birdEYE_SpawnPlayer", "d", playerid)) return true;
	return SpawnPlayer(playerid);
}

stock birdEYE_SetVehiclePos(vehicleid, Float:x, Float:y, Float:z) {
	if(!CallRemoteFunction("birdEYE_SetVehiclePos", "dfff", vehicleid, x, y, z)) return true;
	return SetVehiclePos(vehicleid, x, y, z);
}

#define TogglePlayerSpectating      birdEYE_TogglePlayerSpectating
#define PutPlayerInVehicle          birdEYE_PutPlayerInVehicle
#define SetPlayerPos                birdEYE_SetPlayerPos
#define CreateVehicle               birdEYE_CreateVehicle
#define AddStaticVehicle            birdEYE_AddStaticVehicle
#define AddStaticVehicleEx          birdEYE_AddStaticVehicleEx
#define DestroyVehicle              birdEYE_DestroyVehicle
#define SetPlayerSkin               birdEYE_SetPlayerSkin
#define SetPlayerScore				birdEYE_SetPlayerScore
#define SetVehicleHealth			birdEYE_SetVehicleHealth
#define GivePlayerWeapon 			birdEYE_GivePlayerWeapon
#define ResetPlayerWeapons 			birdEYE_ResetPlayerWeapons
#define SetPlayerAmmo 				birdEYE_SetPlayerAmmo
#define RemovePlayerFromVehicle 	birdEYE_RemovePlayerFromVehicle
#define RepairVehicle 				birdEYE_RepairVehicle
#define SpawnPlayer					birdEYE_SpawnPlayer
