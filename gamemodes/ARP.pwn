// =============================================================================
//   Aurora Roleplay — Arquivo Principal (Orquestrador)
//   ARP.pwn  |  Versão 2026
//   Arquitetura: módulos separados em modulos/*.inc
// =============================================================================

// ---------------------------------------------------------------------------
// 1. Includes de bibliotecas SA-MP
// ---------------------------------------------------------------------------
#include <a_samp>
#include <a_players>
#include <a_vehicles>
#include <a_objects>
#include <a_zones>

// ---------------------------------------------------------------------------
// 2. Plugins / bibliotecas externas
// ---------------------------------------------------------------------------
#include <streamer>
#include <sscanf2>
#include <zcmd>
#include <DOF2>
#include <sampvoice>
#include <discord-connector>
#include <mapandreas>
#include <crashdetect>
#include <foreach>

// ---------------------------------------------------------------------------
// 3. Macro de comando (compatível zcmd / pawncmd)
// ---------------------------------------------------------------------------
#if !defined CMD
    #define CMD:%0(%1,%2) \
        forward cmd_%0(%1,%2); \
        public cmd_%0(%1,%2)
#endif

// ---------------------------------------------------------------------------
// 4. Constantes globais da gamemode
// ---------------------------------------------------------------------------
#define MAX_EMPRESAS            1000
#define MAX_CASAS               1000
#define MAX_ORGS                50
#define MAX_ORG_NAME            50
#define MAX_VEICULOS_PESSOAIS   (MAX_PLAYERS * 3)

// Caminhos de arquivo (scriptfiles/)
#define CAMINHO_JOGADOR         "Players/%s.ini"
#define CAMINHO_CASA            "Casas/%d.ini"
#define CAMINHO_EMPRESA         "Empresas/%d.ini"
#define CAMINHO_ORG             "Organizacoes/Org%d.ini"
#define CAMINHO_VEICULO         "Veiculos/%s.ini"
#define CAMINHO_LOG_ADMIN       "Logs/Admin.ini"
#define CAMINHO_LOG_MODERACAO   "Logs/Moderacao.ini"
#define CAMINHO_CONFIG          "Config/Servidor.ini"
#define CAMINHO_BANIDO          "Banidos/%s.ini"

// ---------------------------------------------------------------------------
// 5. Cores oficiais
// ---------------------------------------------------------------------------
#define COR_DESTAQUE            0x00C8FFFF
#define COR_ERRO                0xFF3B30FF
#define COR_CONFIRMACAO         0x32D74BFF
#define COR_STAFF               0xA855F7FF
#define COR_RADIO               0xFF9500FF
#define COR_BRANCO              0xFFFFFFFF
#define COR_SERVIDOR1           0xD0D0D0FF
#define COR_SERVIDOR5           0x707070FF

// ---------------------------------------------------------------------------
// 6. IDs de dialog centrais (módulos adicionam os seus)
// ---------------------------------------------------------------------------
enum
{
    DIALOG_REGISTRO = 1,
    DIALOG_LOGIN,
    DIALOG_CRIACAO_SEXO,
    DIALOG_CRIACAO_IDADE,
    DIALOG_CRIACAO_ORIGEM,
    DIALOG_PERFIL,
    DIALOG_BANCO_MENU,
    DIALOG_BANCO_DEPOSITAR,
    DIALOG_BANCO_SACAR,
    DIALOG_BANCO_EXTRATO,
    DIALOG_BANCO_TRANSFERIR,
    DIALOG_BANCO_PIN,
    DIALOG_INVENTARIO,
    DIALOG_CASA_MENU,
    DIALOG_EMPRESA_PAINEL,
    DIALOG_VEICULO_MENU,
    DIALOG_CELULAR_MENU,
    DIALOG_CELULAR_GPS,
    DIALOG_CELULAR_SMS,
    DIALOG_CELULAR_CONTATOS,
    DIALOG_ORG_MENU,
    DIALOG_POLICIAL_MENU,
    DIALOG_ADMIN_MENU,
    DIALOG_STATS
}

