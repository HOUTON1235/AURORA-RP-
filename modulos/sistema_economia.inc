// =============================================================================
//   sistema_economia.inc — Núcleo financeiro atômico
//   Aurora Roleplay 2026
//
//   REGRA CENTRAL: Nenhum outro módulo altera pi_dinheiro ou pi_banco
//   diretamente — toda movimentação passa obrigatoriamente por este módulo.
//
//   Invariante: pInfo[playerid][pi_dinheiro] NUNCA excede ECO_CASH_MAXIMO.
//
//   Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 4.10, 21.1
// =============================================================================

// ---------------------------------------------------------------------------
// Constantes do sistema econômico
// ---------------------------------------------------------------------------
#define ECO_CASH_MAXIMO         50000
#define ECO_TAXA_HOSPITAL       500
#define ECO_TAXA_ANUNCIO        100
#define ECO_TAXA_PASSAGEM       5
#define ECO_SEGURO_COBERTURA    0.80
#define ECO_MAX_EXTRATO         50

// ---------------------------------------------------------------------------
// Enum e estrutura do extrato (ring buffer por jogador)
// ---------------------------------------------------------------------------
enum E_Extrato
{
    ext_tipo[32],
    ext_valor,
    ext_envolvido[MAX_PLAYER_NAME],
    ext_timestamp
}

new pExtrato[MAX_PLAYERS][ECO_MAX_EXTRATO][E_Extrato];
new pExtratoIdx[MAX_PLAYERS]; // índice circular (próxima posição a sobrescrever)

// ---------------------------------------------------------------------------
// Tabela de salários por profissão (indexada por E_Profissao)
// ---------------------------------------------------------------------------
static const gSalarioBase[] =
{
    500,    // PROF_DESEMPREGADO
    3000,   // PROF_POLICIAL
    2800,   // PROF_PARAMEDICO
    2500,   // PROF_BOMBEIRO
    2000,   // PROF_MECANICO
    1500,   // PROF_TAXISTA
    1800,   // PROF_MOTORISTA_ONIBUS
    1000,   // PROF_EMPRESARIO  (base; complemento vem do caixa da empresa via sistema_empresa)
    3500,   // PROF_ADVOGADO
    2200    // PROF_JORNALISTA
};

// =============================================================================
// Inicialização por jogador
// =============================================================================

/**
 * Inicializa o extrato do jogador ao conectar.
 * Chamado por OnPlayerConnect em ARP.pwn (já tratado por zero-init global,
 * mas esta função garante reset explícito ao reconectar).
 */
stock Economia_InicializarJogador(playerid)
{
    pExtratoIdx[playerid] = 0;

    for(new i = 0; i < ECO_MAX_EXTRATO; i++)
    {
        pExtrato[playerid][i][ext_tipo][0]      = EOS;
        pExtrato[playerid][i][ext_valor]        = 0;
        pExtrato[playerid][i][ext_envolvido][0] = EOS;
        pExtrato[playerid][i][ext_timestamp]    = 0;
    }
}

// =============================================================================
// Sub-tarefa 1 — Dar / retirar dinheiro em mão com invariante ECO_CASH_MAXIMO
// =============================================================================

/**
 * Credita valor no dinheiro em mão do jogador.
 * Se a soma ultrapassar ECO_CASH_MAXIMO, credita apenas o que couber.
 * Retorna o valor efetivamente creditado (pode ser 0 se já no teto).
 * Req 4.1
 */
stock Economia_DarDinheiro(playerid, valor)
{
    if(valor <= 0) return 0;

    new atual  = pInfo[playerid][pi_dinheiro];
    new espaco = ECO_CASH_MAXIMO - atual;
    if(espaco <= 0) return 0;

    new creditado = (valor > espaco) ? espaco : valor;
    pInfo[playerid][pi_dinheiro] += creditado;

    // Sincroniza o dinheiro exibido pelo SA-MP com o valor real
    GivePlayerMoney(playerid, creditado);

    return creditado;
}

