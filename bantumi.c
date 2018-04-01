#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
/* Bantumi with AI v2.0  */
/* by Alex Andre' Guintu */

const POS_INF = +9999;
const NEG_INF = -9999;
const WIN = 321;
const LOSE = 123;
const TIE = 333;
const ERROR = 12345;

int BEANS = 4;
int MAX_DEPTH = 3;
int NOWHERE = 666;
int NODES = 0;

//prototypes;
void Intro(); void Help();
void DrawDigit(int number, int x, int y);
void DrawNumber(int number, int x, int y);
void DisplayBoard();
void UpdateBoard(int beans, int player, int pot);
void ShowPointer(int player, int pot);
void YourTurn(); void CpuTurn(); void Chomp();
void FinalScores(struct GameState b);
void DeclareDecision(struct GameState b, int result);

//Global Declarations
struct GameState {
    int hole[2][7];
    // [0] - YOU
    // [1] - CPU
};

struct MoveList {
    int *moves;
    int count;
};

struct Move {
    int move;
    int score;
};

int player, winner;
char *tech;
int lastpointer[2];
int lastmove;

//Initialize pots to 4, homepots to 0
void InitHoles(struct GameState *board){
    int i,j;
    for (i=0; i<2; i++){
	for (j=0; j<7; j++){
	    board->hole[i][j] = BEANS;
	}
    }
    board->hole[0][6] = 0;
    board->hole[1][6] = 0;
}

int ApplyMove(struct GameState *b, int player, int move) {
    int count,beans,temp;
    beans=b->hole[player][move]; // get beans from selected move
    b->hole[player][move]=0;     // set beans of move to ZERO
    move++;                      // move to the next move
    do {
	for (count=move;count<7;count++) {
	    if((player!=player && count!=6)) {
	       b->hole[player][count]++;
	       beans--;
	    }
	    if(player==player) {
	       b->hole[player][count]++;
	       beans--;
	    }
	    if (beans==0) break;
	}
	if (player==0) player=1;
	else player=0;
	temp=count;
	move=0;
    } while(beans>0);

    if (player!=player && temp!=6) {  /* check for the 'chomp' rule*/
	if (player==1 && b->hole[1][temp]==1 && b->hole[0][5-temp]!=0) {
	    b->hole[1][6]+=(b->hole[0][5-temp] + 1);
	    b->hole[0][5-temp]=0;
	    b->hole[1][temp]=0;
	} else if (player==0 && b->hole[0][temp]==1 && b->hole[1][5-temp]!=0) {
	    b->hole[0][6]+=(b->hole[1][5-temp] + 1);
	    b->hole[1][5-temp]=0;
	    b->hole[0][temp]=0;
	}
    }
    return (temp==6?1:0);  /* return the last pot where a bean is placed */
}

// Applies the moves to the real game
int TurnAnimate(struct GameState *b, int who, int pot) {
    int count,beans,temp;
    beans=b->hole[who][pot]; // get beans from selected pot
    b->hole[who][pot]=0;     // set beans of pot to ZERO
    UpdateBoard(b->hole[who][pot],who,pot);
    pot++;                   // move to the next pot

    do {
	for (count=pot;count<7;count++) {
	    if((who!=player && count!=6)) {
	       b->hole[who][count]++;
	       UpdateBoard(b->hole[who][count],who,count);
	       beans--;
	    }
	    if(who==player) {
	       b->hole[who][count]++;
	       UpdateBoard(b->hole[who][count],who,count);
	       beans--;
	    }
	    if (beans==0) break;
	}
	if (who==0) who=1;
	else who=0;
	temp=count;
	pot=0;
    } while(beans>0);

    if (who!=player && temp!=6) {  /* check for the 'chomp' rule*/
	if (player==1 && b->hole[1][temp]==1 && b->hole[0][5-temp]!=0) {
	    Chomp();
	    b->hole[1][6]+=(b->hole[0][5-temp] + 1);
	    b->hole[0][5-temp]=0;
	    b->hole[1][temp]=0;
	    UpdateBoard(b->hole[1][temp],1,temp); //empty own hole
	    delay(500);
	    UpdateBoard(b->hole[0][5-temp],0,5-temp); //empty opposite hole
	    delay(500);
	    UpdateBoard(b->hole[1][6],1,6); //update base
	    delay(500);
	} else if (player==0 && b->hole[0][temp]==1 && b->hole[1][5-temp]!=0) {
	    Chomp();
	    b->hole[0][6]+=(b->hole[1][5-temp] + 1);
	    b->hole[1][5-temp]=0;
	    b->hole[0][temp]=0;
	    UpdateBoard(b->hole[0][temp],0,temp);
	    delay(500);
	    UpdateBoard(b->hole[1][5-temp],1,5-temp);
	    delay(500);
	    UpdateBoard(b->hole[0][6],0,6);
	    delay(500);
	}
    }
    return (temp==6?1:0);  /* return the last pot where a bean is placed */
}

