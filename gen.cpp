#include "globals.h"

void z()
{
	DisplayBoard();
	printf(" nodes %d ", nodes);
	_getch();
}

int capture_table[6][6][7];

extern BITBOARD bit_rings[3][64][6];
extern int cont[6][64][6][64];

static int move_count;
int castle;

unsigned int CAPTURE = (1 << 0);
unsigned int CHECK = (1 << 1);
unsigned int INCHECK = (1 << 2);
unsigned int PROMOTE = (1 << 3);
unsigned int PASSED7 = (1 << 4);
unsigned int CASTLE = (1 << 5);
unsigned int PAWNMOVE = (1 << 6);
unsigned int EP = (1 << 7);
unsigned int PROMOTE_KNIGHT = (1 << 8);
unsigned int PROMOTE_BISHOP = (1 << 9);
unsigned int PROMOTE_ROOK = (1 << 10);

void SetCaptureTable();

void Gen(const int s, const int sx);
void GenCaptures(const int s, const int xs);

static void GenEp(const int s);
static void GenCastle(const int s);
static void AddMove(const int x, const int sq, const int flags);
static void AddCapture(const int x, const int sq);
static void AddPromotion(const int from, const int to, const int flags);
static void AddPromoteCapture(const int x, const int sq);
static void AddEP(const int x, const int sq);

void Gen(const int s, const int xs)
{
	int from, to;

	BITBOARD b1, b2, b3;

	move_count = first_move[ply];

	GenEp(s);
	GenCastle(s);

	if (s == WHITE)
	{
		b1 = bit_pieces[WHITE][P] & ((bit_units[BLACK] & not_h_file) >> 7);
		b2 = bit_pieces[WHITE][P] & ((bit_units[BLACK] & not_a_file) >> 9);
		b3 = bit_pieces[WHITE][P] & ~(bit_all >> 8);
	}
	else
	{
		b1 = bit_pieces[BLACK][P] & ((bit_units[WHITE] & not_h_file) << 9);
		b2 = bit_pieces[BLACK][P] & ((bit_units[WHITE] & not_a_file) << 7);
		b3 = bit_pieces[BLACK][P] & ~(bit_all << 8);
	}
	while (b1)
	{
		from = NextBit(b1);
		b1 &= b1 - 1;
		to = pawnleft[s][from];
		if (row2[side][from] == 6)
			AddPromoteCapture(from, to);
		else
			AddCapture(from, to);
	}
	while (b2)
	{
		from = NextBit(b2);
		b2 &= b2 - 1;
		to = pawnright[s][from];
		if (row2[side][from] == 6)
			AddPromoteCapture(from, to);
		else
			AddCapture(from, to);
	}
	while (b3)
	{
		from = NextBit(b3);
		b3 &= b3 - 1;
		if (row2[side][from] == 6)
			AddPromotion(from, pawnplus[side][from], PROMOTE);
		else
			AddMove(from, pawnplus[side][from], 0);
		if (row2[s][from] == 1 && board[pawndouble[s][from]] == EMPTY)
		{
			AddMove(from, pawndouble[s][from], 0);
		}
	}
	b1 = bit_pieces[s][N];
	while (b1)
	{
		from = NextBit(b1);
		b1 &= b1 - 1;
		b2 = bit_moves[N][from] & bit_units[xs];
		while (b2)
		{
			to = NextBit(b2);
			b2 &= b2 - 1;
			AddCapture(from, to);
		}
		b2 = bit_moves[N][from] & ~bit_all;
		while (b2)
		{
			to = NextBit(b2);
			b2 &= b2 - 1;
			AddMove(from, to, 0);
		}
	}
	for (int piece = B; piece <= Q; piece++)
	{
		b1 = bit_pieces[s][piece];
		while (b1)
		{
			from = NextBit(b1);
			b1 &= b1 - 1;
			BITBOARD blocked = 0;

			for (int d = 0; d < 7; d++)
			{
				BITBOARD layer = bit_rings[piece - 2][from][d] & ~blocked;
				if (layer == 0)
					break;

				BITBOARD blockers = layer & bit_all;

				while (blockers) {
					int to = NextBit(blockers);
					blockers &= blockers - 1;
					blocked |= bit_beyond[from][to];
				}
			}

			BITBOARD attacks = bit_moves[piece][from];
			b3 = attacks & bit_units[xs] & ~blocked;
			while (b3)
			{
				to = NextBit(b3);
				b3 &= b3 - 1;
				AddCapture(from, to);
			}
			b3 = attacks & ~bit_all & ~blocked;
			while (b3)
			{
				to = NextBit(b3);
				b3 &= b3 - 1;
				AddMove(from, to, 0);
			}
		}
	}
	from = NextBit(bit_pieces[s][K]);
	b1 = bit_moves[K][from] & bit_units[xs];
	while (b1)
	{
		to = NextBit(b1);
		b1 &= b1 - 1;
		if (!Attack(xs, to))
			AddCapture(from, to);
	}
	b1 = bit_moves[K][from] & ~bit_all;
	while (b1)
	{
		to = NextBit(b1);
		b1 &= b1 - 1;
		if (!Attack(xs, to))
			AddMove(from, to, 0);
	}
	first_move[ply + 1] = move_count;
}

static void AddMove(const int from, const int to, const int flags)
{
	move_list[move_count].from = from;
	move_list[move_count].to = to;
	move_list[move_count].score = history[from][to];
	if (hply > 1)
	{
		int to2 = game_list[hply - 1].to;
		int piece2 = board[to2];
		int piece = board[from];
		move_list[move_count].score += cont[piece2][to2][piece][to];
	}
	move_list[move_count].flags = flags;
	move_count++;
}

