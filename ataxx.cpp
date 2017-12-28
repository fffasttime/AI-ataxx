#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>
#include <sstream>
#include <fstream>
#include <queue>
#include "jsoncpp/json.h" // C++编译时默认包含此库
#include <algorithm>

//#define WIN_CON

#define MAX_TIME 0.88 
#define CTRL_TIME 0.30 

using namespace std;

#define M_SIZE (7)
#ifdef WIN_CON
#define ASSERT(EXPR) assert(EXPR)
#else
#define ASSERT(EXPR)
#endif
const int PIECE_WHITE = 1, PIECE_BLACK = 2, PIECE_EMPTY = 0;

stringstream debug_s;

#define LOG_PRINT
stringstream fout;
#ifdef LOG_PRINT
ofstream filelog("ataxx.log");
#endif
void logRefrsh()
{

	string s;
#ifdef LOG_PRINT
	while (!fout.eof())
	{
		getline(fout, s);
		filelog << s << '\n';
	}
	fout.clear();
#endif
}

bool inBorder(int x, int y)
{
	return (x >= 0 && x< M_SIZE && y >= 0 && y< M_SIZE);
}

#ifdef WIN_CON
#include <windows.h>
#endif

int othercol(int col)
{
	if (col == 2) return 1;
	if (col == 1) return 2;
	ASSERT(col != 0);
	return 0;
}

struct Move
{
	int to;
	int fr;
	bool jump;
	void print()
	{
		int x = to / 8, y = to % 8, fx = fr / 8, fy = fr % 8;
		fout << '<' << x << ',' << y;
		if (jump) fout << ',' << fx << ',' << fy;
		fout << '>';
	}
};

typedef unsigned long long ull;

string ullToString(ull x)
{
	string s; char cc[2] = { 0,0 };
	for (int i = 0;i<64;i++)
	{
		cc[0] = ((x & (1ull << i)) != 0) + '0';
		s += cc;
		if (i % 8 == 7) s += "\n";
	}
	return s;
}

inline int xyToP(int x, int y)
{
	return (x << 3) + y;
}

#define USE_BUILTIN 

inline int first0(ull x)
{
#ifdef USE_BUILTIN
	return __builtin_ctzll(x);
#else
	for (int i = 0;i < 64;i++)
		if ((x >> i) & 1ull)
			return i;
	return 64;
#endif // USE_BUINTIN
}

inline int popcount(ull x)
{
#ifdef USE_BUILTIN
	return __builtin_popcountll(x);
#else
	int c = 0;
	for (int i = 0;i < 64;i++)
		if ((x >> i) & 1ull)
			c++;
	return c;
#endif // USE_BUINTIN
}

//bitboard masks 
//dis1, dis2, only dis2, available area
ull mask1[64], mask2[64], maskj[64], maska, maskedge, maskcor;
void initMask()
{
	memset(mask1, 0, sizeof(mask1));
	memset(mask2, 0, sizeof(mask2));
	maska =maskedge=maskcor= 0;
	maskcor |= 1ull; maskcor|=1ull<<6; maskcor|=1ull<<48; maskcor|=1ull<<54;
	for (int i = 0; i < 7;i++)
		for (int j = 0;j < 7;j++)
		{
			int v = xyToP(i, j);
			maska |= 1ull << v;
			if (i == 0 || j == 0 || i == 6 || j == 6) maskedge |= 1ull << v;
			for (int k = -1;k <= 1;k++)
				for (int l = -1;l <= 1;l++)
					if (inBorder(i + k, j + l) && !(k == 0 && l == 0))
						mask1[v] |= 1ull << xyToP(i + k, j + l);
			for (int k = -2;k <= 2;k++)
				for (int l = -2;l <= 2;l++)
					if (inBorder(i + k, j + l) && !(k == 0 && l == 0))
					{
						mask2[v] |= 1ull << xyToP(i + k, j + l);
						if (!(mask1[v] & (1ull << xyToP(i + k, j + l))))
							maskj[v] |= 1ull << xyToP(i + k, j + l);
					}
		}
}