int CheckWin(struct GameState b) { /* checks if somebody won */
    int won[]={0,0},i,j;
    for (i=0;i<2;i++) {     /* checks the number of empty pots */
	for (j=0;j<6;j++) {
	    if (b.hole[i][j]==0) won[i]++;
	}
    }
    if (won[0]==6 || won[1]==6) /* if the are 6 empty pots in a side */
	return 1;
    return 0;
}

int CanMunch (struct GameState b, int player, int target, int source) {
    // if source and target are not empty
    if (b.hole[player][source]!=0  && b.hole[!player][target]!=0)
	// if landing is empty or equal to source
	if (b.hole[player][5-target]==0 || source==5-target)
	    if ((5-target-source>=0?5-target-source==b.hole[player][source]%13:
			       (5-target-source)+13==b.hole[player][source]))
		return 1;
    return 0;
}

int SingleRiskAt (struct GameState b, int target, int *threat,int player){
    int i,risks=0;
    for (i=0;i<6;i++)
	if (i!=5-target)
	    if (5-target-i>=0?5-target-i==b.hole[!player][i]:
			     (5-target-i)+13==b.hole[!player][i]) {
		*threat=i;
		if (++risks>1) {
		    *threat=-1;
		    return 0;
		}
	    }
    if (*threat==-1) return 0;
    return 1;
}

int RiskMunch(struct GameState b,int player) {
    int i,j;
    int x=-1;
    int munch=-1, risk=-1;
    int gain=-1, loss=-1;

    //Search for repeat turn
    tech = " SS";
    for (i=5;i>=0;i--)
	if ((b.hole[player][i]%13)+i==6)
	    return i;

    //Find Best Munch
      for (i=0;i<6;i++)
	  for (j=0;j<6;j++)
		if (CanMunch(b,player,j,i)) {
		    if (munch==-1) {
			munch=i;
			gain = b.hole[!player][j];
			if (b.hole[player][i]<=13 && b.hole[player][i]>7)
			    gain++;
		    } else {
			x = b.hole[!player][j];
			if (x<=13 && x>7) x++;
			if (gain < x) {
			    gain = x;
			    munch=i;
			}
			x=-1;
		    }
		}

      //Find Highest Risk
      for (i=0;i<6;i++)
	  for (j=0;j<6;j++)
		if (CanMunch(b,!player,j,i)) {
		    if (risk==-1) {
			risk=j;
			loss = b.hole[player][j];
			if (b.hole[!player][i]<=13 && b.hole[!player][i]>7)
			    loss++;
		    } else {
			x = b.hole[player][j];
			if (b.hole[!player][i] <=13 && b.hole[!player][i]>7) x++;
			if (loss < x) {
			    loss=x;
			    risk=j;
			}
			x=-1;
		    }
		}

     // Decide: Munch or Avoid
     if (munch!=-1 && risk!=-1) {
	 if (gain > loss) {
	     tech = " BM,NR";
	     return munch;
	 } else if (munch==risk) {
	     tech = " BM,AR";
	     return risk;
	 } else if (SingleRiskAt(b,risk,&x,player)) {
	     if (CanMunch(b,player,x,munch)) {
		 tech = " AR,BM";
		 return munch;
	     } else {
		 for (i=0;i<5;i++) {
		     if (CanMunch(b,player,x,i)) {
			 tech = " AR,M";
			 return i;
		     }
		 }
	     }
	 } else {
	     tech = " AR,H";
	     return risk;
	 }
    } else if (munch!=-1) {
	 tech = " BM";
	 return munch;
    } else if (risk!=-1) {
	 tech = " AR";
	 return risk;
    }

    //Resort to Guessing
    do{
      i=random(6);
    } while(b.hole[player][i]==0);
    tech = " GW";
    return i;
}