static void AddCapture(const int from, const int to)
{
	move_list[move_count].from = from;
	move_list[move_count].to = to;
	move_list[move_count].flags = CAPTURE;

	int attacker = board[from];
	int captured = board[to];
	int defender_value = EMPTY;
	int defender = GetDefender(side, xside, to, bit_all);

	if (defender > -1)
		defender_value = board[defender];

	int score = capture_table[attacker][captured][defender_value];
	move_list[move_count].score = score + CAPTURE_SCORE;
	move_count++;
}

static void AddPromotion(const int from, const int to, const int flags)
{
	move_list[move_count].from = from;
	move_list[move_count].to = to;
	move_list[move_count].score = CAPTURE_SCORE + 800;
	move_list[move_count].flags = flags;
	move_count++;
}

static void AddPromoteCapture(const int from, const int to)
{
	move_list[move_count].from = from;
	move_list[move_count].to = to;
	move_list[move_count].flags = CAPTURE | PROMOTE;

	int captured = board[to];
	int score = piece_value[captured] - 100;
	int defender = GetDefender(side, xside, to, bit_all);

	if (defender == -1)
		score += 900;

	move_list[move_count].score = score + CAPTURE_SCORE;
	move_count++;
}

static void AddEP(const int from, const int to)
{
	move_list[move_count].from = from;
	move_list[move_count].to = to;
	move_list[move_count].flags = EP;
	move_list[move_count].score = 10 + CAPTURE_SCORE;
	move_count++;
}

static void GenEp(const int s)
{
	int ep = game_list[hply - 1].to;

	if (board[ep] == P && abs(game_list[hply - 1].from - ep) == 16)
	{
		if (col[ep] > 0 && mask[ep - 1] & bit_pieces[s][P])
			AddEP(ep - 1, pawnplus[side][ep]);
		if (col[ep] < 7 && mask[ep + 1] & bit_pieces[s][P])
			AddEP(ep + 1, pawnplus[side][ep]);
	}
}

static void GenCastle(const int s)
{
	if (s == WHITE)
	{
		if (castle & CASTLE_G1 && !(bit_between[H1][E1] & bit_all))
			AddMove(E1, G1, CASTLE);
		if (castle & CASTLE_C1 && !(bit_between[A1][E1] & bit_all))
			AddMove(E1, C1, CASTLE);
	}
	else
	{
		if (castle & CASTLE_G8 && !(bit_between[E8][H8] & bit_all))
			AddMove(E8, G8, CASTLE);
		if (castle & CASTLE_C8 && !(bit_between[E8][A8] & bit_all))
			AddMove(E8, C8, CASTLE);
	}
}

void GenCaptures(const int s, const int xs)
{
	move_count = first_move[ply];

	int from, to;
	BITBOARD b1, b2;

	if (s == WHITE)
	{
		b1 = bit_pieces[WHITE][P] & ((bit_units[BLACK] & not_h_file) >> 7);
		b2 = bit_pieces[WHITE][P] & ((bit_units[BLACK] & not_a_file) >> 9);
	}
	else
	{
		b1 = bit_pieces[BLACK][P] & ((bit_units[WHITE] & not_h_file) << 9);
		b2 = bit_pieces[BLACK][P] & ((bit_units[WHITE] & not_a_file) << 7);
	}
	while (b1)
	{
		from = NextBit(b1);
		b1 &= b1 - 1;
		to = pawnleft[s][from];
		AddCapture(from, to);
	}
	while (b2)
	{
		from = NextBit(b2);
		b2 &= b2 - 1;
		to = pawnright[s][from];
		AddCapture(from, to);
	}

	b1 = bit_pieces[s][N];
	while (b1)
	{
		from = NextBit(b1);
		b1 &= b1 - 1;
		b2 = bit_moves[N][from] & bit_units[xs];
		while (b2)
		{
			to = NextBit(b2);
			b2 &= b2 - 1;
			AddCapture(from, to);
		}
	}
	for (int piece = B; piece <= Q; piece++)
	{
		b1 = bit_pieces[s][piece];
		while (b1)
		{
			from = NextBit(b1);
			b1 &= b1 - 1;
			b2 = bit_moves[piece][from] & bit_units[xs];
			while (b2)
			{
				to = NextBit(b2);
				b2 &= b2 - 1;
				if (!(bit_between[from][to] & bit_all))
				{
					AddCapture(from, to);
				}
				b2 &= bit_after[from][to];
			}
		}
	}

	from = NextBit(bit_pieces[s][K]);
	b2 = bit_moves[K][from] & bit_units[xs];
	while (b2)
	{
		to = NextBit(b2);
		b2 &= b2 - 1;
		if (!Attack(xs, to))
			AddCapture(from, to);
	}
	first_move[ply + 1] = move_count;
}

void SetCaptureTable()
{
	for (int piece = 0; piece < 6; piece++)
		for (int captured = 0; captured < 6; captured++)
		{
			capture_table[piece][captured][EMPTY] = piece_value[captured];
			for (int defender = 0; defender < 6; defender++)
			{
				if (piece_value[piece] < piece_value[captured])
					capture_table[piece][captured][defender] = piece_value[captured];
				else if (piece_value[piece] == piece_value[captured])
					capture_table[piece][captured][defender] = piece_value[captured] / 10;
				else if (piece_value[piece] > piece_value[captured])
				{
					capture_table[piece][captured][defender] = piece_value[captured] - piece_value[piece];
				}
			}
		}
}

int GetCont(const int to2, const int piece2, const int to, const int piece)
{
	return cont[piece2][to2][piece][to];
}