//gameboard
struct Board
{
	ull m[2];
	Board() {
		m[0] = m[1] = 0;
	}
	void clear() {
		m[0] = m[1] = 0;
	}
	int getCol(int p) const {
		return ((m[0]>>p) & 1ull) + ((m[1]>>p) & 1ull) * 2;
	}
	int getColF(int p) const {
		return ((m[0]>>p) & 1ull)*2 + ((m[1]>>p) & 1ull);
	}
	int getCol(int x, int y) const {
		return getCol(xyToP(x, y));
	}
	void setCol(int p, int col) {
		m[col - 1] |= 1ull << p;
	}
	void setCol(int x, int y, int col) {
		setCol(xyToP(x, y), col);
	}
	void set(int _m[7][7])
	{
		for (int i = 0;i<7;i++)
			for (int j = 0;j<7;j++)
				setCol(i, j, _m[i][j]);
	}
	void setPiece(int p, int col)
	{
		int cc = col - 1;
		ull flipd = m[cc ^ 1] & mask1[p];
		m[cc] |= 1ull << p;
		m[cc] |= flipd;
		m[cc ^ 1] ^= flipd;
	}
	int countNearSpaces(int p) const
	{
		ull nul = (~(m[0] | m[1])); //maska not used
		return popcount(mask1[p] & nul);
	}
	int countFlips(int p, int col) const
	{
		return popcount(m[(col - 1)^1] & mask1[p]);
	}
	void cntPiece(int cnt[3])
	{
		cnt[1] = popcount(m[0]);
		cnt[2] = popcount(m[1]);
		cnt[0] = 49 - cnt[1] - cnt[2];
	}
	int cntPieceDelta()
	{
		return popcount(m[0])-popcount(m[1]);
	}
	void cntPieceEnd(int cnt[3], int col)
	{
		int cc = col - 1;
		cnt[col % 2 + 1] = popcount(m[cc ^ 1]);
		cnt[col] = 49 - cnt[col % 2 + 1];
		cnt[0] = 0;
	}
	void setPiece(int p, int fr, int col)
	{
		int cc = col - 1;
		m[cc] ^= 1ull << fr;
		ull flipd = m[cc ^ 1] & mask1[p];
		m[cc] |= 1ull << p;
		m[cc] |= flipd;
		m[cc ^ 1] ^= flipd;
	}
	void setPiece(Move &m, int col)
	{
		if (!m.jump) setPiece(m.to, col);
		else setPiece(m.to, m.fr, col);
	}
	ull createEmpty() const {
		return (~(m[0] | m[1])) & maska;
	}
	ull createJump(int p, int col) const {
		return m[col - 1] & maskj[p];
	}
	//player col can copy on p
	bool canCopy(int p, int col) const {
		return mask1[p] & m[col - 1];
	}
	//player col can play on p
	bool canMove(int p, int col) const {
		return mask2[p] & m[col - 1];
	}
	bool hasMove(int col) const
	{
		int to;
		for (ull empty = createEmpty(); empty; empty ^= 1ull << to)
		{
			to = first0(empty);
			if (mask2[to] & m[col - 1]) return true;
		}
		return false;
	}
	bool isEmpty(int p) const
	{
		return !((m[0] | m[1]) & (1ull << p));
	}
	bool isEmpty(int x, int y) const
	{
		return isEmpty(xyToP(x, y));
	}
	void print() const
	{
		for (int i = 0;i < 7;i++)
		{
			for (int j = 0; j < 7; j++)
				cout << getCol(xyToP(i, j)) << ' ';
			cout << '\n';
		}
	}
	void toArr(int arr[7][7])
	{
		for (int i=0;i<55;i++)
		{
			if (i%8==7) i++;
			arr[i/8][i%8]=getCol(i);
		}
	}
};

