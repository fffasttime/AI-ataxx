#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <queue>
using namespace std;

bool inBorder(int x, int y)
{
	return (x >= 0 && x< M_SIZE && y >= 0 && y< M_SIZE);
}


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
	bool canCopy(int p, int col) const {
		return mask1[p] & m[col - 1];
	}
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

int ptnhash(int a[6])
{
	int ha=0;
	for (int i=0;i<6;i++) ha=ha*3+a[i];
	return ha;
}

const int max_size=160000;
const float le=0.01;

int ccnt;
int mm[max_size][7][7],mcol[max_size],batch_size=200;
float mval[max_size];

float w1[2*4*4*6*6], w2[2*6*6*5*5], w3[2*6*6*6*6],w4[2*9*9*4*4], w5[2*9*9*4*4], w6[2*9*9*4*4], w7[2*9*9*4*4];
float rw1[2*4*4*6*6], rw2[2*6*6*5*5], rw3[2*6*6*6*6],rw4[2*9*9*4*4], rw5[2*9*9*4*4], rw6[2*9*9*4*4], rw7[2*9*9*4*4];

float sse=0;

void printb(int num)
{
	for (int i=0;i<7;i++,cout<<'\n')
		for (int j=0;j<7;j++)
			cout<<mm[num][i][j]<<' ';
}

float ssig=0;

void valid(int num)
{
	int ptnCor[4][6], ptnEdge[4][6];
	getptns(num,ptnCor, ptnEdge);
	int pc[4],pe[4];
	for (int i=0;i<4;i++)
	{
		pc[i]=ptnhash(ptnCor[i]);
		pe[i]=ptnhash(ptnEdge[i]);
	}
	float cmid=getpmid(num)/5.0;
	float ccnt=getccnt(num)/5.0;
	float sigma=0;
	
	
	for (int i=1;i<6;i++)
		for (int j=1;j<6;j++)
			if (mm[num][i][j]==0)
			{
				int v=getSpace(num,i,j);
				sigma+=wspace[v];
			}
	
	for (int i=1;i<6;i++)
	{
		if (mm[num][0][i]==0)
		{
			int v=getESpace(num,0,i);
			sigma+=wedsp[v];
		}
		if (mm[num][6][i]==0)
		{
			int v=getESpace(num,6,i);
			sigma+=wedsp[v];
		}
		if (mm[num][i][0]==0)
		{
			int v=getESpace(num,i,0);
			sigma+=wedsp[v];
		}
		if (mm[num][i][6]==0)
		{
			int v=getESpace(num,i,6);
			sigma+=wedsp[v];
		}
	}
	
	for (int i=0;i<4;i++)
	{
		sigma+=wcor[pc[i]]+wed[pe[i]];
	}
	sigma+=wmid*cmid;
	sigma+=wcnt*ccnt;
	if (mcol[num]==1) sigma+=wcol;
	ssig+=sigma;
	
	float mse=fabs(sigma-mval[num]);
	sse+=mse;
}

void updateArg()
{
	float ee=le/batch_size;
	for (int i=0;i<729;i++)
	{
		wcor[i]+=rwcor[i]*ee;
		wed[i]+=rwed[i]*ee;
	}
	for (int i=0;i<81;i++)
		wspace[i]+=rwspace[i]*ee;
	for (int i=0;i<36;i++)
		wedsp[i]+=rwedsp[i]*ee;
	wmid+=rwmid*ee;
	wcol+=rwcol*ee;
	wcnt+=rwcnt*ee;
	for (int i=0;i<729;i++)
		rwcor[i]=rwed[i]=0;
	for (int i=0;i<81;i++)
		rwspace[i]=0;
	for (int i=0;i<36;i++)
		rwedsp[i]=0;
	rwmid=rwcol=rwcnt=0;
}

int d1[4], d2[8], d3[12], d4[4], d5[12], d6[4], d7[5];

int r0,r1,r2,r3,r4;
void getcc(int num, int pos, int &r0, int &r1, int &r2, int &r3, int &r4)
{
	r0=board[pos];
	r1=board[dis1]
	r1=board[dis1];
	//etc
}

/*
0  1  2  3  4  5  6
8  9  10 11 12 13 14
16 17 18 19 20 21 22
24 25 26 27 28 29 30
32 33 34 35 36 37 38
40 41 42 43 44 45 46 
48 49 50 51 52 53 54
*/

