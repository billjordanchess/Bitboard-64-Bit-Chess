#include "globals.h"//29/10/26

int side, xside;
int fifty;
int ply, hply;
int nodes;

int board[64];

int history[64][64];
int square_score[2][6][64];
int king_endgame[2][64];

int pawn_mat[2];
int piece_mat[2];
int passed[2][64];

move_data move_list[MOVE_STACK];
int first_move[MAX_PLY];

game_data game_list[GAME_STACK];

static void SetKnightMoves();
static void SetKingMoves();
static void SetLinePieces();

const int piece_value[6] =
{
	100, 300, 300, 500, 900, 10000
};

static const int init_color[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

const int init_board[64] =
{
	R, N, B, Q, K, B, N, R,
	P, P, P, P, P, P, P, P,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	P, P, P, P, P, P, P, P,
	R, N, B, Q, K, B, N, R
};

const int col[64] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7
};

const int row[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7
};

const int nediag[64] =
{
  14,13,12,11,10, 9, 8, 7,
  13,12,11,10, 9, 8, 7, 6,
  12,11,10, 9, 8, 7, 6, 5,
  11,10, 9, 8, 7, 6, 5, 4,
  10, 9, 8, 7, 6, 5, 4, 3,
   9, 8, 7, 6, 5, 4, 3, 2,
   8, 7, 6, 5, 4, 3, 2, 1,
   7, 6, 5, 4, 3, 2, 1, 0
};

const int nwdiag[64] =
{
   7, 8, 9,10,11,12,13,14,
   6, 7, 8, 9,10,11,12,13,
   5, 6, 7, 8, 9,10,11,12,
   4, 5, 6, 7, 8, 9,10,11,
   3, 4, 5, 6, 7, 8, 9,10,
   2, 3, 4, 5, 6, 7, 8, 9,
   1, 2, 3, 4, 5, 6, 7, 8,
   0, 1, 2, 3, 4, 5, 6, 7
};

const int Flip[64] =
{
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

static int pawn_score[64] =
{
	0,   0,   0,   0,   0,   0,   0,   0,
	0,   2,   4, -12, -12,   4,   2,   0,
	0,   2,   4,   4,   4,   4,   2,   0,
	0,   2,   4,   8,   8,   4,   2,   0,
	0,   2,   4,   8,   8,   4,   2,   0,
	4,   8,  10,  16,  16,  10,   8,   4,
  100, 100, 100, 100, 100, 100, 100, 100,
	0,   0,   0,   0,   0,   0,   0,   0
};

static int knight_score[64] =
{
  -30, -20, -10,  -8,  -8, -10, -20, -30,
  -16,  -6,  -2,   0,   0,  -2,  -6, -16,
   -8,  -2,   4,   6,   6,   4,  -2,  -8,
   -5,   0,   6,   8,   8,   6,   0,  -5,
   -5,   0,   6,   8,   8,   6,   0,  -5,
  -10,  -2,   4,   6,   6,   4,  -2, -10,
  -20, -10,  -2,   0,   0,  -2, -10, -20,
 -150, -20, -10,  -5,  -5, -10, -20,-150
};

static int bishop_score[64] =
{
	-10, -10, -12, -10, -10, -12, -10, -10,
	  0,   4,   4,   4,   4,   4,   4,   0,
	  2,   4,   6,   6,   6,   6,   4,   2,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  2,   4,   6,   6,   6,   6,   4,   2,
	-10,   4,   4,   4,   4,   4,   4, -10,
	-10, -10, -10, -10, -10, -10, -10, -10
};

static int rook_score[64] =
{
  4,  4,  4,  6,  6,  4,  4,  4,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
 20, 20, 20, 20, 20, 20, 20, 20,
 10, 10, 10, 10, 10, 10, 10, 10
};

static int queen_score[64] =
{
	-10, -10,  -6,  -4,  -4,  -6, -10, -10,
	-10,   2,   2,   2,   2,   2,   2, -10,
	  2,   2,   2,   3,   3,   2,   2,   2,
	  2,   2,   3,   4,   4,   3,   2,   2,
	  2,   2,   3,   4,   4,   3,   2,   2,
	  2,   2,   2,   3,   3,   2,   2,   2,
	-10,   2,   2,   2,   2,   2,   2, -10,
	-10, -10,   2,   2,   2,   2, -10, -10
};

static int king_score[64] =
{
   20,  20,  20, -40,  10, -60,  20,  20,
   15,  20, -25, -30, -30, -45,  20,  15,
  -48, -48, -48, -48, -48, -48, -48, -48,
  -48, -48, -48, -48, -48, -48, -48, -48,
  -48, -48, -48, -48, -48, -48, -48, -48,
  -48, -48, -48, -48, -48, -48, -48, -48,
  -48, -48, -48, -48, -48, -48, -48, -48,
  -48, -48, -48, -48, -48, -48, -48, -48
};

static int king_endgame_score[64] =
{
	  0,   8,  16,  18,  18,  16,  8,   0,
	  8,  16,  24,  32,  32,  24,  16,  8,
	 16,  24,  32,  40,  40,  32,  24,  16,
	 25,  32,  40,  48,  48,  40,  32,  25,
	 25,  32,  40,  48,  48,  40,  32,  25,
	 16,  24,  32,  40,  40,  32,  24,  16,
      8,  16,  24,  32,  32,  24,  16,  8,
	  0,  8,   16,  18,  18,  16,  8,   0
};

int passed_score[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	60,  60,  60,  60  ,60, 60, 60, 60,
	30, 30, 30, 30, 30, 30, 30, 30,
	15, 15, 15, 15,15, 15, 15, 15,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	0, 0, 0, 0, 0, 0, 0, 0
};

int castle_mask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	7, 15, 15, 15,  3, 15, 15, 11
};