// ---------------------------------------------------------------------------
// 7. Enum de profissões (compartilhado entre módulos)
// ---------------------------------------------------------------------------
enum E_Profissao
{
    PROF_DESEMPREGADO,
    PROF_POLICIAL,
    PROF_PARAMEDICO,
    PROF_BOMBEIRO,
    PROF_MECANICO,
    PROF_TAXISTA,
    PROF_MOTORISTA_ONIBUS,
    PROF_EMPRESARIO,
    PROF_ADVOGADO,
    PROF_JORNALISTA
}

// ---------------------------------------------------------------------------
// 8. Enum principal do jogador (E_PlayerInfo)
// ---------------------------------------------------------------------------
enum E_PlayerInfo
{
    // Autenticação
    pi_senha[24],
    pi_email[64],
    pi_tentativas_login,

    // Personagem
    pi_nivel,
    pi_xp,
    pi_reputacao,
    pi_skin,
    pi_sexo,                    // 0 = masculino, 1 = feminino
    pi_idade,
    pi_origem[32],
    pi_data_criacao[20],
    pi_ultimo_login[20],
    pi_horas_jogadas,
    pi_mortes,
    pi_kills,

    // Financeiro
    pi_dinheiro,                // Máx R$50.000 (ECO_CASH_MAXIMO)
    pi_banco,
    pi_divida_hospital,

    // Status físico
    Float:pi_hp,
    Float:pi_colete,
    pi_fome,                    // 0–100
    pi_sede,                    // 0–100

    // Localização persistida
    Float:pi_pos_x,
    Float:pi_pos_y,
    Float:pi_pos_z,
    Float:pi_angulo,
    pi_interior,
    pi_virtual_world,

    // Progressão
    E_Profissao:pi_profissao,

    // Organização
    pi_org_id,                  // -1 se sem org
    pi_org_rank,

    // Empresa
    pi_emp_id,                  // -1 se sem empresa
    pi_emp_cargo,

    // Casa
    pi_casa_id,                 // -1 se sem casa

    // Admin / VIP
    pi_nivel_admin,
    pi_nivel_vip,
    pi_vip_expira,              // timestamp Unix

    // Estado de sessão (não persistido em arquivo)
    bool:pi_logado,
    bool:pi_incapacitado,
    bool:pi_algemado,
    bool:pi_preso,
    pi_timer_incapacitado,
    pi_timer_regen_casa,
    pi_timer_prisao,
    pi_wanted,                  // 0–6 estrelas

    // PIN bancário
    pi_pin[5],

    // Celular
    pi_numero_celular[12],
    bool:pi_celular_aberto,

    // Mutado
    bool:pi_mutado,
    pi_mute_expira              // timestamp Unix
}

// ---------------------------------------------------------------------------
// 9. Array global de dados dos jogadores
// ---------------------------------------------------------------------------
new pInfo[MAX_PLAYERS][E_PlayerInfo];

// ---------------------------------------------------------------------------
// 10. Variável auxiliar de string para formatações
// ---------------------------------------------------------------------------
new string[256];

// ---------------------------------------------------------------------------
// 11. Handles dos timers globais
// ---------------------------------------------------------------------------
new TIMER_AUTOSAVE;
new TIMER_FOME_SEDE;
new TIMER_HUD;
new TIMER_SALARIOS;
new TIMER_IMPOSTOS;
new TIMER_CLIMA;
new TIMER_HORA;
new TIMER_MSG_AUTO;
new TIMER_PASSAGEM_ONIBUS;
new TIMER_EMPRESA_INATIVIDADE;
new TIMER_VIP_CHECK;
new TIMER_MUTE_CHECK;

// ---------------------------------------------------------------------------
// 12. Mapa do servidor (objetos estáticos)
// ---------------------------------------------------------------------------
#include "../mapas/mapserver.inc"

