// pointsCTF.cpp
//

#include "bzfsAPI.h"

class pointsCTF : public bz_Plugin
{
public:
    virtual const char* Name () {return "pointsCTF";}
    virtual void Init(const char* config);
    virtual void Event(bz_EventData *eventData);
    // Some variables for keeping track of score and related.
    int playerCapValue[200] = { 0 };
    int playerSelfCapValue[200] = { 0 };
    // Actually the above could be a single two dimensional array, but for now, this makes it easier to read.

int isInRange(int player)
{
    if ((player >= 0) && (player <= 199))
        return 1;
    else 
        return 0;
}

void resetPlayerCaps(int playerID) 
{
    if (isInRange(playerID) == 1) {
        playerCapValue[playerID]     = 0;
        playerSelfCapValue[playerID] = 0;
    }
}

void resetAllPlayerCaps(void)
{
    for (int i = 0; i < 200; i++)
    {
        resetPlayerCaps(i);
    }
}

void setPlayerScores(int playerID) 
{
    if (isInRange(playerID) == 1) {
        bz_setPlayerWins(playerID, playerCapValue[playerID]);
        bz_setPlayerLosses(playerID, playerSelfCapValue[playerID]);
    }
}

void resetAllPlayersScore(void) 
{
    bz_APIIntList *player_list = bz_newIntList();
    bz_getPlayerIndexList(player_list);
    
    for (unsigned int i = 0; i < player_list->size(); i++) 
    {
        if (bz_getPlayerTeam(player_list->get(i)) != eNoTeam) {// If not equal to eNoTeam, assume valid player. (Inefficient to some extent, since we have observers "reset", but it works.)
            bz_setPlayerTKs(i, 0);
            bz_setPlayerLosses(i, 0);
            bz_setPlayerWins(i, 0);
        }
    }
    bz_deleteIntList(player_list);

}

};

BZ_PLUGIN(pointsCTF)

void pointsCTF::Init( const char* /*commandLine*/ )
{
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_eCaptureEvent);
    Register(bz_eGameStartEvent);
    Register(bz_eGameEndEvent);
    Register(bz_ePlayerScoreChanged);
}

void pointsCTF::Event ( bz_EventData *eventData )
{
    switch (eventData->eventType) 
    {
        case bz_ePlayerJoinEvent: {
        bz_PlayerJoinPartEventData_V1* joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
        
        resetPlayerCaps(joinData->playerID);

    }break;
        
        case bz_ePlayerPartEvent: {
        bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
        
        resetPlayerCaps(partData->playerID);

    }break;
    
    case bz_ePlayerDieEvent: {
        bz_PlayerDieEventData_V2* deathData = (bz_PlayerDieEventData_V2*)eventData;
        
        int victim = deathData->playerID;
        int killer = deathData->killerID;
        
        if (isInRange(victim) == 1) 
        {
            if (bz_getPlayerWins(victim) != playerCapValue[victim]) {
                bz_setPlayerWins(victim, playerCapValue[victim]);
            }
            
            if (bz_getPlayerLosses(victim) != playerSelfCapValue[victim]) {
                bz_setPlayerLosses(victim, playerSelfCapValue[victim]);
            }
        }
        
        if (isInRange(killer) == 1) 
        {
            if (bz_getPlayerWins(killer) != playerCapValue[killer]) {
                bz_setPlayerWins(killer, playerCapValue[killer]);
            }
            
            if (bz_getPlayerLosses(killer) != playerSelfCapValue[killer]) {
                bz_setPlayerLosses(killer, playerSelfCapValue[killer]);
            }
        }

    }break;
    
    
    case bz_eCaptureEvent: {
        bz_CTFCaptureEventData_V1* captureData = (bz_CTFCaptureEventData_V1*)eventData;
        
        bz_eTeamType capTeam = captureData->teamCapping;
        bz_eTeamType cappedTeam = captureData->teamCapped;
        
        int capPlayer = captureData->playerCapping;
        bz_eTeamType playerTeam = bz_getPlayerTeam(capPlayer);
 
        if (isInRange(capPlayer) == 1) { // Because we can only update valid slots.
            // Update arrays.
            if ((playerTeam != eNoTeam) && (playerTeam != cappedTeam)) {
                playerCapValue[capPlayer]++;
                bz_setPlayerWins(capPlayer, playerCapValue[capPlayer]);
            } else {
                playerSelfCapValue[capPlayer]++;
                bz_setPlayerLosses(capPlayer, playerSelfCapValue[capPlayer]);
            }
            
            // Update player Scores.
            
            
        }

// Data
// ---
// (bz_eTeamType) teamCapped - The team whose flag was captured.
// (bz_eTeamType) teamCapping - The team who did the capturing.
// (int)          playerCapping - The player who captured the flag.
// (float[3])     pos - The world position(X,Y,Z) where the flag has been captured
// (float)        rot - The rotational orientation of the capturing player
// (double)       eventTime - This value is the local server time of the event.
    }break;
    
    case bz_eGameStartEvent: {
        bz_GameStartEndEventData_V2* gameStartData = (bz_GameStartEndEventData_V2*)eventData;
        // We reset all player caps.
        resetAllPlayerCaps();
        resetAllPlayersScore();

    }break;
    
    case bz_eGameEndEvent: {
        bz_GameStartEndEventData_V2* gameEndData = (bz_GameStartEndEventData_V2*)eventData;
        
        resetAllPlayerCaps();
        
        //resetAllPlayersScore(); //
        // For now, no need to reset scores, players may wish to view scores and see how many caps made by each.
        // Unsure if a public message announcing captures made by players makes sense...

    }break;
    
    case bz_ePlayerScoreChanged: {
        bz_PlayerScoreChangeEventData_V1* scoreChangeData = (bz_PlayerScoreChangeEventData_V1*)eventData;
        
        // We use wins for captures, losses for self-captures, if there is team kills, we simply set to zero. 
        bz_eScoreElement scoreElement = scoreChangeData->element;
        int playerScoreID = scoreChangeData->playerID;
        int scoreValue = scoreChangeData->thisValue;
        
        
        if (isInRange(playerScoreID) == 1) 
        {
            if (scoreElement == bz_eWins) {
                
                if (scoreValue != playerCapValue[playerScoreID]) {
                    bz_setPlayerWins(playerScoreID, playerCapValue[playerScoreID]);
                }
                
            } else if (scoreElement == bz_eLosses) {
                
                if (scoreValue != playerSelfCapValue[playerScoreID]) {
                    bz_setPlayerLosses(playerScoreID, playerSelfCapValue[playerScoreID]);
                }
                
            } else { // bz_eTKs
                
                if (scoreValue != 0) { // Only possibility is team kill value.
                    bz_setPlayerTKs(playerScoreID, 0); // Erase team kills as we've changed the scoring system.
                }
                
            }
        }
        


// Data
// ---
// (int)          playerID - Player that has had a change of score.
// (bz_eScoreElement) element - The type of score that is being changed.
// (int)          thisValue - The new amount of element score the playerID has.
// (int)          lastValue - The old amount of element score the playerID had.
// (double)       eventTime - Time local server time for the event.
    }break;
    
    default: {
    }break;
        
    
    }
}