//rx beta
namespace AI
{
typedef double Val;
int maxdeep;
int mcol;

int cnt_start[3];

Val eval(Board &board, int col)
{
	int cnt[3];
	board.cntPiece(cnt);
	if (cnt[0] == 0)
	{
		if (cnt[col] - cnt[othercol(col)] >= 0)
			return cnt[col] - cnt[othercol(col)] + 32;
		else
			return cnt[col] - cnt[othercol(col)] - 32;
	}
	return cnt[col] - cnt[othercol(col)];
}
Val evalEnd(Board &board, int col)
{
	int cnt[3];
	board.cntPieceEnd(cnt, othercol(col));
	if (cnt[col] - cnt[othercol(col)]>0)
		return cnt[col] - cnt[othercol(col)] + 32;
	return cnt[col] - cnt[othercol(col)] - 32;
}

const int TX[8] = { -1,-1,-1,0,0,1,1,1 };
const int TY[8] = { -1,0,1,-1,1,-1,0,1 };
const int JX[16] = { -2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2 };
const int JY[16] = { -2,-1,0,1,2,-2,2,-2,2,-2,2,-2,-1,0,1,2 };

int leaf_count;
int stat, statc;

int t0;

#define nan (0.0/0.0)
//jumping evalution, slightly improved win rate
Val evalJump[9][9] = {
	{ -1.16713,-0.86424,-0.13137,-0.22867,1.1016,0.88142,1.0034,1.78831,1.53666 },
	{ -0.338552,0.433612,0.02108,0.854364,0.802064,0.496055,2.05764,1.76271,nan },
	{ 0.288602,-1.54628,0.478222,-0.00937796,0.783388,1.87763,1.83781,nan,nan },
	{ -0.0103483,-0.556221,0.827456,0.744095,1.93756,2.46837,nan,nan,nan },
	{ -0.867383,-1.02618,0.638031,1.61424,2.91738,nan,nan,nan,nan },
	{ -0.0625038,-0.0291924,1.39733,3.28307,nan,nan,nan,nan,nan },
	{ 1.34444,2.46231,2.25722,nan,nan,nan,nan,nan,nan },
	{ 2.1,-0.973076,nan,nan,nan,nan,nan,nan,nan },
	{ nan,nan,nan,nan,nan,nan,nan,nan,nan } };

bool outtime;

//provide next branch
//considering to make it faster
template <bool cut> void geneBranch(Board &board, int col, vector<Move> &v)
{
	int cnt[3];
	board.cntPiece(cnt);
	vector<Move> tv[30];
	int colf = othercol(col);
	int ecnt[3][64];
	int maxk = 0, max2k = 0, maxe = 0, max2e = 0;
	statc++;
	int to;
	for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
	{
		to = first0(empty);
		//if (board.canMove(to, col))   useless
		{
			ecnt[col][to] = board.countFlips(to, col);
			ecnt[colf][to] = board.countFlips(to, colf);
			if (ecnt[col][to])
			{
				if (ecnt[colf][to]>maxk) max2k = maxk, maxk = ecnt[colf][to];
				else if (ecnt[colf][to]>max2k) max2k = ecnt[colf][to];
			}
			if (ecnt[colf][to])
			{
				if (ecnt[col][to]>maxe) max2e = maxe, maxe = ecnt[col][to];
				else if (ecnt[col][to]>max2e) max2e = ecnt[col][to];
			}
		}
	}
	for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
	{
		to = first0(empty);
		if (board.canMove(to, col))
		{
			if (maxe>1 && ecnt[col][to] == 0) continue;
			if (cut && maxe>2 && ecnt[col][to] == 1 && ecnt[colf][to]<4) continue;
			if (ecnt[colf][to])
			{
				Val te = 0;
				if (ecnt[colf][to] == maxk) te = ecnt[col][to] - max2k*0.5;
				else te = ecnt[col][to] - maxk*0.5;
				tv[int(te * 2 + 10 + 0.01)].push_back({ to,49,0 });
			}
			if (ecnt[col][to] == 0 || (ecnt[col][to]<maxe - 2)) continue;
			if (cut && (ecnt[col][to]<maxe - 1)) continue;
			int fr;
			for (ull jfr = board.createJump(to, col); jfr; jfr ^= 1ull << fr)
			{
				fr = first0(jfr);
				//board.print();
				Board tboard = board;
				tboard.setPiece(to, fr, col);
				int cr = tboard.countFlips(fr, col), cc = tboard.countFlips(fr, colf);
				if (cc>1 && cr<4 && (ecnt[col][to]<maxe)) continue;
				if (cr && cr<3 && cc>2 && (ecnt[col][to]<maxe)) continue;
				Val te = 0;
				if (cr && cc>maxk) te = ecnt[col][to] - cc*0.5;
				else if (ecnt[colf][to] == maxk) te = ecnt[col][to] - max2k*0.5;
				else te = ecnt[col][to] - maxk*0.5;
				tv[int(te * 2 + 10 - 1.8 + 0.01)].push_back({ to, fr, 1 });
			}
		}
	}
	int maxv = -1;
	for (int i = 29;i >= 0;i--)
		if (!tv[i].empty())
		{
			if (maxv == -1) maxv = i;
			if (cut && cnt[0]>9 && i<maxv - 5) break;
			v.insert(v.end(), tv[i].begin(), tv[i].end());
		}
	stat += v.size();
}

float wmid[7],wcol[7],wcnt[7],wcor[7][729],wed[7][729],wspace[7][81];

void loadArgs(string filename)
{
	//sections:  0:30~34 1:25~29 2:20~24 3:15~19 4:10~14 5:5~9 6:2~4
	ifstream finput(filename);
	for (int k=0;k<7;k++)
	{
		finput>>wmid[k]>>wcol[k]>>wcnt[k];
		for (int i=0;i<729;i++) finput>>wcor[k][i];
		for (int i=0;i<729;i++) finput>>wed[k][i];
		for (int i=0;i<81;i++) finput>>wspace[k][i];
	}
	fout<<wmid[0]<<' ';
}

//return one section
int getTnum(int remain)
{
	if (remain>=30) return 0;
	else if (remain>=25) return 1;
	else if (remain>=20) return 2;
	else if (remain>=15) return 3;
	else if (remain>=10) return 4;
	else if (remain>=5) return 5;
	return 6;
}

int boardarr[7][7];

Val evalReg(Board &oboard, int col)
{
	Board board;
	if (col==1)
		board=oboard;
	else
	{
		board.m[0]=oboard.m[1];
		board.m[1]=oboard.m[0];
	}
	int cnt[3];
	board.cntPiece(cnt);
	int d=getTnum(cnt[0]);
	float sigma=0;
	int to;
	for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
	{
		to = first0(empty);
		if (board.isEmpty(to))
		{
			int cr=board.countFlips(to, 2), cc=board.countFlips(to, 1);
			sigma+=wspace[d][cr*9+cc];
		}
	}
	
	Board tb=board;
	tb.m[0]&=mask1[27]|(1ull<<27); tb.m[1]&=mask1[27]|(1ull<<27);
	sigma+=wmid[d]*(tb.cntPieceDelta())/5.0;
	sigma+=wcnt[d]*board.cntPieceDelta()/5.0;
	//cout<<cnt_start[0]<<' ';
	//board.print();
	//cout<<wmid[d]*(tb.cntPieceDelta())/5.0<<' '<<wcnt[d]*board.cntPieceDelta()/5.0<<' ';
	board.toArr(boardarr);
	int s=0; s=s*3+boardarr[0][0];s=s*3+boardarr[0][1];s=s*3+boardarr[0][2];s=s*3+boardarr[1][0];s=s*3+boardarr[1][1];s=s*3+boardarr[2][0];sigma+=wcor[d][s];
	s=0;s=s*3+boardarr[0][6];s=s*3+boardarr[0][5];s=s*3+boardarr[0][4];s=s*3+boardarr[1][6];s=s*3+boardarr[1][5];s=s*3+boardarr[2][6];sigma+=wcor[d][s];
	s=0;s=s*3+boardarr[6][0];s=s*3+boardarr[6][1];s=s*3+boardarr[6][2];s=s*3+boardarr[5][0];s=s*3+boardarr[5][1];s=s*3+boardarr[4][0];sigma+=wcor[d][s];
	s=0;s=s*3+boardarr[6][6];s=s*3+boardarr[6][5];s=s*3+boardarr[6][4];s=s*3+boardarr[5][6];s=s*3+boardarr[5][5];s=s*3+boardarr[4][6];sigma+=wcor[d][s];
	s=0;s=s*3+boardarr[0][2];s=s*3+boardarr[0][3];s=s*3+boardarr[0][4];s=s*3+boardarr[1][2];s=s*3+boardarr[1][3];s=s*3+boardarr[1][4];sigma+=wed[d][s];
	s=0;s=s*3+boardarr[6][2];s=s*3+boardarr[6][3];s=s*3+boardarr[6][4];s=s*3+boardarr[5][2];s=s*3+boardarr[5][3];s=s*3+boardarr[5][4];sigma+=wed[d][s];
	s=0;s=s*3+boardarr[2][0];s=s*3+boardarr[3][0];s=s*3+boardarr[4][0];s=s*3+boardarr[2][1];s=s*3+boardarr[3][1];s=s*3+boardarr[4][1];sigma+=wed[d][s];
	s=0;s=s*3+boardarr[2][6];s=s*3+boardarr[3][6];s=s*3+boardarr[4][6];s=s*3+boardarr[2][5];s=s*3+boardarr[3][5];s=s*3+boardarr[4][5];sigma+=wed[d][s];
	//cout<<sigma<<' ';
	//system("pause");
	return sigma;
}

//get evaluation of edge struction
Val edgeEval(Board &board, int col)
{
	Val ret = 0;
	int pos;
	for (ull bpos = (board.m[0] | board.m[1]) & maskedge; bpos; bpos ^= 1ull << pos)
	{
		pos = first0(bpos);
		int te = board.countNearSpaces(pos);
		Val cr;
		if (te == 0) cr = 0.4;
		else if (te == 1) cr = 0.08;
		else cr = 0.12;
		if (maskcor & (1ull<<pos)) cr*=1.5;
		if (board.getCol(pos) == col) ret += cr;
		else ret -= cr;
	}
	return ret;
}

//faster minimax search
Val minimax1(Board &board, int col, Val alpha, Val beta, int deep, Val jumped)
{
	leaf_count++;
	if (outtime) return 100;
	if (leaf_count % 2000 == 0 && clock() - t0>CLOCKS_PER_SEC * MAX_TIME)
	{
		outtime = true;
		return 100;
	}
	if (deep == maxdeep)
	{
		if ((cnt_start[0]>=36 || cnt_start[0]<=10))
			return eval(board, col)*1.0 + jumped;
		int cnt[3];
		board.cntPiece(cnt);
		if (cnt[0]<35 && cnt[0]>=2) return evalReg(board, col);
		return eval(board, col)*1.0 + jumped;
	}
	Val lyval = 0;
	int cnt[3];
	board.cntPiece(cnt);
	if (deep == maxdeep - 1)
	{
		if ((cnt_start[0]>=36 || cnt_start[0]<=10))
		{
			lyval += eval(board, col) * 0.6 + jumped*0.5;
		}
		else
		{
			int cnt[3];
			board.cntPiece(cnt);
			if (cnt[0]<35 && cnt[0]>=2) 
				lyval += evalReg(board, col)*0.6;
			else
			{
				lyval += eval(board, col) * 0.6 + jumped*0.5;
			}
		}
		if (cnt[0]>9 && cnt[0]<42)
			lyval += edgeEval(board, col);
	}
	if (cnt[1]==0 || cnt[2]==0) return eval(board, col)*1.0 + jumped;
	int maxe = 0;
	bool hasmove = false;
	Val maxc = -100;
	int to;
	for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
	{
		to = first0(empty);
		if (board.canCopy(to, col))
		{
			int te = board.countFlips(to, col);
			if (te>maxe) maxe = te;
		}
	}
	for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
	{
		to = first0(empty);
		if (board.canCopy(to, col))
		{
			int te = board.countFlips(to, col);
			if (maxe>1 && te == 0) continue;
			if (maxe>2 && te == 1) continue;
			Board tboard = board;
			tboard.setPiece(to, col);
			Val ret = -minimax1(tboard, othercol(col), -beta, -alpha, deep + 1, 0) + lyval;
			if (ret >= beta) return beta;
			if (ret >= alpha) alpha = ret;
			hasmove = true;
			if (ret>maxc) maxc = ret;
		}
	}
	if (cnt[0] == 1 && hasmove){
		//RETURN 
	}
	else
	{
		for (ull empty = board.createEmpty(); empty; empty ^= 1ull << to)
		{
			to = first0(empty);
			int te = board.countFlips(to, col);
			if (te == 0 || te<maxe - (cnt[0]<7)) continue;
			int fr;
			for (ull jfr = board.createJump(to, col); jfr; jfr ^= 1ull << fr)
			{
				fr = first0(jfr);
				//board.print();
				Board tboard = board;
				tboard.setPiece(to, fr, col);
				int cr = tboard.countFlips(fr, col), cc = tboard.countFlips(fr, othercol(col));
				Val cJump = evalJump[cr][cc] + 0.2;
				if (cnt[0]<5) cJump = 1.4;
				Val ret = -minimax1(tboard, othercol(col), -beta, -alpha, deep + 1, cJump) + lyval;
				if (ret >= beta) return beta;
				if (ret >= alpha) alpha = ret;
				hasmove = true;
				if (ret>maxc) maxc = ret;
			}
		}
	}
	if (!hasmove)
	{
		return evalEnd(board, col) + lyval;
	}
	return maxc;
}

//general minimax search
Val minimax(Board &board, int col, Val alpha, Val beta, int deep, Val jumped)
{
	if (deep == maxdeep - 1)
	{
		return minimax1(board, col, alpha, beta, deep, jumped);
	}
	leaf_count++;
	if (outtime) return 100;
	if (leaf_count % 300 == 0 && clock() - t0>CLOCKS_PER_SEC * MAX_TIME)
	{
		outtime = true;
		return 100;
	}
	//if (deep == maxdeep) {return eval(board, col)*1.0 + jumped;}
	Val lyval = 0;
	//if (deep == maxdeep - 1) {lyval = eval(board, col) * 0.6 + jumped*0.5;}
	int cnt[3];
	board.cntPiece(cnt);
	if (cnt[1]==0 || cnt[2]==0) return eval(board, col)*1.0 + jumped; //midgame win
	
	vector<Move> v;
	geneBranch<1>(board, col, v);
	if (v.empty()) return evalEnd(board, col) + lyval;
	Val maxc = -100;
	for (auto &it : v)
	{
		auto tboard = board; tboard.setPiece(it, col);
		Val cJump = 0;
		if (it.jump)
		{
			int cr = tboard.countFlips(it.fr, col), cc = tboard.countFlips(it.fr, othercol(col));
			cJump = evalJump[cr][cc] + 0.2;
			if (cnt[0]<5) cJump = 1.4;
		}
		Val ret = -minimax(tboard, othercol(col), -beta, -alpha, deep + 1, cJump) + lyval;
		if (ret >= beta) return beta;
		if (ret >= alpha) alpha = ret;
		if (ret > maxc) maxc = ret;
	}
	return maxc;
}

//start searching
Move run(Board &board, int _mcol)
{
	//board.print();
	leaf_count = 0;
	outtime = false;
	int cnt[3];
	board.cntPiece(cnt);
	board.cntPiece(cnt_start);
	fout << "AI1  col:" << _mcol << ' ' << "\n";
	Val maxc, lastc;
	mcol = _mcol;
	//std::pair
	struct Type
	{
		Val x;
		Move move;
		bool operator<(const Type &v) const{return x>v.x;}
	};
	const Val RANGE = 0.38;
	t0 = clock();
	vector<Type> list, lastlist;
	vector<Move> v;
	geneBranch<0>(board, mcol, v);
	for (auto &it : v)
	{
		maxdeep = 4;
		auto tboard = board; tboard.setPiece(it, mcol);
		Val ret = -minimax(tboard, othercol(mcol), -100, 100, 0, 0);
		list.push_back({ ret,it });
	}
	sort(list.begin(), list.end());
	while (list.size()>((cnt[0]<7) ? 12 : 6)) list.pop_back();
	v.clear();
	for (auto &it : list) v.push_back(it.move);

	while (clock()-t0<CLOCKS_PER_SEC*CTRL_TIME && maxdeep<35)
	{
		maxdeep++;
		maxc = -100;
		list.clear();
		for (auto &it:v)
		{
			auto tboard=board; tboard.setPiece(it, mcol);
			Val ret = -minimax(tboard, othercol(mcol), -100, 100, 0, 0);
			if (ret > maxc) maxc = ret;
			list.push_back({ret,it});
		}
		if (clock()-t0>CLOCKS_PER_SEC* MAX_TIME)
		{
			maxc=lastc;
			list.clear();
			list.insert(list.end(),lastlist.begin(),lastlist.end());
			maxdeep--;
			break;
		}
		lastc=maxc;
		lastlist.clear();
		lastlist.insert(lastlist.end(),list.begin(),list.end());
	
		sort(list.begin(),list.end());
		vector<Type> probmove;
		for (size_t i=0;i<5&&i<list.size();i++)
			//if (it.x>maxc - RANGE)
			probmove.push_back(list[i]);
		/* 
		for (auto &it : probmove)
		{
		it.move.print();
		fout << ':' << it.x << "    ";
		}
		fout << " TTL:"<<clock()-t0<< '\n';
		
	
		fout<< maxdeep << " " << clock()-t0<<" "<<leaf_count << "\n";*/ 
	}
	//maxmove.print();
	
	fout << "maxd:" << maxdeep << " maxc:" << maxc << " TTL:" << clock() - t0 << " leaf:" << leaf_count << " stat:" << (float)stat / statc << '\n';
	vector<Type> probmove;
	for (auto &it : list)
		if (it.x>maxc - RANGE)
			probmove.push_back(it);
	for (auto &it : probmove)
	{
		it.move.print();
		fout << ':' << it.x << "    ";
	}
	fout << '\n';
	return probmove[rand() % probmove.size()].move;
}
}

