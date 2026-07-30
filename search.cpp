#include <setjmp.h>
#include <assert.h>

#include "globals.h"

int cont[6][64][6][64];

static void SelectMove(const int from);
static void SetHashMove();
static void DisplayPV(int i);
void z();

void LoadBook();
int Book();

jmp_buf env;
bool stop_search;

static int currentmax;

move_data root_move;

//int GetDefender(const int s, const int xs, const int sq, const BITBOARD occ);
int LowestAttacker(const int s, const int xs, const int sq, const BITBOARD occ);
bool Attack2(const int s, const int sq, const BITBOARD occ);
bool IsPinned(const int s, const int sq, const int king, const BITBOARD occ);
bool IsLineAttack(const int s, const int sq, const BITBOARD occ);

void think()
{
	int bookflag = 0;

	root_move = {};

	if (hply < 8)
	{
		if (hply <= 1)
			LoadBook();
		bookflag = Book();
		if (bookflag > 0)
		{
			return;
		}
	}

	int score;

	stop_search = false;
	ply = 0;
	nodes = 0;

	NewPosition();

	memset(history, 0, sizeof(history));
	memset(cont, 0, sizeof(cont));

	start_time = GetTime();

	if (fixed_time == 0)
	{
		if (Attack(xside, NextBit(bit_pieces[side][K])))
			max_time = max_time / 2;
	}

	stop_time = start_time + max_time;

	if (setjmp(env))
	{
		stop_search = true;

		while (ply)
			UnMakeMove();

		return;
	}

	for (int i = 1; i <= max_depth; ++i)
	{
		currentmax = i;
		if (max_depth > 1)
		{
			int elapsed = GetTime() - start_time;

			if (fixed_time)
			{
				if (elapsed >= max_time)
					break;
			}
			else
			{
				if (elapsed >= max_time / 8)
				{
					break;
				}
			}
		}
		score = Search(-10000, 10000, i);

		std::cout << i << " " << score << " " << (GetTime() - start_time) / 10 << " " << nodes;
		if (LookUp(side))
		{
			DisplayPV(i);
		}
		std::cout << std::endl;

		if (score > 9000 || score < -9000)
		{
			break;
		}
	}
}

static int Search(int alpha, int beta, int depth)
{
	if (ply && Reps2())
	{
		return 0;
	}

	if (depth < 1)
		return CaptureSearch(alpha, beta);

	nodes++;

	if ((nodes & 4095) == 0 && fixed_depth == 0)
	{
		CheckUp();
	}

	if (ply > MAX_PLY - 2)
		return Eval();

	int bestscore = -10001;

	int check = 0;

	if (Attack(xside, NextBit(bit_pieces[side][K])))
	{
		check = 1;
	}
	Gen(side, xside);

	move_data bestmove = move_list[first_move[ply]];

	if (LookUp(side))
		SetHashMove();

	int count = 0;
	int score;
	int d;

	for (int i = first_move[ply]; i < first_move[ply + 1]; ++i)
	{
		SelectMove(i);

		if (!MakeMove(move_list[i]))
			continue;

		count++;

		if (Attack(xside, NextBit(bit_pieces[side][K])))
		{
			d = depth;
		}
		else
		{
			d = depth - 3;
			if (move_list[i].flags & (CAPTURE | EP) || count == 1 || check == 1)
			{
				d = depth - 1;
			}
			else if (move_list[i].score > 0)
			{
				d = depth - 2;
			}
		}

		score = -Search(-beta, -alpha, d);

		UnMakeMove();

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = move_list[i];
			if (ply == 0)
			{
				root_move = bestmove;
			}
			if (score > alpha)
			{
				if (score >= beta)
				{
					int from = move_list[i].from;
					int to = move_list[i].to;
					{
						if (!(mask[to] & bit_all))
							history[from][to] += depth;
						if (hply > 2)
						{
							int to2 = game_list[hply - 1].to;
							int cont_score = cont[board[to2]][to2][board[from]][to];
							if (cont_score >= 16000)
								cont[board[to2]][to2][board[from]][to] /= 2;
							else
								cont[board[to2]][to2][board[from]][to] += depth * depth;
						}
					}
					AddHash(side, move_list[i]);
					return beta;
				}
				alpha = score;
			}
		}
	}
	if (count == 0)
	{
		if (Attack(xside, NextBit(bit_pieces[side][K])))
		{
			return -10000 + ply;
		}
		else
			return 0;
	}
	if (fifty >= 100)
		return 0;
	AddHash(side, bestmove);
	return alpha;
}

