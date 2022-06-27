# server state
uint8 READY     = 0 # nonterminal
# task state
uint8 ACTIVE    = 1 # nonterminal
uint8 SUCCEEDED = 2 # terminal
uint8 PREEMPTED = 3 # terminal
uint8 ABORTED   = 4 # terminal
uint8 REJECTED  = 5 # terminal
uint8 PAUSED    = 6 # nonterminal

string task_id
string type
bool success
uint8 status
string message