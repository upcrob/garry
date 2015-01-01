/*	
 * boardutil.h
 * by Rob Upcraft
 */

/* Defines */
#define EMPTY 0
#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6
#define BLACK 50  //black pieces are equal to [PIECE_ID] + BLACK
#define WHITE 0

char* buildNewBoard();
char* toString(char pieceId);
char getPieceAt(char* board, int row, int col);
void setPieceAt(char* board, int row, int col, char pieceId);
char isBlack(char pieceId);
char isWhite(char pieceId);
void cloneBoard(char* board, char* newBoard);
const char* positionString(int row, int col);
int totalPieces(char* board);
void printBoard(char* board);