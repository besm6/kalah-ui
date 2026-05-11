#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// All functions use void* handles so Swift can import them without a typed struct.
void *kalah_create(void);
void  kalah_destroy(void *h);

// App state: 0=Welcome 1=EnterName 2=SelectGender 3=SelectDifficulty 4=Playing
int  kalah_get_app_state(const void *h);
void kalah_proceed_from_welcome(void *h);
void kalah_submit_name(void *h, const char *name);  // truncated to 24 chars
void kalah_select_gender(void *h, int g);           // 0=unknown 1=male 2=female
void kalah_select_level(void *h, int l);            // 1=novice..4=master

// Returns: 0=INVALID 1=SWITCH_TURN 2=EXTRA_TURN
int  kalah_sow(void *h, int pit);         // USER move, pit index 0-5
int  kalah_do_ai_move(void *h, int pit);  // JINN move, pit index 0-5
int  kalah_select_ai_move(void *h);       // returns best pit index for JINN
void kalah_new_game(void *h);

// Fills caller-provided 14-int buffer:
// [0-5]=USER pits, [6]=USER kalah, [7-12]=JINN pits, [13]=JINN kalah
void kalah_get_pits(const void *h, int *out_pits);

int  kalah_current_player(const void *h);  // 0=USER 1=JINN
int  kalah_is_game_over(const void *h);    // 1 if over, 0 otherwise
int  kalah_winner(const void *h);          // 0=user wins 1=jinn wins -1=tie
void kalah_collect_remaining(void *h);     // sweep stones into kalahs at game end

// Returned pointer is valid until the next call on this handle
const char *kalah_get_user_name(const void *h);

#ifdef __cplusplus
}
#endif
