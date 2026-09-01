// =============================================================================
// SISTEMA DE BANCO — Aurora Roleplay 2026
// sistema_banco.inc
// =============================================================================

stock Banco_ProcessarDialog(playerid, dialogid, response, listitem, inputtext[])
{
    if(!response) return 1;
    
    if(dialogid == DIALOG_BANCO_MENU)
    {
        switch(listitem)
        {
            case 0: // Depositar
            {
                ShowPlayerDialog(playerid, DIALOG_BANCO_DEPOSITAR, DIALOG_STYLE_INPUT,
                    "Banco", "Digite a quantia para depositar:", "Depositar", "Voltar");
            }
            case 1: // Sacar
            {
                ShowPlayerDialog(playerid, DIALOG_BANCO_SACAR, DIALOG_STYLE_INPUT,
                    "Banco", "Digite a quantia para sacar:", "Sacar", "Voltar");
            }
        }
    }
    return 1;
}