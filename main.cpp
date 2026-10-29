#include "stdafx.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/timeb.h>

#include "globals.h"//29/10/26

extern move_data root_move;

int GetChecker(const int s, const int sq);
void Evasion(const int s, const int xs, const int checker);
void GenCheck(const int s, const int xs);

void ShowHelp();
void SetUp();
void xboard();
void SetCaptureTable();

int ParseMove(string s);
string MoveString(int from, int to, int promote);

static char piece_char[2][6] =
{
  {'P', 'N', 'B', 'R', 'Q', 'K'},
  {'p', 'n', 'b', 'r', 'q', 'k'}
};

static int board_color[64] =
{
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1
};

int LoadDiagram(string file);

char fen_name[256];

int flip = 0;

int computer_side;

int fixed_time;
int fixed_depth = 1;
int max_time;
int start_time;
int stop_time;
int max_depth;
int turn = 0;

static void PostResult();

void NewGame();
void SetMaterial();
void SetBits();
void SetKingDefences();

int main()
{
	cout << "Bills 64-Bit Bitboard Chess Engine" << endl;
	cout << "Version 2.1, 29/10/26" << endl;
	cout << "Bill Jordan 2026" << endl;
	cout << "FIDE Master and multiple state champion." << endl;
	cout << "I have published a number of chess books" << endl;
	cout << "including books on chess programming." << endl;
	cout << "My author page at Amazon is at billjordanchess" << endl;
	cout << "https://www.amazon.com/-/e/B07F5WSFHZ" << endl;
	cout << endl;
	cout << "\"help\" displays a list of commands." << endl;
	cout << endl;

	string s;
	string sFen;
	string sText;

	int m;
	int turns = 0;
	int t;

	double nps;

	fixed_time = 0;

	SetBits();
	SetKingDefences();
	SetCaptureTable();
	SetUp();

	cout << "Engine loaded." << endl;

	side = 0;//

	while (true)
	{
		if (side == computer_side)
		{
			think();
			turns++;

			currentkey = GetKey();
			currentlock = GetLock();

			cout << endl << "Computer's move: ";
			Alg(root_move.from, root_move.to); cout << endl;

			cout << "Collisions " << collisions << endl;
			collisions = 0;

			MakeMove(root_move);

			SetMaterial();

			t = GetTime() - start_time;
			cout << "Time: " << t;
			if (t > 0)
				nps = (double)nodes / (double)t;
			else
				nps = 0;
			nps *= 1000.0;

			cout << endl << "Nodes per second: " << (int)nps << endl;
			ply = 0;

			first_move[0] = 0;
			PostResult();
			cout << "Turn " << ++turn << endl;
			DisplayBoard();
			continue;
		}
		cout << "Enter move or command> ";
		cin >> s;

		if (s == "d")
		{
			DisplayBoard();
			continue;
		}
		if (s == "f")
		{
			flip = 1 - flip;
			DisplayBoard();
			continue;
		}
		if (s == "go")
		{
			computer_side = side;
			continue;
		}
		if (s == "help")
		{
			ShowHelp();
			continue;
		}
		if (s == "moves")
		{
			cout << "Moves" << endl;
			for (int i = 0; i < first_move[1]; ++i)
			{
				Alg(move_list[i].from, move_list[i].to);
			}
			continue;
		}
		if (s == "new")
		{
			NewGame();
			computer_side = EMPTY;
			continue;
		}
		if (s == "on" || s == "p")
		{
			computer_side = side;
			continue;
		}
		if (s == "off")
		{
			computer_side = EMPTY;
			continue;
		}
		if (s == "quit")
		{
			cout << "Program exiting" << endl;
			break;
		}
		if (s == "sb")
		{
			sFen = "c:\\users\\bill\\desktop\\bscp\\";
			cin >> sText;
			sFen += sText + ".fen";
			LoadDiagram(sFen);
			continue;
			//
			sFen = "c:\\diagrams\\";//You will need to replace this with the folder you use for your diagrams. 
			cin >> sText;
			sFen += sText + ".fen";
			LoadDiagram(sFen);
			continue;
		}
		if (s == "sd")
		{
			cin >> max_depth;
			max_time = 1 << 25;
			fixed_depth = 1;
			fixed_time = 0;
			continue;
		}
		if (s == "st")
		{
			cin >> max_time;
			max_time *= 1000;
			max_depth = MAX_PLY;
			fixed_time = 1;
			fixed_depth = 0;
			continue;
		}
		if (s == "sw")
		{
			side = 1 - side;
			xside = 1 - xside;
			continue;
		}
		if (s == "undo")
		{
			if (!hply)
				continue;
			computer_side = EMPTY;
			UnMakeMove();
			ply = 0;
			if (first_move[0] != 0)
				first_move[0] = 0;
			continue;
		}
		if (s == "xboard")
		{
			xboard();
			break;
		}

		ply = 0;
		first_move[0] = 0;
		GenCheck(side, xside);
		m = ParseMove(s);
		if (m == -1 || !MakeMove(move_list[m]))
		{
			cout << "Illegal move." << endl;
			cout << s << endl;
			MoveString(move_list[m].from, move_list[m].to, 0);
			if (m == -1)
				cout << " m = -1" << endl;
		}
		if (game_list[hply].flags & PROMOTE && (row[move_list[m].to] == 0 || row[move_list[m].to] == 7))
		{
			RemovePiece(xside, Q, move_list[m].to);
			if (s[4] == 'n' || s[4] == 'N')
				AddPiece(xside, N, move_list[m].to);
			else if (s[4] == 'b' || s[4] == 'B')
				AddPiece(xside, B, move_list[m].to);
			else if (s[4] == 'r' || s[4] == 'R')
				AddPiece(xside, R, move_list[m].to);
			else AddPiece(xside, Q, move_list[m].to);
		}
	}
	Free();
	return 0;
}