static int CaptureSearch(int alpha, int beta)
{
	nodes++;

	int score = Eval();

	if (score > alpha)
	{
		if (score >= beta)
		{
			return beta;
		}
		alpha = score;
	}
	else if (score + piece_value[Q] < alpha)
		return alpha;

	int recapture_score = 0, bestmove = 0;
	int best = 0;
	const int king = NextBit(bit_pieces[side][K]);

	GenCaptures(side, xside);

	for (int i = first_move[ply]; i < first_move[ply + 1]; ++i)
	{
		SelectMove(i);
		int to = move_list[i].to;
		if (score + piece_value[board[to]] < alpha)
		{
			continue;
		}
		int from = move_list[i].from;
		if (IsPinned(xside, from, king, bit_all & ~mask[to]))
		{
			continue;
		}

		recapture_score = ReCaptureSearch(side, xside, from, to);

		if (recapture_score > best)
		{
			best = recapture_score;
			bestmove = i;
		}
	}
	if (best > 0)
	{
		score += best;
	}
	if (score > alpha)
	{
		if (score >= beta)
		{
			if (best > 0)
				AddHash(side, move_list[bestmove]);
			return beta;
		}
		return score;
	}
	return alpha;
}

static int ReCaptureSearch(int s, int xs, const int attacker, const int sq)
{
	int next = attacker;
	int count = 0;
	int score[14];

	BITBOARD occ = bit_all;

	score[0] = piece_value[board[sq]];
	score[1] = piece_value[board[attacker]];

	int total_score = 0;

	while (count < 12)
	{
		occ &= ~mask[next];
		s ^= 1;
		xs ^= 1;

		if (IsLineAttack(s, NextBit(bit_pieces[xs][K]), occ))
		{
			count++;
			break;
		}
		count++;

		next = LowestAttacker(s, xs, sq, occ);

		if (next > -1)
		{
			score[count + 1] = piece_value[board[next]];
			if (score[count] > score[count - 1] + score[count + 1])
			{
				count--;
				break;
			}
		}
		else
		{
			break;
		}
	}
	while (count > 1)
	{
		if (score[count - 1] >= score[count - 2])
			count -= 2;
		else
			break;
	}
	for (int x = 0; x < count; x++)
	{
		if (x % 2 == 0)
			total_score += score[x];
		else
			total_score -= score[x];
	}
	return total_score;
}

bool Attack2(const int s, const int sq, const BITBOARD occ)
{
	if (bit_pawncaptures[s ^ 1][sq] & bit_pieces[s][P] & occ)
		return true;
	if (bit_moves[N][sq] & bit_pieces[s][N] & occ)
		return true;

	BITBOARD b1 = bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]) & occ;
	b1 |= (bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q])) & occ;
	while (b1)
	{
		int from = NextBit(b1);
		if (!(bit_between[from][sq] & bit_all))
			return true;
		b1 &= b1 - 1;
	}
	if (bit_moves[K][sq] & bit_pieces[s][K])
		return true;
	return false;
}

static int Reps2()
{
	for (int i = hply - 4; i >= 0 && i >= hply - fifty; i -= 2)
	{
		if (game_list[i].hashkey == currentkey && game_list[i].lock == currentlock)
			return 1;
	}
	return 0;
}

static void SelectMove(const int from)
{
	int best_score = move_list[from].score;
	int best_move = from;
	for (int i = from + 1; i < first_move[ply + 1]; i++)
		if (move_list[i].score > best_score)
		{
			best_score = move_list[i].score;
			best_move = i;
		}

	move_data g = move_list[from];
	move_list[from] = move_list[best_move];
	move_list[best_move] = g;
}

static void CheckUp()
{
	if (ply <= 1) return;

	if (GetTime() >= stop_time || max_time < 50)
	{
		stop_search = true;
		longjmp(env, 1);
	}
}

static void SetHashMove()
{
	for (int x = first_move[ply]; x < first_move[ply + 1]; x++)
	{
		if (move_list[x].from == hash_move.from && move_list[x].to == hash_move.to)
		{
			move_list[x].score = HASH_SCORE;
			return;
		}
	}
}

static void DisplayPV(int i)
{
	for (int x = 0; x < i; x++)
	{
		if (LookUp(side) == false)
			break;
		std::cout << " ";
		Alg(hash_move.from, hash_move.to);
		MakeMove(hash_move);
	}
	while (ply)
		UnMakeMove();
}