int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
int gridInfo[7][7] = { 0 }; // 先x后y，记录棋盘状态
int blackPieceCount = 2, whitePieceCount = 2;
static int delta[24][2] = {
	{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 },
	{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },
	{ 2,0 },{ 2,1 },{ 2,2 },{ 1,2 },
	{ 0,2 },{ -1,2 },{ -2,2 },{ -2,1 },
	{ -2,0 },{ -2,-1 },{ -2,-2 },{ -1,-2 },
	{ 0,-2 },{ 1,-2 },{ 2,-2 },{ 2,-1 } };

// 判断是否在地图内
inline bool inMap(int x, int y)
{
	if (x < 0 || x > 6 || y < 0 || y > 6)
		return false;
	return true;
}

// 向Direction方向改动坐标，并返回是否越界
inline bool MoveStep(int &x, int &y, int Direction)
{
	x = x + delta[Direction][0];
	y = y + delta[Direction][1];
	return inMap(x, y);
}

// 在坐标处落子，检查是否合法或模拟落子
bool ProcStep(int x0, int y0, int x1, int y1, int color)
{
	if (color != 1 && color != -1)
		return false;
	if (x1 == -1) // 无路可走，跳过此回合
		return true;
	if (inMap(x0, y0) == false || inMap(x1, y1) == false) // 超出边界
		return false;
	int dx, dy, x, y, currCount = 0, dir;
	dx = abs(x0 - x1), dy = abs(y0 - y1);
	if ((dx == 0 && dy == 0) || dx > 2 || dy > 2) // 保证不会移动到原来位置，而且移动始终在5×5区域内
		return false;
	if (gridInfo[x1][y1] != 0) // 保证移动到的位置为空
		return false;
	if (dx == 2 || dy == 2) // 如果走的是5×5的外围，则不是复制粘贴
		gridInfo[x0][y0] = 0;
	gridInfo[x1][y1] = color;
	for (dir = 0; dir < 8; dir++) // 影响邻近8个位置
	{
		x = x1 + delta[dir][0];
		y = y1 + delta[dir][1];
		if (inMap(x, y) && gridInfo[x][y] == -color)
		{
			currCount++;
			gridInfo[x][y] = color;
		}
	}
	if (currCount != 0)
	{
		if (color == 1)
		{
			blackPieceCount += currCount;
			whitePieceCount -= currCount;
		}
		else
		{
			whitePieceCount += currCount;
			blackPieceCount -= currCount;
		}
	}
	return true;
}