static int ParseMove(string s)
{
	int from, to, i;

	if (s[0] < 'a' || s[0] > 'h' ||
		s[1] < '1' || s[1] > '8' ||
		s[2] < 'a' || s[2] > 'h' ||
		s[3] < '1' || s[3] > '8')
		return -1;

	from = s[0] - 'a';
	from += ((s[1] - '0') - 1) * 8;
	to = s[2] - 'a';
	to += ((s[3] - '0') - 1) * 8;

	for (i = 0; i < first_move[1]; ++i)
		if (move_list[i].from == from && move_list[i].to == to)
		{
			if (s.length() > 4)
			{
				if (s[4] == 'n' || s[4] == 'N')
					move_list[i].flags |= PROMOTE_KNIGHT;
				if (s[4] == 'b' || s[4] == 'B')
					move_list[i].flags |= PROMOTE_BISHOP;
				else if (s[4] == 'r' || s[4] == 'R')
					move_list[i].flags |= PROMOTE_ROOK;
			}
			return i;
		}
	return -1;
}

void DisplayBoard()
{
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int text = 15;

	int i;
	int x = 0;
	int c;

	if (flip == 0)
		cout << "\n8";
	else
		cout << "\n1";

	for (int j = 0; j < 64; ++j)
	{
		if (flip == 0)
			i = Flip[j];
		else
			i = 63 - Flip[j];
		c = 6;
		if (bit_units[WHITE] & mask[i]) c = 0;
		if (bit_units[BLACK] & mask[i]) c = 1;
		switch (c)
		{
		case EMPTY:
			if (board_color[i] == 0)
				text = 127;
			else
				text = 34;
			SetConsoleTextAttribute(hConsole, text);

			cout << "  ";
			SetConsoleTextAttribute(hConsole, 15);
			break;
		case 0:
			if (board_color[i] == 0)
				text = 126;
			else
				text = 46;
			SetConsoleTextAttribute(hConsole, text);
			cout << " " << piece_char[c][board[i]];
			SetConsoleTextAttribute(hConsole, 15);
			break;

		case 1:
			if (board_color[i] == 0)
				text = 112;
			else
				text = 32;
			SetConsoleTextAttribute(hConsole, text);
			cout << " " << piece_char[c][board[i]];
			SetConsoleTextAttribute(hConsole, 15);
			break;

		default:
			cout << " ." << c;
			break;

		}
		if ((bit_all & mask[i]) && board[i] == EMPTY)
			if (x == 0)
				cout << " " << c;
			else
				cout << " ";
		if (board[i] < 0 || board[i] > 6)
			if (x == 0)
				cout << " ." << board[i];
			else
				cout << "  " << board[i];
		if (flip == 0)
		{
			if ((j + 1) % 8 == 0 && j != 63)
				cout << endl << row[i];
		}
		else
		{
			if ((j + 1) % 8 == 0 && row[i] != 7)
				cout << endl << row[j] + 2;
		}
	}
	if (flip == 0)
		cout << "\n\n   a b c d e f g h\n\n";
	else
		cout << "\n\n   h g f e d c b a\n\n";
}

