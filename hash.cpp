#include "globals.h"

BITBOARD hashkey[2][6][64];
BITBOARD lock[2][6][64];

BITBOARD currentkey,currentlock;
BITBOARD collisions;

static const BITBOARD MAXHASH =  5000000;
static const BITBOARD HASHSIZE = 5000000;

move_data hash_move;

struct hashp 
{
	U64 hashlock;
	int from;
	int to;
	int flags;
};

hashp* hashpos[2];

void RandomizeHash()
{
	for (int piece = 0; piece < 6; piece++)
		for (int x = 0; x < 64; x++)
		{
			hashkey[WHITE][piece][x] = Random(HASHSIZE);
			hashkey[BLACK][piece][x] = Random(HASHSIZE);
			lock[WHITE][piece][x] = Random(HASHSIZE);
			lock[BLACK][piece][x] = Random(HASHSIZE);
		}
	hashpos[0] = new hashp[MAXHASH];
	hashpos[1] = new hashp[MAXHASH];
}

int Random(const int x)
{
	return rand() % x;
}

void Free()
{
	delete[] hashpos[0];
	delete[] hashpos[1];
}

void AddHash(const int s, const move_data m)
{
	hashp* ptr = &hashpos[s][currentkey];
	ptr->hashlock = currentlock;
	ptr->from = m.from;
	ptr->to = m.to;
	ptr->flags = m.flags;
}

void AddKey(const int s, const int piece, const int sq)
{
	currentkey ^= hashkey[s][piece][sq];
	currentlock ^= lock[s][piece][sq];
}

U64 GetLock()
{
	U64 loc = 0;
	for (int x = 0; x < 64; x++)
	{
		if (board[x] != EMPTY)
		{
			if (bit_units[WHITE] & mask[x])
				loc ^= lock[WHITE][board[x]][x];
			if (bit_units[BLACK] & mask[x])
				loc ^= lock[BLACK][board[x]][x];
		}
	}
	return loc;
}

U64 GetKey()
{
	U64 key = 0;
	for (int x = 0; x < 64; x++)
	{
		if (board[x] != EMPTY)
		{
			if (bit_units[WHITE] & mask[x])
				key ^= hashkey[WHITE][board[x]][x];
			if (bit_units[BLACK] & mask[x])
				key ^= hashkey[BLACK][board[x]][x];
		}
	}
	return key;
}

bool LookUp(const int s)
{
	if (hashpos[s][currentkey].hashlock != currentlock)
	{
		collisions++;
		return false;
	}
	hash_move.from = hashpos[s][currentkey].from;
	hash_move.to = hashpos[s][currentkey].to;
	hash_move.flags = hashpos[s][currentkey].flags;
	return true;
}