// 检查color方有无合法棋步
bool CheckIfHasValidMove(int color) // 检查有没有地方可以走
{
	int x0, y0, x, y, dir;
	for (y0 = 0; y0 < 7; y0++)
		for (x0 = 0; x0 < 7; x0++)
		{
			if (gridInfo[x0][y0] != color)
				continue;
			for (dir = 0; dir < 24; dir++)
			{
				x = x0 + delta[dir][0];
				y = y0 + delta[dir][1];
				if (inMap(x, y) == false)
					continue;
				if (gridInfo[x][y] == 0)
					return true;
			}
		}
	return false;
}

int chgCol(int c)
{
	if (c == 1) return 2;
	if (c == 0) return 0;
	if (c == -1) return 1;
	ASSERT(0);
	return 0;
}

#ifndef WIN_CON 
void runOnline()
{
	int x0, y0, x1, y1;

	// 初始化棋盘
	gridInfo[0][0] = gridInfo[6][6] = 1;  //|黑|白|
	gridInfo[6][0] = gridInfo[0][6] = -1; //|白|黑|

										  // 读入JSON
	string str;
	getline(cin, str);
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);
	AI::loadArgs("data/ataxxarg.txt");
	// 分析自己收到的输入和自己过往的输出，并恢复状态
	int turnID = input["responses"].size();
	currBotColor = input["requests"][(Json::Value::UInt) 0]["x0"].asInt() < 0 ? 1 : -1; // 第一回合收到坐标是-1, -1，说明我是黑方
	for (int i = 0; i < turnID; i++)
	{
		// 根据这些输入输出逐渐恢复状态到当前回合
		x0 = input["requests"][i]["x0"].asInt();
		y0 = input["requests"][i]["y0"].asInt();
		x1 = input["requests"][i]["x1"].asInt();
		y1 = input["requests"][i]["y1"].asInt();
		if (x0 >= 0)
			ProcStep(x0, y0, x1, y1, -currBotColor); // 模拟对方落子
		x0 = input["responses"][i]["x0"].asInt();
		y0 = input["responses"][i]["y0"].asInt();
		x1 = input["responses"][i]["x1"].asInt();
		y1 = input["responses"][i]["y1"].asInt();
		if (x0 >= 0)
			ProcStep(x0, y0, x1, y1, currBotColor); // 模拟己方落子
	}

	// 看看自己本回合输入
	x0 = input["requests"][turnID]["x0"].asInt();
	y0 = input["requests"][turnID]["y0"].asInt();
	x1 = input["requests"][turnID]["x1"].asInt();
	y1 = input["requests"][turnID]["y1"].asInt();
	if (x0 >= 0)
		ProcStep(x0, y0, x1, y1, -currBotColor); // 模拟对方落子

	Board board;
	for (int i = 0; i<7; i++)
		for (int j = 0; j<7; j++)
			board.setCol(i, j, chgCol(gridInfo[i][j]));
	auto dec = AI::run(board, chgCol(currBotColor));
	if (!dec.jump)
		dec.fr = first0(board.m[chgCol(currBotColor) - 1] & mask1[dec.to]);
	//logRefrsh();
	string debugText;
	getline(debug_s, debugText);
	while (!fout.eof())
	{
		debugText += "    ";
		string ts;
		getline(fout, ts);
		debugText += ts;
	}
	// 决策结束，输出结果（你只需修改以上部分）
	Json::Value ret;
	ret["response"]["x0"] = dec.fr / 8;
	ret["response"]["y0"] = dec.fr % 8;
	ret["response"]["x1"] = dec.to / 8;
	ret["response"]["y1"] = dec.to % 8;
	ret["debug"] = debugText;
	Json::StyledWriter writer;
	cout << writer.write(ret) << endl;
}
#endif

int main()
{
	//int seed=0;
	auto seed = (unsigned)time(NULL);
	fout << seed << '\n';
	debug_s << seed << ' ';
	initMask();
	srand(seed);
	//runOnline();
	logRefrsh();
	system("pause"); //auto ignore
	return 0;
}