static void xboard()
{
	int computer_side;
	int m;
	int analyze = 0;
	int lookup;
	int post = 1;

	string s;
	string sFen;
	string sText;

	signal(SIGINT, SIG_IGN);
	cout << endl;
	NewGame();
	fixed_time = 0;
	computer_side = EMPTY;

	while (true)
	{
		fflush(stdout);
		if (side == computer_side)
		{
			think();
			SetMaterial();
			currentkey = GetKey();
			currentlock = GetLock();

			move_list[0].from = root_move.from;
			move_list[0].to = root_move.to;
			cout << "move " << MoveString(root_move.from, root_move.to, 0) << endl;
			cout.flush();

			MakeMove(root_move);

			ply = 0;
			PostResult();
			continue;
		}
		cin >> s;
		if (s == "xboard")
			continue;
		if (s == "new")
		{
			NewGame();
			computer_side = 1;
			continue;
		}
		if (s == "quit")
			return;
		if (s == "force")
		{
			computer_side = EMPTY;
			continue;
		}
		if (s == "white")
		{
			side = 0;
			xside = 1;
			computer_side = 1;
			continue;
		}
		if (s == "black")
		{
			side = 1;
			xside = 0;
			computer_side = 0;
			continue;
		}
		if (s == "st")
		{
			cin >> max_time;
			max_time *= 1000;
			max_depth = MAX_PLY;
			fixed_time = 1;
			fixed_depth = 0;
			continue;
		}
		if (s == "sd")
		{
			cin >> max_depth;
			max_time = 1 << 25;
			fixed_time = 0;
			fixed_depth = 1;
			continue;
		}
		if (s == "time")
		{
			cin >> max_time;
			if (max_time < 200)
				max_depth = 1;
			else
			{
				max_time /= 2;
				max_depth = MAX_PLY;
			}
			continue;
		}
		if (s == "protover")
		{
			int version;
			cin >> version;
			cout << "feature done=1" << endl;
			continue;
		}
		if (s == "computer")
		{
			continue;
		}
		if (s == "otim")
		{
			int dummy;
			cin >> dummy;
			continue;
		}
		if (s == "go")
		{
			computer_side = side;
			continue;
		}
		if (s == "random")
			continue;
		if (s == "level")
		{
			int moves, inc;
			string minutes;

			cin >> moves >> minutes >> inc;
			continue;
		}
		if (s == "result")
		{
			string rest;
			getline(cin, rest);   // consumes: 0-1 {Arena Adjudication}
			computer_side = EMPTY;
			continue;
		}
		if (s == "?")
		{
			computer_side = EMPTY;
			continue;
		}
		if (s == "accepted")
		{
			string dummy;
			cin >> dummy;
			continue;
		}
		if (s == "rejected")
		{
			string dummy;
			cin >> dummy;
			continue;
		}
		if (s == "post")
		{
			post = 1;
			continue;
		}
		if (s == "nopost")
		{
			post = 0;
			continue;
		}
		if (s == "setboard")
		{
			string fen;
			getline(cin, fen);
			fen = "c:\\users\\bill\\desktop\\bscp\\" + fen;

			if (fen.length() > 0 && fen[0] == ' ')
				fen.erase(0, 1);

			LoadDiagram(fen);

			ply = 0;
			computer_side = EMPTY;
			continue;
		}
		if (s == "hard")
			continue;
		if (s == "easy")
			continue;
		if (s == "hint")
		{
			think();
			currentkey = GetKey();
			currentlock = GetLock();
			lookup = LookUp(side);
			if (hash_move.from == 0 && hash_move.to == 0)
				continue;
			cout << "Hint: " << MoveString(root_move.from, root_move.to, 0) << endl;
			cout.flush();
			continue;
		}
		if (s == "undo")
		{
			if (!hply)
				continue;
			UnMakeMove();
			ply = 0;
			continue;
		}
		if (s == "remove")
		{
			if (hply < 2)
				continue;
			UnMakeMove();
			UnMakeMove();
			ply = 0;
			continue;
		}

		first_move[0] = 0;
		GenCheck(side, xside);

		m = ParseMove(s);
		if (m == -1 || !MakeMove(move_list[m]))
		{
			DisplayBoard();
			cout << "Error (unknown command): " << s << endl;
		}
		else
		{
			ply = 0;
			PostResult();
		}
	}
}