struct MoveList ValidMoves(struct GameState b, int player) {
    int i,j;
    struct MoveList moves;
    moves.count=0;
    for (i=0;i<6;i++)
	if (b.hole[player][i] != 0) moves.count++;
    moves.moves = (int*)malloc(sizeof(int)*moves.count);
    for (i=0,j=0;i<6;i++)
	if (b.hole[player][i] != 0) {
	     moves.moves[j] = i;
	     j++;
	}
    return moves;
}

int EvalBoard (struct GameState b, int player){
    return b.hole[player][6];
}

struct Move abMinMax (struct GameState b,
		      int depth, int limit,
		      int alpha, int beta) {
    int i, newdepth=depth;
    int player = !(depth%2);
    int endgame, extraturn;
    struct GameState newb;
    struct MoveList list;
    struct Move best;
    struct Move some;

    NODES++;
    endgame = CheckWin(b);
    if (depth == limit || endgame){
	if (endgame) DetermineWinner(&b);
	best.score = EvalBoard(b,player);
	return best;
    }

    list = ValidMoves(b,player);
    if (depth==0)
       best.move = NOWHERE;
    best.score = NEG_INF;


    if (player) {
	for (i=0;i<list.count;i++) {
	    newb = b;
	    extraturn = ApplyMove(&newb,player,list.moves[i]);
	    if (!extraturn) newdepth = depth+1;
	    some = abMinMax(newb,newdepth,limit,alpha,beta);
	    //if (some.score>alpha){
	    //	alpha = some.score;
	    if (some.score>best.score){
		best.score=some.score;
		if (depth==0)
		    best.move = list.moves[i];
	    }
	    //}
	    //if (alpha>=beta) {
	    //	best.score = alpha;
	    //	return best;
	    //}
	}
	//best.score=alpha;
	return best;
    } else {
	for (i=0;i<list.count;i++) {
	    newb = b;
	    extraturn = ApplyMove(&newb,player,list.moves[i]);
	    if (!extraturn) newdepth = depth+1;
	    some = abMinMax(newb,newdepth,limit,alpha,beta);
	    //if (some.score<beta){
	    //	beta = some.score;
	    if (some.score>best.score){
		best.score=some.score;
		if (depth==0)
		    best.move = list.moves[i];
	    }
	    //}
	    //if (beta<=alpha) {
	    //	best.score = beta;
	    //	return best;
	    //}
	}
	//best.score=beta;
	return best;
    }
}