/**
 * Retira valor do dinheiro em mão do jogador.
 * Falha silenciosa se saldo insuficiente.
 * Retorna 1 em sucesso, 0 se saldo insuficiente.
 * Req 4.1
 */
stock Economia_RetirarDinheiro(playerid, valor)
{
    if(valor <= 0) return 0;
    if(pInfo[playerid][pi_dinheiro] < valor) return 0;

    pInfo[playerid][pi_dinheiro] -= valor;

    // Remove do contador exibido pelo SA-MP (negativo = dedução)
    GivePlayerMoney(playerid, -valor);

    return 1;
}

// =============================================================================
// Sub-tarefa 2 — Depositar / Sacar (operações atômicas cash ↔ banco)
// =============================================================================

/**
 * Depósito atômico: deduz do dinheiro em mão e credita no banco.
 * Retorna 1 em sucesso, 0 se dinheiro em mão insuficiente.
 * Req 4.3
 */
stock Economia_Depositar(playerid, valor)
{
    if(valor <= 0) return 0;
    if(pInfo[playerid][pi_dinheiro] < valor) return 0;

    // Operação atômica: ambos os campos alterados na mesma instrução de fluxo
    pInfo[playerid][pi_dinheiro] -= valor;
    pInfo[playerid][pi_banco]    += valor;

    GivePlayerMoney(playerid, -valor);

    new tipo[]      = "Deposito";
    new envolvido[] = "Banco";
    Economia_RegistrarExtrato(playerid, tipo, valor, envolvido, gettime());

    return 1;
}

/**
 * Saque atômico: deduz do banco e credita no dinheiro em mão.
 * Respeita o teto de ECO_CASH_MAXIMO para o cash.
 * Retorna 1 em sucesso, 0 se banco insuficiente ou teto de cash atingido.
 * Req 4.4, 4.5
 */
stock Economia_Sacar(playerid, valor)
{
    if(valor <= 0) return 0;
    if(pInfo[playerid][pi_banco] < valor) return 0;

    // Verificar se o saque não ultrapassa o teto de cash (Req 4.4)
    new novoCash = pInfo[playerid][pi_dinheiro] + valor;
    if(novoCash > ECO_CASH_MAXIMO) return 0;

    // Operação atômica
    pInfo[playerid][pi_banco]    -= valor;
    pInfo[playerid][pi_dinheiro] += valor;

    GivePlayerMoney(playerid, valor);

    new tipo[]      = "Saque";
    new envolvido[] = "Banco";
    Economia_RegistrarExtrato(playerid, tipo, valor, envolvido, gettime());

    return 1;
}

// =============================================================================
// Sub-tarefa 3 — Transferência banco→banco, CreditarBanco e DebitarBanco
// =============================================================================

/**
 * Crédito direto no saldo bancário (salários, seguros, reversões, etc.).
 * O saldo bancário não possui limite superior.
 * Req 4.1
 */
stock Economia_CreditarBanco(playerid, valor)
{
    if(valor <= 0) return;
    pInfo[playerid][pi_banco] += valor;
}

/**
 * Débito direto do saldo bancário.
 * Retorna 1 em sucesso, 0 se saldo insuficiente.
 * Req 4.1
 */
stock Economia_DebitarBanco(playerid, valor)
{
    if(valor <= 0) return 0;
    if(pInfo[playerid][pi_banco] < valor) return 0;

    pInfo[playerid][pi_banco] -= valor;
    return 1;
}

/**
 * Transferência atômica banco→banco entre dois jogadores.
 * Registra extrato em ambos os lados (remetente e destinatário).
 * Retorna 1 em sucesso, 0 se saldo do remetente insuficiente.
 * Req 4.6
 */