void GenCheck(const int s, const int xs)
{
	const int king = NextBit(bit_pieces[side][K]);
	int checker = GetChecker(xside, king);

	if (checker == -1)
		Gen(side, xside);
	else
		Evasion(side, xside, checker);
}

static void PostResult()
{
	int i;
	int flag = 0;

	SetMaterial();
	GenCheck(side, xside);
	for (i = 0; i < first_move[1]; ++i)
		if (MakeMove(move_list[i]))
		{
			UnMakeMove();
			flag = 1;
			break;
		}

	if (pawn_mat[WHITE] == 0 && pawn_mat[BLACK] == 0 &&
		piece_mat[WHITE] <= piece_value[N] && piece_mat[BLACK] <= piece_value[N])
	{
		cout << "1/2-1/2 {Stalemate}" << endl;
		computer_side = EMPTY;
		return;
	}
	if (i == first_move[1] && flag == 0)
	{
		Gen(side, xside);
		DisplayBoard();
		cout << " end of game ";
		if (Attack(xside, NextBit(bit_pieces[side][K])))
		{
			if (side == 0)
			{
				cout << "0-1 {BLACK mates}" << endl;
				cout << "0-1 {BLACK mates}" << endl;
			}
			else
			{
				cout << "1-0 {WHITE mates}" << endl;
			}
		}
		else
		{
			cout << "1/2-1/2 {Stalemate}" << endl;
		}
		computer_side = EMPTY;
	}
	else if (Reps() >= 3)
	{
		cout << "1/2-1/2 {Draw by repetition}" << endl;
		computer_side = EMPTY;
	}
	else if (fifty >= 100)
	{
		cout << "1/2-1/2 {Draw by fifty move rule}" << endl;
		computer_side = EMPTY;
	}
}

static int Reps()
{
	int r = 0;

	for (int i = hply; i >= hply - fifty; i -= 2)
		if (game_list[i].hashkey == currentkey && game_list[i].lock == currentlock)
			r++;
	return r;
}