// ---------------------------------------------------------------------------
// 13. Includes dos módulos na ordem do grafo de dependências
//     (cada módulo usa apenas os declarados antes dele)
// ---------------------------------------------------------------------------
#include "../modulos/sistema_economia.inc"
#include "../modulos/sistema_inventario.inc"
#include "../modulos/sistema_chat.inc"
#include "../modulos/sistema_voip.inc"
#include "../modulos/sistema_progressao.inc"
#include "../modulos/sistema_login.inc"
#include "../modulos/sistema_personagem.inc"
#include "../modulos/sistema_banco.inc"
#include "../modulos/sistema_casa.inc"
#include "../modulos/sistema_empresa.inc"
#include "../modulos/sistema_veiculo.inc"
#include "../modulos/sistema_celular.inc"
#include "../modulos/sistema_organizacao.inc"
#include "../modulos/sistema_policial.inc"
#include "../modulos/sistema_cidade.inc"
#include "../modulos/sistema_admin.inc"
// #include "../modulos/sistema_botsrck.inc"

// ---------------------------------------------------------------------------
// 14. Callbacks principais
// ---------------------------------------------------------------------------

public OnGameModeInit()
{
    // Configurações gerais da gamemode
    SetGameModeText("Aurora Roleplay 2026");
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_STREAMED);
    ShowNameTags(1);
    EnableStuntBonusForAll(0);
    DisableInteriorEnterExits();
    UsePlayerPedAnims();
    SetWeather(10);
    SetWorldTime(12);

    // Inicializa dados globais de módulos
    VoIP_InicializarTodos();
    Casa_CarregarTodas();
    Empresa_CarregarTodas();
    Org_CarregarTodas();

    // Inicia sistema de transporte público
    // Cidade_IniciarTransportePublico();

    // --- Timers globais ---
    TIMER_AUTOSAVE          = SetTimer("Timer_AutoSave",           300000,  true);
    TIMER_FOME_SEDE         = SetTimer("Timer_FomeSede",           600000,  true);
    TIMER_HUD               = SetTimer("Timer_HUD",                1000,    true);
    TIMER_SALARIOS          = SetTimer("Timer_Salarios",           3600000, true);
    TIMER_IMPOSTOS          = SetTimer("Timer_Impostos",           2592000000, true);
    TIMER_CLIMA             = SetTimer("Timer_Clima",              1800000, true);
    TIMER_HORA              = SetTimer("Timer_Hora",               60000,   true);
    TIMER_MSG_AUTO          = SetTimer("Timer_MsgAuto",            900000,  true);
    TIMER_PASSAGEM_ONIBUS   = SetTimer("Timer_PassagemOnibus",     2000,    true);
    TIMER_EMPRESA_INATIVIDADE = SetTimer("Timer_EmpresaInatividade", 86400000, true);
    TIMER_VIP_CHECK         = SetTimer("Timer_VipCheck",           86400000, true);
    TIMER_MUTE_CHECK        = SetTimer("Timer_MuteCheck",          60000,   true);

    printf("[ARP] Aurora Roleplay carregado com sucesso.");
    return 1;
}

public OnGameModeExit()
{
    // Cancela timers globais
    KillTimer(TIMER_AUTOSAVE);
    KillTimer(TIMER_FOME_SEDE);
    KillTimer(TIMER_HUD);
    KillTimer(TIMER_SALARIOS);
    KillTimer(TIMER_IMPOSTOS);
    KillTimer(TIMER_CLIMA);
    KillTimer(TIMER_HORA);
    KillTimer(TIMER_MSG_AUTO);
    KillTimer(TIMER_PASSAGEM_ONIBUS);
    KillTimer(TIMER_EMPRESA_INATIVIDADE);
    KillTimer(TIMER_VIP_CHECK);
    KillTimer(TIMER_MUTE_CHECK);

    // Salva todos os jogadores conectados
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado])
        {
            Personagem_Salvar(i);
        }
    }

    // Salva veículos
    for(new v = 0; v < MAX_VEICULOS_PESSOAIS; v++)
    {
        Veiculo_Salvar(v);
    }

    // Salva casas
    for(new c = 0; c < MAX_CASAS; c++)
    {
        Casa_Salvar(c);
    }

    // Salva empresas
    for(new e = 0; e < MAX_EMPRESAS; e++)
    {
        Empresa_Salvar(e);
    }

    // Salva organizações
    for(new o = 0; o < MAX_ORGS; o++)
    {
        Org_Salvar(o);
    }

    printf("[ARP] Gamemode encerrada. Dados salvos.");
    return 1;
}