stock Economia_Transferir(remetente, destinatario, valor)
{
    if(valor <= 0) return 0;
    if(remetente == destinatario) return 0;
    if(pInfo[remetente][pi_banco] < valor) return 0;

    // Débito e crédito atômicos
    pInfo[remetente][pi_banco]    -= valor;
    pInfo[destinatario][pi_banco] += valor;

    new ts = gettime();

    // Extrato do remetente
    new nomeDestinatario[MAX_PLAYER_NAME];
    GetPlayerName(destinatario, nomeDestinatario, MAX_PLAYER_NAME);
    new tipoSaida[] = "Transf. Enviada";
    Economia_RegistrarExtrato(remetente, tipoSaida, valor, nomeDestinatario, ts);

    // Extrato do destinatário
    new nomeRemetente[MAX_PLAYER_NAME];
    GetPlayerName(remetente, nomeRemetente, MAX_PLAYER_NAME);
    new tipoEntrada[] = "Transf. Recebida";
    Economia_RegistrarExtrato(destinatario, tipoEntrada, valor, nomeRemetente, ts);

    return 1;
}

// =============================================================================
// Sub-tarefa 4 — Ring buffer do extrato (últimas 50 transações por jogador)
// =============================================================================

/**
 * Registra uma entrada no extrato do jogador usando ring buffer circular.
 * Quando o buffer está cheio (50 entradas), a entrada mais antiga é
 * sobrescrita pela nova (política FIFO circular).
 * Req 4.7
 */
stock Economia_RegistrarExtrato(playerid, tipo[], valor, envolvido[], timestamp)
{
    new idx = pExtratoIdx[playerid];

    // Copia tipo (máx 31 chars + null)
    strcopy(pExtrato[playerid][idx][ext_tipo], tipo, 32);

    // Valor e timestamp
    pExtrato[playerid][idx][ext_valor]     = valor;
    pExtrato[playerid][idx][ext_timestamp] = timestamp;

    // Copia envolvido (máx MAX_PLAYER_NAME chars)
    strcopy(pExtrato[playerid][idx][ext_envolvido], envolvido, MAX_PLAYER_NAME);

    // Avança índice circular: volta ao 0 quando chega ao fim
    pExtratoIdx[playerid] = (idx + 1) % ECO_MAX_EXTRATO;
}

// =============================================================================
// Sub-tarefa 5 — Ciclos de salários e impostos
// =============================================================================

/**
 * Ciclo de salários — chamado pelo TIMER_SALARIOS (a cada 1h real).
 *
 * Para cada jogador conectado e logado:
 *   1. Credita o salário base da profissão ativa no saldo bancário.
 *   2. Calcula a estimativa de imposto futuro sobre o novo patrimônio e
 *      exibe aviso informativo (Req 21.1 — ciclo econômico autossustentável).
 *
 * Req 4.8, 21.1
 */
stock Economia_CicloSalarios()
{
    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;

        new E_Profissao:prof = pInfo[i][pi_profissao];
        new salario = gSalarioBase[_:prof];

        // Credita salário no banco
        Economia_CreditarBanco(i, salario);

        // Registra extrato de crédito de salário
        new tipo[]      = "Salario";
        new envolvido[] = "Servidor";
        Economia_RegistrarExtrato(i, tipo, salario, envolvido, gettime());

        // Notifica o jogador sobre o crédito
        new msg[192];
        format(msg, sizeof(msg),
            "[ECONOMIA] Salario de R$%d creditado no seu banco. Saldo: R$%d.",
            salario, pInfo[i][pi_banco]);
        SendClientMessage(i, COR_CONFIRMACAO, msg);

        // --- Req 21.1: estimativa de imposto para tornar o ciclo autossustentável ---
        // Calcula alíquota sobre o patrimônio atual (dinheiro + banco pós-salário)
        new patrimonio = pInfo[i][pi_dinheiro] + pInfo[i][pi_banco];
        new Float:aliquota = Economia_CalcularAliquota(patrimonio);
        new impostoEstimado = floatround(float(patrimonio) * aliquota, floatround_floor);

        if(impostoEstimado > 0)
        {
            new aviso[192];
            format(aviso, sizeof(aviso),
                "[ECONOMIA] Imposto estimado: R$%d (%.0f%%). Proximo ciclo em ~30 dias.",
                impostoEstimado, aliquota * 100.0);
            SendClientMessage(i, COR_SERVIDOR1, aviso);
        }
    }
}