static int LoadDiagram(string file_name)
{
	ifstream file(file_name);
	if (!file) {
		cerr << "File not found!" << endl;
		return 1;
	}

	string fen_text;

	getline(file, fen_text);

	int x, n = 0;

	int c = 0, i = 0, j;

	memset(pawn_mat, 0, sizeof(pawn_mat));
	memset(piece_mat, 0, sizeof(piece_mat));
	memset(bit_pieces, 0, sizeof(bit_pieces));
	memset(bit_units, 0, sizeof(bit_units));
	bit_all = 0;

	for (x = 0; x < 64; x++)
	{
		board[x] = EMPTY;
	}

	while (fen_text[c])
	{
		if (fen_text[c] >= '0' && fen_text[c] <= '8')
			i += fen_text[c] - 48;
		if (fen_text[c] == '\\')
			continue;
		j = Flip[i];

		switch (fen_text[c])
		{
		case 'K': AddPiece(0, K, j); i++; break;
		case 'Q': AddPiece(0, Q, j); i++; break;
		case 'R': AddPiece(0, R, j); i++; break;
		case 'B': AddPiece(0, B, j); i++; break;
		case 'N': AddPiece(0, N, j); i++; break;
		case 'P': AddPiece(0, P, j); i++; break;
		case 'k': AddPiece(1, K, j); i++; break;
		case 'q': AddPiece(1, Q, j); i++; break;
		case 'r': AddPiece(1, R, j); i++; break;
		case 'b': AddPiece(1, B, j); i++; break;
		case 'n': AddPiece(1, N, j); i++; break;
		case 'p': AddPiece(1, P, j); i++; break;
		}
		c++;
		if (fen_text[c] == ' ')
			break;
		if (i > 63)
			break;
	}
	if (fen_text[c] == ' ' && fen_text[c + 2] == ' ')
	{
		if (fen_text[c + 1] == 'w')
		{
			side = 0; xside = 1;
		}
		if (fen_text[c + 1] == 'b')
		{
			side = 1; xside = 0;
		}
	}

	castle = 0;
	while (fen_text[c])
	{
		switch (fen_text[c])
		{
		case '-': break;
		case 'K':if (bit_pieces[WHITE][K] & mask[E1]) castle |= CASTLE_G1;
			break;
		case 'Q':if (bit_pieces[WHITE][K] & mask[E1]) castle |= CASTLE_C1;
			break;
		case 'k':if (bit_pieces[BLACK][K] & mask[E8]) castle |= CASTLE_G8;
			break;
		case 'q':if (bit_pieces[BLACK][K] & mask[E8]) castle |= CASTLE_C8;
			break;
		default:break;
		}
		c++;
	}

	NewPosition();
	DisplayBoard();
	hply = 8;//

	Gen(side, xside);

	if (side == 0)
		cout << "WHITE to move" << endl;
	else
		cout << "BLACK to move" << endl;
	cout << fen_text << endl;
	return 0;
}

static void ShowHelp()
{
	cout << "d - Displays the board." << endl;
	cout << "f - Flips the board." << endl;
	cout << "go - Starts the engine." << endl;
	cout << "help - Displays help on the commands." << endl;
	cout << "moves - Displays of list of possible moves." << endl;
	cout << "new - Starts a new game." << endl;
	cout << "off - Turns the computer player off." << endl;
	cout << "on or p - The computer plays a move." << endl;
	cout << "sb - Loads a fen diagram." << endl;
	cout << "sd - Sets the search depth." << endl;
	cout << "st - Sets the time limit per move in seconds." << endl;
	cout << "sw - Switches sides." << endl;
	cout << "quit - Quits the program." << endl;
	cout << "undo - Takes back the last move." << endl;
	cout << "xboard - Starts xboard." << endl;
}

void SetUp()
{
	RandomizeHash();
	SetTables();
	InitBoard();
	computer_side = EMPTY;
	max_time = 1 << 25;
	max_depth = 8;
}

void NewGame()
{
	InitBoard();
	Gen(side, xside);
}

void SetMaterial()
{
	int color;
	memset(pawn_mat, 0, sizeof(pawn_mat));
	memset(piece_mat, 0, sizeof(piece_mat));

	for (int x = 0; x < 64; x++)
	{
		if (board[x] != EMPTY)
		{
			if (bit_units[WHITE] & mask[x])
				color = 0;
			else
				color = 1;
			if (board[x] == P)
				pawn_mat[color] += piece_value[P];
			else
				piece_mat[color] += piece_value[board[x]];
		}
	}
}

int GetTime()
{
	struct timeb timebuffer;
	ftime(&timebuffer);
	return (timebuffer.time * 1000) + timebuffer.millitm;
}

static string MoveString(int from, int to, int promote)
{
	string str = "";

	str += col[from] + 'a';
	str += row[from] + '1';
	str += col[to] + 'a';
	str += row[to] + '1';

	string promote_piece[] = { "-","n","b","r","q","q" };

	if (promote > 0)
	{
		str += promote_piece[promote];
	}
	return str;
}