public OnPlayerConnect(playerid)
{
    // Inicializa campos de sessão
    pInfo[playerid][pi_logado]          = false;
    pInfo[playerid][pi_incapacitado]    = false;
    pInfo[playerid][pi_algemado]        = false;
    pInfo[playerid][pi_preso]           = false;
    pInfo[playerid][pi_celular_aberto]  = false;
    pInfo[playerid][pi_mutado]          = false;
    pInfo[playerid][pi_timer_incapacitado] = 0;
    pInfo[playerid][pi_timer_regen_casa]   = 0;
    pInfo[playerid][pi_timer_prisao]       = 0;
    pInfo[playerid][pi_wanted]          = 0;

    // Inicializa VoIP local
    VoIP_CriarStreamLocal(playerid);

    // Inicia fluxo de login
    Login_ExibirRegistroOuLogin(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    if(pInfo[playerid][pi_logado])
    {
        Personagem_Salvar(playerid);
    }

    // Encerra chamada ativa se houver
    if(pChamadaAlvo[playerid] != -1)
    {
        Celular_EncerrarChamada(playerid);
    }

    // Cancela timers individuais
    if(pInfo[playerid][pi_timer_incapacitado] != 0)
    {
        KillTimer(pInfo[playerid][pi_timer_incapacitado]);
        pInfo[playerid][pi_timer_incapacitado] = 0;
    }
    if(pInfo[playerid][pi_timer_regen_casa] != 0)
    {
        KillTimer(pInfo[playerid][pi_timer_regen_casa]);
        pInfo[playerid][pi_timer_regen_casa] = 0;
    }
    if(pInfo[playerid][pi_timer_prisao] != 0)
    {
        KillTimer(pInfo[playerid][pi_timer_prisao]);
        pInfo[playerid][pi_timer_prisao] = 0;
    }

    // Destrói stream VoIP local
    VoIP_DestruirStreamLocal(playerid);
    return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
    // Incrementa mortes do jogador que morreu
    pInfo[playerid][pi_mortes]++;

    // Incrementa kills do assassino (se não foi suicídio)
    if(killerid != INVALID_PLAYER_ID)
    {
        pInfo[killerid][pi_kills]++;
    }

    Personagem_Incapacitar(playerid, killerid);
    return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    Veiculo_ProcessarDestruicao_PorSAMPID(vehicleid);
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    // Cada módulo trata seus próprios dialogs via funções dedicadas.
    // O roteamento é feito por intervalo de IDs de dialog.

    // Login / Registro
    if(dialogid == DIALOG_REGISTRO || dialogid == DIALOG_LOGIN)
    {
        return Login_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Criação de personagem
    if(dialogid == DIALOG_CRIACAO_SEXO
    || dialogid == DIALOG_CRIACAO_IDADE
    || dialogid == DIALOG_CRIACAO_ORIGEM)
    {
        return Personagem_ProcessarCriacao(playerid, dialogid, response, listitem, inputtext);
    }

    // Banco
    if(dialogid >= DIALOG_BANCO_MENU && dialogid <= DIALOG_BANCO_PIN)
    {
        return Banco_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Inventário
    if(dialogid == DIALOG_INVENTARIO)
    {
        return Inventario_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Casa
    if(dialogid == DIALOG_CASA_MENU)
    {
        return Casa_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Empresa
    if(dialogid == DIALOG_EMPRESA_PAINEL)
    {
        return Empresa_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Celular
    if(dialogid >= DIALOG_CELULAR_MENU && dialogid <= DIALOG_CELULAR_CONTATOS)
    {
        return Celular_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Organização
    if(dialogid == DIALOG_ORG_MENU)
    {
        return Org_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Policial
    if(dialogid == DIALOG_POLICIAL_MENU)
    {
        return Policial_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    // Admin
    if(dialogid == DIALOG_ADMIN_MENU)
    {
        return Admin_ProcessarDialog(playerid, dialogid, response, listitem, inputtext);
    }

    return 0;
}

public OnPlayerUpdate(playerid)
{
    // Atualiza posição em tempo real
    if(IsPlayerConnected(playerid) && pInfo[playerid][pi_logado])
    {
        new Float:x, Float:y, Float:z, Float:a;
        GetPlayerPos(playerid, x, y, z);
        GetPlayerFacingAngle(playerid, a);
        pInfo[playerid][pi_pos_x] = x;
        pInfo[playerid][pi_pos_y] = y;
        pInfo[playerid][pi_pos_z] = z;
        pInfo[playerid][pi_angulo] = a;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// 15. Callback do Discord Connector
// ---------------------------------------------------------------------------
public OnDCConnect(botid)
{
    // DCC_SendChannelMessage(botid, 0, "✅ Servidor Aurora Roleplay online.");
    return 1;
}

// ---------------------------------------------------------------------------
// 16. Callback de spawn de NPC (sistema_botsrck)
// ---------------------------------------------------------------------------
public OnPlayerSpawn(playerid)
{
    if(IsPlayerNPC(playerid))
    {
        return 1;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// 17. Callbacks dos timers globais (encaminham para os módulos responsáveis)
// ---------------------------------------------------------------------------

forward Timer_AutoSave();
public Timer_AutoSave()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado])
            Personagem_Salvar(i);
    }
    return 1;
}

forward Timer_FomeSede();
public Timer_FomeSede()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado] && !pInfo[i][pi_incapacitado])
        {
            // Decai fome –5 e sede –7; aplica dano se < 20
            pInfo[i][pi_fome] -= 5;
            if(pInfo[i][pi_fome] < 0)  pInfo[i][pi_fome] = 0;

            pInfo[i][pi_sede] -= 7;
            if(pInfo[i][pi_sede] < 0)  pInfo[i][pi_sede] = 0;

            if(pInfo[i][pi_fome] < 20 || pInfo[i][pi_sede] < 20)
            {
                new Float:hp;
                GetPlayerHealth(i, hp);
                if(hp > 10.0) SetPlayerHealth(i, hp - 5.0);
            }
        }
    }
    return 1;
}

forward Timer_HUD();
public Timer_HUD()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado])
            Personagem_AtualizarHUD(i);
    }
    return 1;
}

forward Timer_Salarios();
public Timer_Salarios()
{
    Economia_CicloSalarios();
    return 1;
}

forward Timer_Impostos();
public Timer_Impostos()
{
    Economia_CicloImpostos();
    return 1;
}

forward Timer_Clima();
public Timer_Clima()
{
    Cidade_AtualizarClima();
    return 1;
}

forward Timer_Hora();
public Timer_Hora()
{
    Cidade_AtualizarHora();
    return 1;
}

forward Timer_MsgAuto();
public Timer_MsgAuto()
{
    Cidade_EnviarMensagemAutomatica();
    return 1;
}

forward Timer_PassagemOnibus();
public Timer_PassagemOnibus()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado])
            Cidade_VerificarPassagemOnibus(i);
    }
    return 1;
}

forward Timer_EmpresaInatividade();
public Timer_EmpresaInatividade()
{
    for(new e = 0; e < MAX_EMPRESAS; e++)
    {
        Empresa_VerificarInatividade(e);
    }
    return 1;
}

forward Timer_VipCheck();
public Timer_VipCheck()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado])
            Admin_VerificarVIPExpirado(i);
    }
    return 1;
}

forward Timer_MuteCheck();
public Timer_MuteCheck()
{
    foreach(new i : Player)
    {
        if(pInfo[i][pi_logado] && pInfo[i][pi_mutado])
        {
            new timestamp = gettime();
            if(timestamp >= pInfo[i][pi_mute_expira])
            {
                pInfo[i][pi_mutado]     = false;
                pInfo[i][pi_mute_expira] = 0;
                SendClientMessage(i, COR_CONFIRMACAO, "[SERVER] Seu silenciamento expirou.");
            }
        }
    }
    return 1;
}