void main(void) {
    int choice, confirm;
    int keypress,exit;
    struct GameState game;
    clrscr();
    randomize();
    player=random(2);
    Intro();
    exit=getch();
    while (exit!='q' && exit!='Q') { // main game logic
	clrscr();
	if (exit=='h' || exit=='H'){
	    Help();
	} else if (exit==13) {
	clrscr();
	winner=333;
	player=!player;
	lastpointer[0]=-1; lastpointer[1]=-1;
	lastmove=-1;
	InitHoles(&game);
	DisplayBoard(game);
	do {
	    do{
		if (player==0) {
		    if (CheckWin(game)) break;
		    if (lastmove!=-1) {
			ShowPointer(0,lastmove);
			choice=lastmove;
		    } else {
			ShowPointer(0,0);
			choice=0;
		    }
		    YourTurn();
		    DrawDigit(choice+1,40,10);
		    do{
			keypress=getch();
			confirm = 0;
			switch (keypress) {
			    case 75: if (choice>0)
					 choice--;
					 ShowPointer(0,choice);
					 DrawDigit(choice+1,40,10);
				     break;
			    case 77: if (choice<5)
					 choice++;
					 ShowPointer(0,choice);
					 DrawDigit(choice+1,40,10);
				     break;
			    case 13: if (game.hole[0][choice]!=0) {
					 lastmove=choice;
					 confirm=1;
				     }
				     break;
			    case 'b': choice = RiskMunch(game,0);
				      lastmove = choice;
				      ShowPointer(0,choice);
				      DrawDigit(choice+1,40,10);
				      cputs(tech);
				      confirm=1;
				      break;
			    case 'c' : break;
			}
		    } while(!confirm);
		} else {
		    if (CheckWin(game)) break;
		    CpuTurn();
		    DrawDigit(99,40,10);
		    //choice = RiskMunch(game,1);
		    choice =  abMinMax(game,0,MAX_DEPTH,NEG_INF,POS_INF).move;
		    if (choice<0 || choice>5)
			delay(500);
		    delay(500);
		    DrawDigit(choice+1,40,10);
		    //cputs(tech);
		    delay(600);
		    //getch();
		}
		if (CheckWin(game)) break;
	    } while(TurnAnimate(&game,player,choice));
	    if (player==0) player=1;
	    else player=0;
	} while(!CheckWin(game));
	clrscr();
	DeclareDecision(game,DetermineWinner(&game));
	}
	clrscr();
	Intro();
	exit = getch();
    }
}

void Help() {
    gotoxy(20,5);
    cputs("€ﬂ€ €ﬂ€ €ﬂ‹ ﬂ€ﬂ € € €ﬂ‹ﬂ‹ ﬂ  € € €ﬂ€ €   €ﬂ€"); gotoxy(20,6);
    cputs("€ﬂ‹ € € € €  €  € € € € € €  €‹€ €‹€ €   €‹€"); gotoxy(20,7);
    cputs("€‹€ €ﬂ€ € €  €  €‹€ €   € €  € € €‹‹ €‹‹ €  "); gotoxy(20,8);
    gotoxy(14,9);
    cputs("Use the Æ Ø keys to Move the cursor to your desired pot.");
    gotoxy(26,10);
    cputs("Press enter to confirm your move.");
    gotoxy(20,11);
    cputs("The player with most number of points wins!");
    gotoxy(29,20);
    cputs("PRESS A KEY TO GO BACK");
    getch();
}

void CpuTurn() {
    gotoxy(25,10);
    cputs("                         "); gotoxy(25,11);
    cputs("€ﬂ€ €ﬂ€ € € ‹            "); gotoxy(25,12);
    cputs("€   €‹€ € €              "); gotoxy(25,13);
    cputs("€‹€ €   €‹€ ﬂ            "); gotoxy(25,14);
    cputs("                         ");
}

void Chomp() {
    textcolor(RED);
    gotoxy(25,10);
    cputs("                         "); gotoxy(25,11);
    cputs("€ﬂ€ € € €ﬂ‹ €ﬂ‹ﬂ‹ €ﬂ€ €  "); gotoxy(25,12);
    cputs("€   €‹€ € € € € € €‹€ €  "); gotoxy(25,13);
    cputs("€‹€ € € ﬂ‹€ € € € €   ‹  "); gotoxy(25,14);
    cputs("                         ");
    textcolor(LIGHTGRAY);
}

void YourTurn() {
    gotoxy(25,10);
    cputs("                         "); gotoxy(25,11);
    cputs("€ € €ﬂ‹ € € ‹            "); gotoxy(25,12);
    cputs("ﬂﬂ€ € € € €              "); gotoxy(25,13);
    cputs("€‹€ ﬂ‹€ €‹€ ﬂ            "); gotoxy(25,14);
    cputs("                         ");
}

void ShowScore(struct GameState b){
    int x=29, y=5;
    DrawNumber(b.hole[1][6],x,y);
    gotoxy(x+10,y+3);
    cputs("ﬂﬂﬂ");
    DrawNumber(b.hole[0][6],x+16,y);
}

