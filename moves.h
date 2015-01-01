/*	moves.h
	by Rob Upcraft */

//Move structure
typedef struct
{
	char pieceId;
	char startRow;
	char startCol;
	char endRow;
	char endCol;
	int boardScore;
} Move;

//MoveSet structure
typedef struct
{
	int cap;
	int size;
	Move** moves;
} MoveSet;

/*
 * Struct for bestMoveHelper()
 */
typedef struct
{
	int index;
	int value;
} MoveDef;

//Movement defines
#define RAND_RANGE 20	//Range in which random terminal node selection is allowed
#define CHECKMATE_VALUE 50000;
#define CASTLE -2
#define KINGSIDE -3
#define QUEENSIDE -4

//Evaluation function defines
#define ROB 1
#define TURING 2
#define BERLINER 3

//Cluster values
#define HEAD 0

/* Prototypes */
MoveSet* newMoveSet();
MoveSet* getLegalMoves(char* board, int color, Move* prevMove, int evaluator);
Move* getMove(MoveSet* set, int index);
char kingInCheck(char* board, char color);
char pieceSafe(char* board, int row, int col);
char spaceSafe(char* board, char color, int row, int col);
int evaluateBoard(char* board, int functionId);
void addMove(MoveSet* set, char pieceId, int startRow, int startCol, int endRow, int endCol, int boardScore);
void printMove(Move* move);
char* moveToString(Move* move);
char getColumnChar(int col);
void printMoves(MoveSet* moves);
void destroyMoveSet(MoveSet* set);
void executeMove(char* board, Move* move, Move* prevMove);
char blackCastleInvalid();
char whiteCastleInvalid();
void setBlackCastleInvalid(char validity);
void setWhiteCastleInvalid(char validity);
Move* bestMove(char* board, int color, Move* prevMove, int depth, int evaluator);
MoveDef* bestMoveHelper(char* board, int color, int depth, MoveSet* legalMoves, Move* prevMove, int evaluator);
Move* newMove();
Move* constructMove(char pieceId, char startRow, char startCol, char endRow, char endCol, int boardScore);
Move* cloneMove(Move* m);
MoveSet* subMoveSet(MoveSet* set, int start, int length);

//Cluster prototypes
void sendMove(int receiver, Move* move);
Move* receiveMove();
void sendMoveSet(int receiver, MoveSet* set);	//send a moveset to a slave node
MoveSet* receiveMoveSet();	//receive a moveset from the head node
void sendMoveDef(MoveDef* best);	//sends best move to head node
MoveDef* receiveMoveDef(int sender);	//receive the best move from a slave node
