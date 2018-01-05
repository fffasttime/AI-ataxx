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
#include <algorithm>
using namespace std;

#define M_SIZE 7

bool inBorder(int x, int y)
{
	return (x >= 0 && x< M_SIZE && y >= 0 && y< M_SIZE);
}


int othercol(int col)
{
	if (col == 2) return 1;
	if (col == 1) return 2;
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
		//fout << '<' << x << ',' << y;
		//if (jump) fout << ',' << fx << ',' << fy;
		//fout << '>';
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
	void print()
	{
		for (int i=0;i<7;i++,cout<<'\n')
			for (int j=0;j<7;j++)
				cout<<getCol(i,j)<<' ';
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
				if (_m[i][j])
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
	int countDis1(int p, int col) const
	{
		return popcount(m[(col - 1)] & mask1[p]);
	}
	int countDis2(int p, int col) const
	{
		return popcount(m[(col - 1)] & mask2[p]);
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
Board board[max_size]; 
int mcol[max_size],batch_size=200;
float mval[max_size];

float w1[3*4*4*6*6], w2[3*6*6*5*5], w3[3*6*6*6*6],w4[3*9*9*4*4], w5[3*9*9*4*4], w6[3*9*9*4*4], w7[3*9*9*4*4],wcol,wodd,wb,wcnt;
float rw1[3*4*4*6*6], rw2[3*6*6*5*5], rw3[3*6*6*6*6],rw4[3*9*9*4*4], rw5[3*9*9*4*4], rw6[3*9*9*4*4], rw7[3*9*9*4*4],rwcol,rwodd,rwb,rwcnt;

float sse=0;

void printb(int mm[7][7])
{
	for (int i=0;i<7;i++,cout<<'\n')
		for (int j=0;j<7;j++)
			cout<<mm[i][j]<<' ';
}

float ssig=0;

void updateArg()
{
	float ee=le/batch_size;
	for (int i=0;i<3*4*4*6*6;i++) 
		w1[i]+=rw1[i]*ee,rw1[i]=0;
	for (int i=0;i<3*6*6*5*5;i++) 
		w2[i]+=rw2[i]*ee,rw2[i]=0;
	for (int i=0;i<3*6*6*6*6;i++) 
		w3[i]+=rw3[i]*ee,rw3[i]=0;
	for (int i=0;i<3*9*9*4*4;i++) 
		w4[i]+=rw4[i]*ee,rw4[i]=0;
	for (int i=0;i<3*9*9*4*4;i++) 
		w5[i]+=rw5[i]*ee,rw5[i]=0;
	for (int i=0;i<3*9*9*4*4;i++) 
		w6[i]+=rw6[i]*ee,rw6[i]=0;
	for (int i=0;i<3*9*9*4*4;i++) 
		w7[i]+=rw7[i]*ee,rw7[i]=0;
	wcol+=rwcol*ee; rwcol=0;
	wodd+=rwodd*ee; rwodd=0;
	wcnt+=rwcnt*ee; rwcnt=0;
	wb+=rwb*ee; rwb=0;
}

int d1[4], d2[8], d3[12], d4[4], d5[12], d6[4], d7[5];

int gnum,r0,r1,r2,r3,r4;
void getcc1(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
}

void getcc2(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
	if (r3>1) r3--; if (r3==5) r3--;
	if (r4>1) r4--; if (r4==5) r4--; 
}

void getcc3(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
	r3=(r3+2)/3; 
	r4=(r4+2)/3;
}

void getcc4(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
	r3/=2; 
	r4/=2;
}

void getcc5(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
	r3=(r3+3)/4; 
	r4=(r4+3)/4;
}

void getcc6(int p)
{
	r0=board[gnum].getCol(p);
	r1=board[gnum].countDis1(p,1);
	r2=board[gnum].countDis1(p,2);
	r3=board[gnum].countDis2(p,1)-r1;
	r4=board[gnum].countDis2(p,2)-r2;
	r3/=4; if (r3==4) r4--;
	r4/=4; if (r3==4) r4--;
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
	gnum=num;
	int t;
	getcc1(0); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[0]=t;
	getcc1(6); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[1]=t;
	getcc1(48); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[2]=t;
	getcc1(54); t=r4+r3*6+r2*36+r1*144+r0*576;
	d1[3]=t;
	
	getcc2(1); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[0]=t;
	getcc2(5); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[1]=t;
	getcc2(8); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[2]=t;
	getcc2(14); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[3]=t;
	getcc2(40); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[4]=t;
	getcc2(46); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[5]=t;
	getcc2(49); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[6]=t;
	getcc2(53); t=r4+r3*5+r2*25+r1*150+r0*900;
	d2[7]=t;
	
	getcc3(2); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[0]=t;
	getcc3(3); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[1]=t;
	getcc3(4); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[2]=t;
	getcc3(50); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[3]=t;
	getcc3(51); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[4]=t;
	getcc3(52); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[5]=t;
	getcc3(16); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[6]=t;
	getcc3(24); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[7]=t;
	getcc3(32); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[8]=t;
	getcc3(22); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[9]=t;
	getcc3(30); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[10]=t;
	getcc3(38); t=r4+r3*6+r2*36+r1*216+r0*1296;
	d3[11]=t;
	
	getcc4(9); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[0]=t;
	getcc4(13); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[1]=t;
	getcc4(41); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[2]=t;
	getcc4(45); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d4[3]=t;
	
	getcc5(10); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[0]=t;
	getcc5(11); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[1]=t;
	getcc5(12); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[5]=t;
	getcc5(42); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[3]=t;
	getcc5(43); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[4]=t;
	getcc5(44); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[5]=t;
	getcc5(17); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[6]=t;
	getcc5(25); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[7]=t;
	getcc5(33); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[8]=t;
	getcc5(21); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[9]=t;
	getcc5(29); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[10]=t;
	getcc5(37); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d5[11]=t;
	
	getcc6(18); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[0]=t;
	getcc6(20); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[1]=t;
	getcc6(34); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[2]=t;
	getcc6(36); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d6[3]=t;
	
	getcc6(19); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[0]=t;
	getcc6(26); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[1]=t;
	getcc6(27); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[2]=t;
	getcc6(28); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[3]=t;
	getcc6(35); t=r4+r3*4+r2*16+r1*144+r0*1296;
	d7[4]=t;
}

void valid(int num)
{
	getnumbers(num);
	int cnt[3]; board[num].cntPiece(cnt);
	float sigma=0;
	
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
		
	sigma+=wb;
	sigma+=(cnt[1]-cnt[2])*wcnt;
	if (cnt[0]&1) sigma+=wodd;
	if (mcol[num]==1) sigma+=wcol;
	
	ssig+=sigma;
	
	float mse=fabs(sigma-mval[num]);
	sse+=mse;
}

void accuGrad(int num)
{
	//board[num].print();
	getnumbers(num);
	int cnt[3]; board[num].cntPiece(cnt);
	float sigma=0;
	
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
		
	sigma+=wb;
	sigma+=(cnt[1]-cnt[2])*wcnt;
	if (cnt[0]&1) sigma+=wodd;
	if (mcol[num]==1) sigma+=wcol;
	
	float mse=(sigma-mval[num])*(sigma-mval[num])/2, delta=mval[num]-sigma;
	sse+=fabs(delta);
	
	for (int i=0;i<4;i++)
	{
		rw1[d1[i]]+=delta;
		rw4[d4[i]]+=delta;
		rw6[d6[i]]+=delta;
	}
	for (int i=0;i<12;i++)
	{
		rw3[d3[i]]+=delta;
		rw5[d5[i]]+=delta;
	}
	for (int i=0;i<8;i++)
		rw2[d2[i]]+=delta;
	for (int i=0;i<5;i++)
		rw7[d7[i]]+=delta;
	
	rwb+=delta/16;
	//rwcnt+=delta/64;
	if (cnt[0]&1) wodd+=delta/8;
	//if (mcol[num]==1) rwcol+=delta/8;
	/*
	if (fabs(rw1[1038]+90.7)<0.1)
	{
		board[num].print();
		cout<<num;
		system("pause");
	}*/
}

void saveArgs()
{
	ofstream foutp("trained2_4.txt");
	foutp<<wcol<<'\n';
	for (int i=0;i<3*4*4*6*6;i++) foutp<<w1[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*6*6*5*5;i++) foutp<<w2[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*6*6*6*6;i++) foutp<<w3[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*9*9*4*4;i++) foutp<<w4[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*9*9*4*4;i++) foutp<<w5[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*9*9*4*4;i++) foutp<<w6[i]<<' ';
	foutp<<'\n';
	for (int i=0;i<3*9*9*4*4;i++) foutp<<w7[i]<<' ';
	foutp<<'\n';
}

int mm[7][7];

void shuffle()
{
	for (int i=0;i<ccnt;i++)
	{
		int rd=rand()*32768+rand(); rd%=ccnt;
		swap(board[i],board[rd]);
		swap(mval[i],mval[rd]);
		//swap col	
	}
}

int main()
{ 
	initMask();
	ifstream fin("ataxxdata1_2.txt");
	int col;
	while (fin>>col)
	{
		mcol[ccnt]=col;
		for (int i=0;i<7;i++)
			for (int j=0;j<7;j++)
			{ 
				fin>>mm[i][j];
				if (mcol[ccnt]==1) 
					if (mm[i][j]!=0)
						mm[i][j]=mm[i][j]%2+1;
			} 
		board[ccnt].clear();
		board[ccnt].set(mm);
		fin>>mval[ccnt];
		//if (mcol[ccnt]==1) mval[ccnt]=-mval[ccnt];
		//mval[ccnt]-=5.4;
		ccnt++;
	}
	cout<<ccnt<<" board loaded\n";
	ccnt=40000;
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
	//cout<<"wcol:"<<wcol<<"\nwmid:"<<wmid<<"\n";
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
	cout<<"wcol:"<<wcol<<'\n';
	cout<<"wodd:"<<wodd<<'\n';
	cout<<"wb:"<<wb<<'\n';
	cout<<ssig/1000<<'\n';
	cout<<vy_/1000<<'\n';
	//saveArgs();
	return 0;
}