void UpdateBoard(int beans, int player, int pot) {
    DrawNumber(beans,SetX(player,pot),SetY(player,pot));
    ShowPointer(player,pot);
    delay(300);
}

void Zero(int x,int y) {
   int i,j;
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("€ €"); gotoxy(x,y+2);
    cputs("€ €"); gotoxy(x,y+3);
    cputs("€ €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void One(int x, int y){
    int i;
    gotoxy(x,y);
    cputs(" € "); gotoxy(x,y+1);
    cputs("ﬂ€ "); gotoxy(x,y+2);
    cputs(" € "); gotoxy(x,y+3);
    cputs(" € "); gotoxy(x,y+4);
    cputs("‹€‹");
}

void Two(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("  €"); gotoxy(x,y+2);
    cputs("‹‹€"); gotoxy(x,y+3);
    cputs("€  "); gotoxy(x,y+4);
    cputs("€‹‹");

}

void Three(int x, int y) {
    gotoxy(x,y);
    cputs("ﬂﬂ€"); gotoxy(x,y+1);
    cputs(" ‹€"); gotoxy(x,y+2);
    cputs("  €"); gotoxy(x,y+3);
    cputs("  €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void Four(int x, int y) {
    gotoxy(x,y);
    cputs("‹ €"); gotoxy(x,y+1);
    cputs("€ €"); gotoxy(x,y+2);
    cputs("ﬂﬂ€"); gotoxy(x,y+3);
    cputs("  €"); gotoxy(x,y+4);
    cputs("  €");
}

void Five(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂﬂ"); gotoxy(x,y+1);
    cputs("€  "); gotoxy(x,y+2);
    cputs("€‹‹"); gotoxy(x,y+3);
    cputs("  €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void Six(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("€  "); gotoxy(x,y+2);
    cputs("€ﬂ€"); gotoxy(x,y+3);
    cputs("€ €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void Seven(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("  €"); gotoxy(x,y+2);
    cputs(" ﬂ€"); gotoxy(x,y+3);
    cputs("  €"); gotoxy(x,y+4);
    cputs("  €");
}

void Eight(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("€ €"); gotoxy(x,y+2);
    cputs("€ﬂ€"); gotoxy(x,y+3);
    cputs("€ €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void Nine(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("€ €"); gotoxy(x,y+2);
    cputs("ﬂﬂ€"); gotoxy(x,y+3);
    cputs("  €"); gotoxy(x,y+4);
    cputs("€‹€");
}

void What(int x, int y) {
    gotoxy(x,y);
    cputs("€ﬂ€"); gotoxy(x,y+1);
    cputs("ﬂ €"); gotoxy(x,y+2);
    cputs(" €ﬂ"); gotoxy(x,y+3);
    cputs(" ﬂ "); gotoxy(x,y+4);
    cputs(" ‹ ");
}

void DrawDigit(int number, int x, int y) {
    if (number>=0 && number<10) {
	switch (number){
	    case 0: Zero(x,y); break;
	    case 1: One(x,y); break;
	    case 2: Two(x,y); break;
	    case 3: Three(x,y); break;
	    case 4: Four(x,y); break;
	    case 5: Five(x,y); break;
	    case 6: Six(x,y); break;
	    case 7: Seven(x,y); break;
	    case 8: Eight(x,y); break;
	    case 9: Nine(x,y);
	}
    } else What(x,y);
}

void DrawNumber(int number, int x, int y) {
    if (number>=0 && number<100){
	if (number<10)
	   Zero(x,y);
	else DrawDigit(((number-(number%10))/10),x,y);
	DrawDigit(number%10,x+4,y);
    }
}

void DisplayBoard(struct GameState b) {
    int i,j;
    gotoxy(SetX(0,6)+2,SetY(0,6)+6);
    cputs("YOU");
    gotoxy(SetX(1,6)+2,SetY(1,6)+6);
    cputs("CPU");
    for (i=0;i<2;i++){ //Draw initial board
	for (j=0;j<7;j++) {
	    DrawNumber(b.hole[i][j],SetX(i,j),SetY(i,j));
	}
    }

    for (i=5;i>=0;i--){ //Draw labels
	gotoxy(SetX(1,i)+3,SetY(1,i)-2);
	cprintf("%d",i+1); //add +1
    }
    for (i=0;i<6;i++){
	gotoxy(SetX(0,i)+3,SetY(0,i)+7);
	cprintf("%d",i+1); //add +1
    }
}

int SetX(int player, int pot) {
    if (player==1)
	if (pot==6) return 4;
	else return (5-pot)*10+12;
    else if (pot==6)
	return 70;
    else return pot*10+12;
}

int SetY(int player, int pot) {
    if (player==1)
	if (pot==6) return 10;
	else return 3;
    else if (pot==6) return 10;
	else return 17;
}

void ShowPointer(int player, int pot) {
    int x,y;

    //erase last position
    if (lastpointer[0]!=-1){
	x=SetX(lastpointer[0],lastpointer[1]);
	y=SetY(lastpointer[0],lastpointer[1]);
	if (lastpointer[1]==6) {
	    if (lastpointer[0]==0) {
		x-=4;
		gotoxy(x,y+1);
		cputs("   "); gotoxy(x,y+2);
		cputs("   "); gotoxy(x,y+3);
		cputs("   ");
	    } else {
		x+=8;
		gotoxy(x,y+1);
		cputs("   "); gotoxy(x,y+2);
		cputs("   "); gotoxy(x,y+3);
		cputs("   ");
	    }
	} else {
	    x+=2;
	    if (lastpointer[0]==0){
		y-=2;
		gotoxy(x,y);
		cputs("   ");
	    } else {
		y+=6;
		gotoxy(x,y);
		cputs("   ");
	    }
	}
    }

    lastpointer[0] = player;
    lastpointer[1] = pot;

    x=SetX(player,pot);
    y=SetY(player,pot);

    if (pot==6) {
	if (player==0) {
	    x-=4;
	    gotoxy(x,y+1);
	    cputs("‹  "); gotoxy(x,y+2);
	    cputs(" €˛"); gotoxy(x,y+3);
	    cputs("ﬂ  "); gotoxy(x,y+2);
	} else {
	    x+=8;
	    gotoxy(x,y+1);
	    cputs("  ‹"); gotoxy(x,y+2);
	    cputs("˛€ "); gotoxy(x,y+3);
	    cputs("  ﬂ"); gotoxy(x+2,y+2);
	}
    }else {
	x+=2;
	if (player==0){
	    y-=2;
	    gotoxy(x,y);
	    cputs("ﬂ‹ﬂ"); gotoxy(x+1,y);
	} else {
	    y+=6;
	    gotoxy(x,y);
	    cputs("‹ﬂ‹"); gotoxy(x+1,y);
	}
    }
}

void YouLose(struct GameState b) {
    int x=23,y=13;
    ShowScore(b);
    textcolor(RED);
    gotoxy(x,y);
    cputs("‹ € €ﬂ‹ € €   €   €ﬂ‹ ‹ﬂ‹ ‹ﬂ‹  € € €"); gotoxy(x,y+1);
    cputs("€ € € € € €   €   € € €   € €  € € €"); gotoxy(x,y+2);
    cputs("ﬂﬂ€ € € € €   €   € €  ﬂ‹ €‹€  € € €"); gotoxy(x,y+3);
    cputs("  € € € € €   €   € €   € €    ﬂ ﬂ ﬂ"); gotoxy(x,y+4);
    cputs("‹‹€ ﬂ‹€ ﬂ‹€   €‹‹ ﬂ‹€ ﬂ‹ﬂ ﬂ‹ﬂ  ‹ ‹ ‹"); gotoxy(x,y+6);
    cputs("    Better luck next time, loser.");
    textcolor(LIGHTGRAY);
    getch();
}

void YouWin(struct GameState b) {
    int x=19,y=13;
    ShowScore(b);
    textcolor(GREEN);
    gotoxy(x,y);
    cputs("‹ € €ﬂ‹ € €   €   € ﬂ €‹ﬂ‹   ‹€‹ ‹€‹   € € €"); gotoxy(x,y+1);
    cputs("€ € € € € €   €   € ‹ €  €   €€€€€ﬂ€   € € €"); gotoxy(x,y+2);
    cputs("ﬂﬂ€ € € € €   € € € € €  €   ﬂ€€€‹€ﬂ   € € €"); gotoxy(x,y+3);
    cputs("  € € € € €   € € € € €  €    ﬂ€€€ﬂ    ﬂ ﬂ ﬂ"); gotoxy(x,y+4);
    cputs("‹‹€ ﬂ‹€ ﬂ‹€   ﬂ‹ﬂ‹ﬂ € €  €     ﬂ€ﬂ     ‹ ‹ ‹"); gotoxy(x,y+6);
    cputs("    Wow! You're so good! Amazing! Hurray!");
    textcolor(LIGHTGRAY);
    getch();
}

void NoWinner (struct GameState b) {
    int x=17,y=13;
    ShowScore(b);
    gotoxy(x,y);
    cputs("ﬂ€ﬂ ﬂ€ﬂ ﬂ€ ‹ﬂ‹   ‹ﬂ‹   €ﬂ‹ €ﬂ‹ ‹ﬂ‹ €   €  € € €"); gotoxy(x,y+1);
    cputs(" €   €  ﬂ  €     € €   € € € € € € €   €  € € €"); gotoxy(x,y+2);
    cputs(" €   €      ﬂ‹   € €   € € €ﬂ‹ € € € ‹ €  € € €"); gotoxy(x,y+3);
    cputs(" €   €     ‹ €   €‹€   € € € € €‹€ € € €  ﬂ ﬂ ﬂ"); gotoxy(x,y+4);
    cputs("‹€‹  €     ﬂ‹ﬂ   € €   €‹ﬂ € € € € ﬂ‹ﬂ‹ﬂ  ‹ ‹ ‹"); gotoxy(x+1,y+6);
    cputs("It's a good game, but we're still both losers.");
    getch();
}

int DetermineWinner(struct GameState *b) {
    int i,j;
    for (i=0;i<2;i++)
       for (j=0;j<6;j++)
	   b->hole[i][6]+=b->hole[i][j];

    for(i=0;i<6;i++) b->hole[1][i]=0;
    for(i=0;i<6;i++) b->hole[0][i]=0;

    if (b->hole[0][6]>b->hole[1][6]) return WIN;
    if (b->hole[0][6]<b->hole[1][6]) return LOSE;
    if (b->hole[0][6]==b->hole[1][6]) return TIE;
    return ERROR;
}

void DeclareDecision(struct GameState b, int result){
    DisplayBoard(b);
    delay(2500);
    clrscr();
    if (result==WIN) YouWin(b);
    if (result==LOSE) YouLose(b);
    if (result==TIE) NoWinner(b);
}
void Intro(){
    int x=18, y=8;
    clrscr();
    gotoxy(x,y+1);
    cputs("€ﬂﬂﬂ‹ ‹ﬂﬂﬂ‹ €‹ﬂﬂ‹ ﬂﬂ€ﬂﬂ €   € €‹ﬂﬂ‹‹ﬂﬂ‹ ﬂ   "); gotoxy(x,y+2);
    cputs("€   € €   € €   €   €   €   € €   €   € €   "); gotoxy(x,y+2);
    cputs("€ﬂﬂﬂ‹ €   € €   €   €   €   € €   €   € €   "); gotoxy(x,y+3);
    cputs("€   € €‹‹‹€ €   €   €   €   € €   €   € €   "); gotoxy(x,y+4);
    cputs("€‹‹‹ﬂ €   € €   €   €   ﬂ‹‹‹ﬂ €   €   € € V2");
    gotoxy(x+10,y+7);
    cputs("Press Enter to Continue.");
    gotoxy(x+11,y+8);
    cputs("H for Help. Q to Quit.");
}
