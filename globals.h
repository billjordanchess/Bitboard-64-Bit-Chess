#include <memory.h>//29/10/26
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include <assert.h>
#include <iostream>

using namespace std;

void ShowAll(int ply);//
void z();

#define BITBOARD unsigned __int64 
#define U64 unsigned __int64

extern unsigned int CAPTURE;
extern unsigned int INCHECK;
extern unsigned int ATTACK;
extern unsigned int CASTLE;
extern unsigned int QUIET;
extern unsigned int KILLER;
extern unsigned int PASSED6;
extern unsigned int PASSED7;
extern unsigned int PROMOTE;
extern unsigned int CHECK;
extern unsigned int DEFEND;
extern unsigned int PAWNMOVE;
extern unsigned int EP;
extern unsigned int MATETHREAT;
extern unsigned int PROMOTE_KNIGHT;
extern unsigned int PROMOTE_BISHOP;
extern unsigned int PROMOTE_ROOK;

enum {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

const int P = 0;
const int N = 1;
const int B = 2;
const int R = 3;
const int Q = 4;
const int K = 5;
const int EMPTY = 6;

const int WHITE = 0;
const int BLACK = 1;

const int MAX_PLY = 64;
const int MOVE_STACK = 4000;
const int GAME_STACK = 2000;

const int HASH_SCORE = 3000000;
const int CAPTURE_SCORE = 2000000;

constexpr int CASTLE_G1 = 1;
constexpr int CASTLE_C1 = 2;
constexpr int CASTLE_G8 = 4;
constexpr int CASTLE_C8 = 8;

struct move_data {
	int from;
	int to;
	unsigned int flags;
	int score;
};

struct game_data {
	int from;
	int to;
	unsigned int flags;
	int capture;
	int fifty;
	int castle;
	BITBOARD hashkey;
	BITBOARD lock;
};

extern move_data hash_move;

extern const int piece_value[6];
extern int pawn_mat[2];
extern int piece_mat[2];

extern int king_endgame[2][64];
extern int passed[2][64];

extern int side, xside;
extern int fifty;
extern int ply, hply;
extern int castle;
extern int castle_mask[64];

extern int nodes;

extern int board[64];
extern const int init_color[64];
extern const int init_board[64];

extern int history[64][64];

extern int square_score[2][6][64];

extern const int row[64];
extern const int col[64];
extern const int nwdiag[64];
extern const int nediag[64];

extern int first_move[MAX_PLY];
extern move_data move_list[MOVE_STACK];
extern game_data game_list[GAME_STACK];

//extern int hash_start,hash_dest;
extern BITBOARD currentkey, currentlock;

extern int fixed_time;
extern int fixed_depth;
extern int max_time;
extern int start_time;
extern int stop_time;
extern int max_depth;

extern BITBOARD collisions;

extern int turn;

extern const int Flip[64];

extern int row2[2][64];

extern BITBOARD bit_after[64][64];
extern BITBOARD bit_between[64][64];
extern BITBOARD bit_ray_edge[6][64];
extern BITBOARD bit_beyond[64][64];

extern int pawndouble[2][64];
extern int pawnplus[2][64];
extern int pawnleft[2][64];
extern int pawnright[2][64];

extern BITBOARD bit_pawncaptures[2][64];
extern BITBOARD bit_left[2][64];
extern BITBOARD bit_right[2][64];
extern BITBOARD bit_moves[6][64];

//current position
extern BITBOARD bit_pieces[2][7];
extern BITBOARD bit_units[2];
extern BITBOARD bit_all;

extern BITBOARD mask_passed[2][64];
extern BITBOARD mask_path[2][64];

extern BITBOARD mask_cols[64];

extern BITBOARD mask[64];

extern BITBOARD mask_isolated[64];

extern BITBOARD not_a_file;
extern BITBOARD not_h_file;

extern BITBOARD mask_edges;

//init.cpp
void InitBoard();
void SetTables();
void NewPosition();
void Alg(int a, int b);
void Algebraic(int a);
void SetMoves();

//search.cpp
void think();
int Search(int alpha, int beta, int depth);
int CaptureSearch(int alpha, int beta);
int ReCaptureSearch(int, int, const int, const int);
int Reps2();
void CheckUp();

//gen.cpp
void Gen(int, int);
//void AddMove(const int x,const int sq,const int);
void GenCaptures(const int, const int);

//attack.cpp
bool Attack(const int s, const int sq);
int GetDefender(const int s, const int x, const int sq, const BITBOARD occ);

//update.cpp
void UpdatePiece(const int s, const int p, const int from, const int to);
void RemovePiece(const int s, const int p, const int sq);
void AddPiece(const int s, const int p, const int sq);
int MakeMove(move_data m);
void UnMakeMove();

//eval.cpp
int Eval();

//hash.cpp
void RandomizeHash();
int Random(const int x);
void AddKey(const int, const int, const int);
BITBOARD GetKey();
BITBOARD GetLock();
void Free();
void AddHash(const int s, const move_data m, const int);
bool LookUp(const int s);

//main.cpp
int main();
int GetTime();
void NewPosition();
void DisplayBoard();
int Reps();

int NextBit(BITBOARD bb);

void PrintBitBoard(BITBOARD bb);

int lsb64(U64 bb);
int msb64(U64 bb);