/**
 * Ciclo de impostos — chamado pelo TIMER_IMPOSTOS (a cada 30 dias reais).
 *
 * Calcula o imposto sobre o patrimônio líquido (dinheiro + banco) e
 * desconta preferencialmente do banco; se insuficiente, desconta do
 * dinheiro em mão; se nem isso cobrir, cobra tudo disponível (Req 4.10).
 *
 * Alíquotas (Req 4.9):
 *   Patrimônio < R$50.000           → 2%
 *   R$50.000  ≤ Patrimônio < R$200.000 → 5%
 *   R$200.000 ≤ Patrimônio < R$500.000 → 8%
 *   Patrimônio ≥ R$500.000          → 12%
 *
 * Req 4.9, 4.10, 21.1
 */
stock Economia_CicloImpostos()
{
    foreach(new i : Player)
    {
        if(!pInfo[i][pi_logado]) continue;

        // Patrimônio líquido: dinheiro em mão + saldo bancário
        // Veículos e casas são somados em Personagem_Patrimonio (sistema_personagem),
        // mas este módulo não depende dele para evitar dependência circular.
        new patrimonio = pInfo[i][pi_dinheiro] + pInfo[i][pi_banco];

        // Calcula alíquota e imposto
        new Float:aliquota = Economia_CalcularAliquota(patrimonio);
        new imposto = floatround(float(patrimonio) * aliquota, floatround_floor);
        if(imposto <= 0) continue;

        new msg[256];
        new tipo[]      = "Imposto Mensal";
        new envolvido[] = "Receita Federal";

        if(pInfo[i][pi_banco] >= imposto)
        {
            // Caso padrão: desconta do banco (Req 4.9)
            pInfo[i][pi_banco] -= imposto;
            Economia_RegistrarExtrato(i, tipo, imposto, envolvido, gettime());

            format(msg, sizeof(msg),
                "[ECONOMIA] Imposto mensal de R$%d (%.0f%%) debitado do seu banco.",
                imposto, aliquota * 100.0);
            SendClientMessage(i, COR_SERVIDOR1, msg);
        }
        else
        {
            // Saldo bancário insuficiente — desconta o restante do dinheiro em mão (Req 4.10)
            new debitadoBanco = pInfo[i][pi_banco];
            new restante      = imposto - debitadoBanco;

            pInfo[i][pi_banco] = 0;

            if(pInfo[i][pi_dinheiro] >= restante)
            {
                pInfo[i][pi_dinheiro] -= restante;
                GivePlayerMoney(i, -restante);
            }
            else
            {
                // Nem o dinheiro em mão cobre o restante: cobra tudo que há
                new debitadoCash = pInfo[i][pi_dinheiro];
                pInfo[i][pi_dinheiro] = 0;
                GivePlayerMoney(i, -debitadoCash);
            }

            Economia_RegistrarExtrato(i, tipo, imposto, envolvido, gettime());

            format(msg, sizeof(msg),
                "[ECONOMIA] Imposto mensal de R$%d: banco insuficiente. Descontado do dinheiro em mao.",
                imposto);
            SendClientMessage(i, COR_ERRO, msg);
        }
    }
}

// =============================================================================
// Funções auxiliares internas
// =============================================================================

/**
 * Retorna a alíquota de imposto (Float) para o patrimônio informado.
 * Utilizado por Economia_CicloSalarios (estimativa) e Economia_CicloImpostos.
 * Req 4.9
 *
 *   Patrimônio < R$50.000           → 2%  (0.02)
 *   R$50.000  ≤ Patrimônio < R$200.000 → 5%  (0.05)
 *   R$200.000 ≤ Patrimônio < R$500.000 → 8%  (0.08)
 *   Patrimônio ≥ R$500.000          → 12% (0.12)
 */
static stock Float:Economia_CalcularAliquota(patrimonio)
{
    if(patrimonio < 50000)
        return 0.02;
    if(patrimonio < 200000)
        return 0.05;
    if(patrimonio < 500000)
        return 0.08;
    return 0.12;
}
