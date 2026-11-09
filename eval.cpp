#include "globals.h"

constexpr int ISOLATED = 20;

void SetKingDefences();

int Eval();

static int EvalPawn(const int s, const int x);
static int EvalRook(const int s, const int x);

static int queenside_pawns[2], kingside_pawns[2];

extern BITBOARD mask_kingside;
extern BITBOARD mask_queenside;

int kingside_defence[2][64];
int queenside_defence[2][64];

void SetKingDefences()
{
	memset(kingside_defence, 0, sizeof(kingside_defence));
	kingside_defence[0][F2] = 8;
	kingside_defence[0][G2] = 10;
	kingside_defence[0][H2] = 8;
	kingside_defence[0][F3] = 8;
	kingside_defence[0][G3] = 6;
	kingside_defence[0][H3] = 8;
	kingside_defence[1][F7] = 8;
	kingside_defence[1][G7] = 10;
	kingside_defence[1][H7] = 8;
	kingside_defence[1][F6] = 8;
	kingside_defence[1][G6] = 6;
	kingside_defence[1][H6] = 8;

	memset(queenside_defence, 0, sizeof(queenside_defence));
	queenside_defence[0][A2] = 8;
	queenside_defence[0][B2] = 10;
	queenside_defence[0][C2] = 8;
	queenside_defence[0][A3] = 8;
	queenside_defence[0][B3] = 6;
	queenside_defence[0][C3] = 8;
	queenside_defence[1][A7] = 8;
	queenside_defence[1][B7] = 10;
	queenside_defence[1][C7] = 8;
	queenside_defence[1][A6] = 8;
	queenside_defence[1][B6] = 6;
	queenside_defence[1][C6] = 8;
}

int Eval()
{
	int score[2] = { 0,0 };

	memset(queenside_pawns, 0, sizeof(queenside_pawns));
	memset(kingside_pawns, 0, sizeof(kingside_pawns));

	BITBOARD b1;
	int sq;

	for (int x = 0; x < 2; x++)
	{
		b1 = bit_pieces[x][P];
		while (b1)
		{
			sq = NextBit(b1);
			b1 &= b1 - 1;
			score[x] += square_score[x][P][sq];
			score[x] += EvalPawn(x, sq);
		}
		b1 = bit_pieces[x][N];
		while (b1)
		{
			sq = NextBit(b1);
			b1 &= b1 - 1;
			score[x] += square_score[x][N][sq];
		}
		b1 = bit_pieces[x][B];
		while (b1)
		{
			sq = NextBit(b1);
			b1 &= b1 - 1;
			score[x] += square_score[x][B][sq];
		}
		b1 = bit_pieces[x][R];
		while (b1)
		{
			sq = NextBit(b1);
			b1 &= b1 - 1;
			score[x] += square_score[x][R][sq];
			score[x] += EvalRook(x, sq);
		}
		b1 = bit_pieces[x][Q];
		while (b1)
		{
			sq = NextBit(b1);
			b1 &= b1 - 1;
			score[x] += square_score[x][Q][sq];
		}

		if (bit_pieces[x ^ 1][Q] == 0)
			score[x] += king_endgame[x][NextBit(bit_pieces[x][K])];
		else
		{
			score[x] += square_score[x][K][NextBit(bit_pieces[x][K])];
			if (bit_pieces[x][K] & mask_kingside)
				score[x] += kingside_pawns[x];
			else if (bit_pieces[x][K] & mask_queenside)
				score[x] += queenside_pawns[x];
		}
	}
	return score[side] - score[xside];
}

static int EvalPawn(const int s, const int x)
{
	int score = 0;

	if (!(mask_passed[s][x] & bit_pieces[s ^ 1][P]) && !(mask_path[s][x] & bit_pieces[s][P]))
	{
		score += passed[s][x];
	}
	if ((mask_isolated[x] & bit_pieces[s][P]) == 0)
		score -= ISOLATED;
	kingside_pawns[s] += kingside_defence[s][x];
	queenside_pawns[s] += queenside_defence[s][x];

	return score;
}

static int EvalRook(const int s, const int sq)
{
	int score = 0;
	if (!(mask_cols[sq] & bit_pieces[s][P]))
	{
		score = 10;
		if (!(mask_cols[sq] & bit_pieces[s ^ 1][P]))
			score = 20;
	}
	return score;
}
