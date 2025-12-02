/**
* Top Speed 3 - Multiplayer Stubs
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*
* Stub implementations for multiplayer functionality when building without network support
* (e.g., Emscripten/WASM builds)
*/

#ifdef TOPSPEED_DISABLE_MULTIPLAYER

#include "RaceServer.h"
#include "RaceClient.h"
#include "LevelMultiplayer.h"
#include "Game.h"
#include "Menu.h"

// =============================================================================
// RaceServer stubs
// =============================================================================

RaceServer::RaceServer(Game* game) : m_game(game), m_server(nullptr), m_trackSelected(false), m_raceStarted(false), m_nRaceResults(0), m_finalizing(false), m_lastUpdateTime(0.0f)
{
    m_track[0] = '\0';
}

RaceServer::~RaceServer() {}

void RaceServer::initialize() {}
void RaceServer::finalize() {}
void RaceServer::run(Float elapsed) {}
void RaceServer::loadCustomTrack(Char* trackname) {}
void RaceServer::sendDisconnect(Int id) {}
void RaceServer::startRace() {}
void RaceServer::stopRace() {}
void RaceServer::abortRace() {}
void RaceServer::resetTrack() {}
void RaceServer::sendPacket(PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPacketTo(UInt to, PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPacketExceptTo(UInt to, PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPacketToNotReady(PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPacketToRacers(PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPacketToRacersExceptTo(UInt to, PacketBase* packet, UInt size, Boolean secure) {}
void RaceServer::sendPlayerDisconnected(UInt player) {}
void RaceServer::onPacket(UInt from, void* buffer, UInt size) {}
void RaceServer::onAddConnection(UInt id) {}
void RaceServer::onRemoveConnection(UInt id) {}
void RaceServer::onSessionLost() {}
UInt RaceServer::nRacers() { return 0; }

// =============================================================================
// RaceClient stubs
// =============================================================================

RaceClient::RaceClient(Game* game) : m_game(game), m_menu(nullptr), m_client(nullptr), m_playerNumber(0), m_playerId(0), m_nrOfLaps(0), m_trackSelected(false), m_nResults(0), m_prevPlayerState(PlayerState::waiting), m_playerState(PlayerState::waiting), m_connected(false), m_startRace(false), m_sessionLost(false), m_forceDisconnected(false), m_raceAborted(false), m_playerBumped(false), m_playerBumpX(0), m_playerBumpY(0), m_playerBumpSpeed(0)
{
    m_track[0] = '\0';
    for (int i = 0; i < NMAXPLAYERS; i++) {
        m_playerFinished[i] = false;
        m_playerFinalize[i] = false;
        m_playerStarted[i] = false;
        m_playerCrashed[i] = false;
        m_results[i] = 0;
    }
}

RaceClient::RaceClient(Menu* menu) : m_game(nullptr), m_menu(menu), m_client(nullptr), m_playerNumber(0), m_playerId(0), m_nrOfLaps(0), m_trackSelected(false), m_nResults(0), m_prevPlayerState(PlayerState::waiting), m_playerState(PlayerState::waiting), m_connected(false), m_startRace(false), m_sessionLost(false), m_forceDisconnected(false), m_raceAborted(false), m_playerBumped(false), m_playerBumpX(0), m_playerBumpY(0), m_playerBumpSpeed(0)
{
    m_track[0] = '\0';
    for (int i = 0; i < NMAXPLAYERS; i++) {
        m_playerFinished[i] = false;
        m_playerFinalize[i] = false;
        m_playerStarted[i] = false;
        m_playerCrashed[i] = false;
        m_results[i] = 0;
    }
}

RaceClient::~RaceClient() {}

void RaceClient::initialize() {}
void RaceClient::finalize() {}
void RaceClient::startEnumSessions(char* ipAddress) {}
void RaceClient::stopEnumSessions() {}
UInt RaceClient::nSessions() { return 0; }
UInt RaceClient::session(UInt i, DirectX::Client::SessionInfo& info) { return 1; /* always fail - no sessions */ }
UInt RaceClient::joinSession(UInt session) { return 0; }
UInt RaceClient::joinSessionAt(Char* ipAddress) { return 0; }
void RaceClient::sendData(PlayerData data, Boolean secure) {}
void RaceClient::resetTrack() {}
Boolean RaceClient::playerFinished(UInt player) { return false; }
Boolean RaceClient::playerFinalize(UInt player) { return false; }
Boolean RaceClient::playerStarted(UInt player) { return false; }
Boolean RaceClient::playerCrashed(UInt player) { return false; }
Boolean RaceClient::playerBumped(Int& bumpX, Int& bumpY, Int& bumpSpeed) { return false; }
void RaceClient::playerFinished() {}
void RaceClient::playerFinalize() {}
void RaceClient::playerStarted() {}
void RaceClient::playerCrashed() {}
void RaceClient::playerUpdateState() {}
UInt RaceClient::raceResults(UInt* &results) { results = nullptr; return 0; }
void RaceClient::resetResults() {}
void RaceClient::onPacket(UInt from, void* buffer, UInt size) {}
void RaceClient::onSessionLost() {}
void RaceClient::sendPacket(PacketBase* packet, UInt size, Boolean secure) {}

// =============================================================================
// LevelMultiplayer stubs
// =============================================================================

LevelMultiplayer::LevelMultiplayer(Game* game, UInt nrOfLaps, Char* track, Track::TrackData trackData, Boolean automaticTransmission, UInt vehicle, Char* vehicleFile)
    : Level(game, nrOfLaps, track, trackData, automaticTransmission, vehicle, vehicleFile), m_isServer(false), m_updateClient(0.0f), m_lastComment(0.0f), m_position(0), m_infoKeyReleased(false), m_startPressed(false)
{
    m_soundYouAre = nullptr;
    m_soundPlayer = nullptr;
    m_soundWaitingForPlayers = nullptr;
    m_soundHasJoinedServer = nullptr;
    m_soundHasJoinedRace = nullptr;
    m_soundHasLeftServer = nullptr;
    m_soundHasLeftRace = nullptr;
    m_soundPressEnterToStart = nullptr;
    m_soundRaceAborted = nullptr;
    m_soundConnectedToServer = nullptr;
    for (int i = 0; i < NMAXPLAYERS; i++) {
        m_soundPosition[i] = nullptr;
        m_soundPlayerNr[i] = nullptr;
        m_soundFinished[i] = nullptr;
    }
    for (int i = 0; i < NVEHICLES; i++) {
        m_soundVehicle[i] = nullptr;
    }
}

LevelMultiplayer::~LevelMultiplayer() {}

void LevelMultiplayer::initialize(Boolean isServer) { m_isServer = isServer; }
void LevelMultiplayer::finalize() {}
void LevelMultiplayer::run(Float elapsed) {}
void LevelMultiplayer::playerDisconnected(UInt player) {}
UInt LevelMultiplayer::calculatePlayerPerc(UInt player) { return 0; }
void LevelMultiplayer::onStart() {}
void LevelMultiplayer::onCrash() {}
void LevelMultiplayer::comment(Boolean automatic) {}
void LevelMultiplayer::updateResults() {}

#endif // TOPSPEED_DISABLE_MULTIPLAYER