/*
1 2 3 3 3 2 1
2 4 5 5 5 4 2
3 5 6 7 6 5 3
3 5 7 7 7 5 3
3 5 6 7 6 5 3
2 4 5 5 5 4 2
1 2 3 3 3 2 1
*/
void getnumbers(int num)
{
	int t;
	getcc(0); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[0]=t;
	getcc(6); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[1]=t;
	getcc(48); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[2]=t;
	getcc(54); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[3]=t;
	
	getcc(1); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[0]=t;
	getcc(5); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[1]=t;
	getcc(8); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[2]=t;
	getcc(14); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[3]=t;
	getcc(40); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[4]=t;
	getcc(46); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[5]=t;
	getcc(49); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[6]=t;
	getcc(53); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[7]=t;
	
	getcc(2); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[0]=t;
	getcc(3); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[1]=t;
	getcc(4); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[2]=t;
	getcc(50); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[3]=t;
	getcc(51); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[4]=t;
	getcc(52); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[5]=t;
	getcc(16); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[6]=t;
	getcc(24); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[7]=t;
	getcc(32); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[8]=t;
	getcc(22); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[9]=t;
	getcc(30); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[10]=t;
	getcc(38); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[11]=t;
	
	getcc(9); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[0]=t;
	getcc(13); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[1]=t;
	getcc(41); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[2]=t;
	getcc(45); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[3]=t;
	
	getcc(10); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[0]=t;
	getcc(11); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[1]=t;
	getcc(12); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[5]=t;
	getcc(42); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[3]=t;
	getcc(43); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[4]=t;
	getcc(44); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[5]=t;
	getcc(17); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[6]=t;
	getcc(25); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[7]=t;
	getcc(33); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[8]=t;
	getcc(21); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[9]=t;
	getcc(29); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[10]=t;
	getcc(37); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[11]=t;
	
	getcc(18); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[0]=t;
	getcc(20); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[1]=t;
	getcc(34); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[2]=t;
	getcc(36); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[3]=t;
	
	getcc(19); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[0]=t;
	getcc(26); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[1]=t;
	getcc(27); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[2]=t;
	getcc(28); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[3]=t;
	getcc(35); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[4]=t;
}

void accuGrad(int num)
{
	getnumbers(num);
	for (int i=0;i<4;i++)
	{
		sigma+=w1[d1[i]];
		sigma+=w4[d4[i]];
		sigma+=w6[d6[i]];
	}
	for (int i=0;i<12;i++)
	{
		sigma+=w3[d3[i]];
		sigma+=w5[d5[i]];
	}
	for (int i=0;i<8;i++)
		sigma+=w2[d2[i]];
	for (int i=0;i<5;i++)
		sigma+=w7[d7[i]];
		
	float mse=(sigma-mval[num])*(sigma-mval[num])/2, delta=mval[num]-sigma;
	sse+=fabs(delta);
	
}

void saveArgs()
{
	ofstream foutp("trained35_39.txt");
	foutp<<wmid<<'\n';
	foutp<<wcol<<'\n';
	foutp<<wcnt<<'\n';
	for (int i=0;i<729;i++) foutp<<wcor[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<729;i++) foutp<<wed[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<81;i++) foutp<<wspace[i]<<' ';
	foutp<<'\n';
}

int main()
{ 
	ifstream fin("ataxxdata35_39.txt");
	int col;
	while (fin>>col)
	{
		mcol[ccnt]=col;
		for (int i=0;i<7;i++)
			for (int j=0;j<7;j++)
			{ 
				fin>>mm[ccnt][i][j];
				if (mcol[ccnt]==1) 
					if (mm[ccnt][i][j]!=0)
						mm[ccnt][i][j]=mm[ccnt][i][j]%2+1;
			} 
		fin>>mval[ccnt];
		//if (mcol[ccnt]==1) mval[ccnt]=-mval[ccnt];
		//mval[ccnt]-=5.4;
		ccnt++;
	}
	cout<<ccnt<<" board loaded\n";
	ccnt=80000;
	for (int i=0;i<40000;i++) 
	{
		for (int j=0;j<batch_size;j++)
			accuGrad((i*batch_size+j)%ccnt);
		if (i%200==199)
		{
			cout<<i<<' '<<sse/(200*batch_size)<<'\n';
			sse=0;
		}
		updateArg();
	}
	cout<<"wcol:"<<wcol<<"\nwmid:"<<wmid<<"\n";
	float vy_=0;
	for (int i=ccnt;i<ccnt+1000;i++)
	
	{
		valid(i);
		if (i%200==199)
		{
			cout<<i<<' '<<sse/200<<'\n';
			sse=0;
		}
		vy_+=mval[i];
	}
	cout<<"wcnt:"<<wcnt<<'\n';
	cout<<ssig/1000<<'\n';
	cout<<vy_/1000<<'\n';
	saveArgs();
	return 0;
}

