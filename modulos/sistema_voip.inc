// =============================================================================
//   sistema_voip.inc — Streams SampVoice
//   Aurora Roleplay 2026
//   Requirements: 12.6–12.7
//   API: sampvoice.inc (SvCreate*, SvAttach*, SvDetach*, SvDelete*, SV_NULL)
// =============================================================================

// SV_STREAM: é alias de SV_PTR: que é vazio — usamos o tipo nativo
// SV_NULL = 0 (stream inválido)

// ---------------------------------------------------------------------------
// Arrays de streams
// ---------------------------------------------------------------------------
new SV_STREAM:streamLocal[MAX_PLAYERS];
new SV_STREAM:streamFaccao[MAX_ORGS];

// ---------------------------------------------------------------------------
// Inicialização
// ---------------------------------------------------------------------------
stock VoIP_InicializarTodos()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
        streamLocal[i] = SV_STREAM:SV_NULL;

    for(new o = 0; o < MAX_ORGS; o++)
        streamFaccao[o] = SV_STREAM:SV_NULL;
}

// ---------------------------------------------------------------------------
// Stream local por jogador (alcance 15m via SvCreateSLStreamAtPlayer) — Req 12.6
// ---------------------------------------------------------------------------
stock VoIP_CriarStreamLocal(playerid)
{
    if(streamLocal[playerid] != SV_STREAM:SV_NULL)
        SvDeleteStream(streamLocal[playerid]);

    // Cria stream dinâmico centrado no jogador com raio de 15m
    streamLocal[playerid] = SvCreateSLStreamAtPlayer(CHAT_RAIO_LOCAL, playerid);

    if(streamLocal[playerid] != SV_STREAM:SV_NULL)
    {
        // O jogador é speaker (transmite) e listener (ouve outros)
        SvAttachSpeakerToStream(streamLocal[playerid], playerid);
    }
}

stock VoIP_DestruirStreamLocal(playerid)
{
    if(streamLocal[playerid] != SV_STREAM:SV_NULL)
    {
        SvDetachAllSpeakersFromStream(streamLocal[playerid]);
        SvDetachAllListenersFromStream(streamLocal[playerid]);
        SvDeleteStream(streamLocal[playerid]);
        streamLocal[playerid] = SV_STREAM:SV_NULL;
    }
}

// ---------------------------------------------------------------------------
// Stream de facção (global, com efeito de volume reduzido) — Req 12.7
// ---------------------------------------------------------------------------
stock VoIP_CriarStreamFaccao(org_id)
{
    if(org_id < 0 || org_id >= MAX_ORGS) return;

    if(streamFaccao[org_id] != SV_STREAM:SV_NULL)
        SvDeleteStream(streamFaccao[org_id]);

    // Stream global (sem limite de distância)
    streamFaccao[org_id] = SvCreateGStream(0xFFAA0080);

    // Efeito de estática: reduz volume para 50%
    if(streamFaccao[org_id] != SV_STREAM:SV_NULL)
        SvStreamParameterSet(streamFaccao[org_id], SV_PARAMETER_VOLUME, 0.5);
}

stock VoIP_EntrarFaccao(playerid, org_id)
{
    if(org_id < 0 || org_id >= MAX_ORGS) return;

    if(streamFaccao[org_id] == SV_STREAM:SV_NULL)
        VoIP_CriarStreamFaccao(org_id);

    SvAttachListenerToStream(streamFaccao[org_id], playerid);
    SvAttachSpeakerToStream(streamFaccao[org_id], playerid);
}

stock VoIP_SairFaccao(playerid, org_id)
{
    if(org_id < 0 || org_id >= MAX_ORGS) return;
    if(streamFaccao[org_id] == SV_STREAM:SV_NULL) return;

    SvDetachListenerFromStream(streamFaccao[org_id], playerid);
    SvDetachSpeakerFromStream(streamFaccao[org_id], playerid);
}

// ---------------------------------------------------------------------------
// Stream privado para chamadas telefônicas — Req 11.4, 11.5
// ---------------------------------------------------------------------------
stock SV_STREAM:VoIP_CriarStreamChamada(playerid_a, playerid_b)
{
    new SV_STREAM:stream = SvCreateGStream();
    if(stream == SV_STREAM:SV_NULL) return SV_STREAM:SV_NULL;

    SvAttachListenerToStream(stream, playerid_a);
    SvAttachListenerToStream(stream, playerid_b);
    SvAttachSpeakerToStream(stream, playerid_a);
    SvAttachSpeakerToStream(stream, playerid_b);
    return stream;
}

stock VoIP_DestruirStreamChamada(SV_STREAM:stream, playerid_a, playerid_b)
{
    if(stream == SV_STREAM:SV_NULL) return;

    SvDetachListenerFromStream(stream, playerid_a);
    SvDetachListenerFromStream(stream, playerid_b);
    SvDetachSpeakerFromStream(stream, playerid_a);
    SvDetachSpeakerFromStream(stream, playerid_b);
    SvDeleteStream(stream);
}

// ---------------------------------------------------------------------------
// Callbacks SampVoice
// ---------------------------------------------------------------------------
forward SV_UINT:OnSvPlayerActivationKeyDown(SV_UINT:playerid, SV_UINT:keyid);
public SV_UINT:OnSvPlayerActivationKeyDown(SV_UINT:playerid, SV_UINT:keyid)
{
    #pragma unused playerid, keyid
    return 1;
}

forward SV_UINT:OnSvPlayerActivationKeyUp(SV_UINT:playerid, SV_UINT:keyid);
public SV_UINT:OnSvPlayerActivationKeyUp(SV_UINT:playerid, SV_UINT:keyid)
{
    #pragma unused playerid, keyid
    return 1;
}
