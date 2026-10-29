#include <setjmp.h>
#include <assert.h>

#include "globals.h"//29/10/26

move_data refute[MAX_PLY];
static void SetRefuteMove(const int from, const int to);
const int REFUTE_SCORE = 500000;
int frontier[8] = { 0,0,8,20,10,20,0 };

int GetBounds();

int cont[6][64][6][64];

static void SelectMove(const int from);
static void SetHashMove(const int from, const int to);
static void DisplayPV(int i);

void LoadBook();
int Book();
int GetHashDepth();
int GetChecker(const int s, const int sq);
void Evasion(const int s, const int xs, const int checker);

jmp_buf env;
bool stop_search;

static int currentmax;

move_data root_move;

int LowestAttacker(const int s, const int xs, const int sq, const BITBOARD occ);
bool Attack3(const int s, const int sq, const BITBOARD occ);
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

        cout << i << " " << score << " " << (GetTime() - start_time) / 10 << " " << nodes;
        if (LookUp(side))
        {
            DisplayPV(i);
        }
        cout << endl;

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

    const int king = NextBit(bit_pieces[side][K]);
    int checker = GetChecker(xside, king);

    if (checker == -1)
        Gen(side, xside);
    else
        Evasion(side, xside, checker);

    move_data bestmove = move_list[first_move[ply]];

    refute[ply + 1].from = 0;
    refute[ply + 1].to = 0;

    if (LookUp(side))
        SetHashMove(hash_move.from, hash_move.to);
    if (refute[ply].from != 0 || refute[ply].to != 0)
        SetRefuteMove(refute[ply].from, refute[ply].to);

    int count = 0;
    int score;
    int d;
    int ev1 = Eval();
    const int xking = NextBit(bit_pieces[xside][K]);

    for (int i = first_move[ply]; i < first_move[ply + 1]; i++)
    {
        SelectMove(i);
        int from = move_list[i].from;
        int to = move_list[i].to;
        int piece = board[from];
        unsigned int flags = move_list[i].flags;

        if (depth == 1 && count > 0 && !(flags & (CAPTURE | EP | PROMOTE)))
        {
            int ev = ev1 + square_score[side][piece][to] - square_score[side][piece][from] + frontier[piece];
            if (ev <= alpha)
            {
                continue;
            }
        }

        if (!MakeMove(move_list[i]))
            continue;
      
        count++;

        d = depth - 1;
        if (Attack(xside, xking))
        {
            d = depth;
        }
        else if(depth > 1)
        {
            d = depth - 3;
            if (flags & (CAPTURE | EP) || count == 1 || checker == 1)// || move_list[i].score==REFUTE_SCORE)
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
                if (score >= beta || score > 9900)
                {
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
                    if (!(flags & CAPTURE) && move_list[i].score != HASH_SCORE)
                        refute[ply] = move_list[i];
                    AddHash(side, move_list[i], depth);
                    return beta;
                }
                alpha = score;
            }
            
            if (move_list[i].score == HASH_SCORE)
            {
                if (depth == 1)
                    break;
                if (GetHashDepth() == depth)
                {
                    break;
                }
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
    AddHash(side, bestmove, depth);
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
                AddHash(side, move_list[bestmove], 0);
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

static void SetHashMove(const int from, const int to)
{
    for (int x = first_move[ply]; x < first_move[ply + 1]; x++)
    {
        if (move_list[x].from == from && move_list[x].to == to)
        {
            move_list[x].score = HASH_SCORE;
            return;
        }
    }
}

static void SetRefuteMove(const int from, const int to)
{
    for (int x = first_move[ply]; x < first_move[ply + 1]; x++)
    {
        if (move_list[x].from == from && move_list[x].to == to)
        {
            move_list[x].score = REFUTE_SCORE;
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
        cout << " ";
        Alg(hash_move.from, hash_move.to);
        MakeMove(hash_move);
    }
    while (ply)
        UnMakeMove();
}