void SetTables()
{
	for (int x = 0; x < 64; x++)
	{
		square_score[WHITE][P][x] = pawn_score[x] + piece_value[P];
		square_score[WHITE][N][x] = knight_score[x] + piece_value[N];
		square_score[WHITE][B][x] = bishop_score[x] + piece_value[B];
		square_score[WHITE][R][x] = rook_score[x] + piece_value[R];
		square_score[WHITE][Q][x] = queen_score[x] + piece_value[Q];
		square_score[WHITE][K][x] = king_score[x];

		square_score[BLACK][P][x] = pawn_score[Flip[x]] + piece_value[P];
		square_score[BLACK][N][x] = knight_score[Flip[x]] + piece_value[N];
		square_score[BLACK][B][x] = bishop_score[Flip[x]] + piece_value[B];
		square_score[BLACK][R][x] = rook_score[Flip[x]] + piece_value[R];
		square_score[BLACK][Q][x] = queen_score[Flip[x]] + piece_value[Q];
		square_score[BLACK][K][x] = king_score[Flip[x]];

		king_endgame[WHITE][x] = king_endgame_score[x] - square_score[WHITE][K][x];
		king_endgame[BLACK][x] = king_endgame_score[x] - square_score[BLACK][K][x];
		passed[WHITE][x] = passed_score[Flip[x]];
		passed[BLACK][x] = passed_score[x];
	}
}

void InitBoard()
{
	memset(bit_pieces, 0, sizeof(bit_pieces));
	memset(bit_units, 0, sizeof(bit_units));
	bit_all = 0;
	for (int x = 0; x < 64; x++)
	{
		board[x] = EMPTY;
	}
	for (int x = 0; x < 64; ++x)
	{
		if (init_board[x] != EMPTY)
			AddPiece(init_color[x], init_board[x], x);
	}
	side = WHITE;
	xside = BLACK;
	fifty = 0;
	castle = 15;
	ply = 0;
	hply = 1;
	first_move[0] = 0;
	turn = 0;
}

void NewPosition()
{
	memset(pawn_mat, 0, sizeof(pawn_mat));
	memset(piece_mat, 0, sizeof(piece_mat));

	for (int i = 0; i < 64; i++)
	{
		if (board[i] != EMPTY)
		{
			if (bit_units[WHITE] & mask[i])
				AddPiece(WHITE, board[i], i);
			if (bit_units[BLACK] & mask[i])
				AddPiece(BLACK, board[i], i);
		}
	}
	currentkey = GetKey();
	currentlock = GetLock();
}

void Alg(int a, int b)
{
	Algebraic(a);
	Algebraic(b);
}

void Algebraic(int a)
{
	if (a < 0 || a > 63) return;
	char c[2] = "a";
	c[0] = 96 + 1 + col[a];
	cout << c << row[a] + 1;
}

/*dont need below here*/
int done[100];
int GetBest(int ply);//
void ShowAll(int ply)
{

	move_data* g;
	DisplayBoard();
	memset(done, 0, sizeof(done));

	cout << " ply ";
	cout << ply;
	cout << " nodes ";
	cout << nodes;
	cout << " side ";
	cout << side;
	cout << " xside ";
	cout << xside;
	cout << endl;

	cout << " one " << first_move[ply];
	cout << " two " << first_move[ply + 1];
	Alg(move_list[first_move[0]].from, move_list[first_move[0]].to);

	cout << endl;

	int j;
	for (int i = first_move[ply]; i < first_move[ply + 1]; i++)
		//   for(int i=first_move[ply+1];i<first_move[ply + 2];i++)
	{
		j = GetBest(ply);
		{
			//how to display current line?
			g = &move_list[j];
			Alg(move_list[j].from, move_list[j].to);
			cout << "\n";
			cout << " ";
			cout << " score ";
			cout << g->score;
			cout << endl;
		}
	}
	cout << endl;

	_getch();
}

int GetBest(int ply)
{
	move_data* g;
	int bestscore = -100000000;
	int best = 0;
	for (int i = 0; i < first_move[ply + 1] - first_move[ply]; i++)
	{
		if (done[i] == 1) continue;
		g = &move_list[first_move[ply] + i];
		if (g->from == 0 && g->to == 0)
			continue;//
		if (g->score > bestscore)
		{
			bestscore = g->score;
			best = i;
		}
	}
	if (best < 1000) done[best] = 1;//1000?
	return first_move[ply] + best;
}