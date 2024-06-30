#include "../game/gamelogic.h"
#include "../vcfsolver/VCFsolver.h"
#include "../forbiddenPoint/ForbiddenPointFinder.h"

/*
 * gamelogic.cpp
 * Logics of game rules
 * Some other game logics are in board.h/cpp
 *
 * Gomoku as a representive
 */

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;


static int connectionLengthOneDirection(
  const Board& board,
  Player pla,
  bool isSixWin,
  Loc loc,
  short adj,
  bool& isLife) 
{
  Loc tmploc = loc;
  int conNum = 0;
  isLife = false;
  while(1) {
    tmploc += adj;
    if(!board.isOnBoard(tmploc))
      break;
    if(board.colors[tmploc] == pla)
      conNum++;
    else if(board.colors[tmploc] == C_EMPTY) {
      isLife = true;

      if(!isSixWin) {
        tmploc += adj;
        if(board.isOnBoard(tmploc) && board.colors[tmploc] == pla)
          isLife = false;
      }
      break;
    } 
    else
      break;
  }
  return conNum;
}

static GameLogic::MovePriority getMovePriorityOneDirectionAssumeLegal(
  const Board& board,
  Player pla,
  bool isSixWinMe,
  bool isSixWinOpp,
  Loc loc,
  int adj)  {
  Player opp = getOpp(pla);
  bool isMyLife1, isMyLife2, isOppLife1, isOppLife2;
  int myConNum = connectionLengthOneDirection(board, pla, isSixWinMe, loc, adj, isMyLife1) +
                 connectionLengthOneDirection(board, pla, isSixWinMe, loc, -adj, isMyLife2) + 1;
  int oppConNum = connectionLengthOneDirection(board, opp, isSixWinOpp, loc, adj, isOppLife1) +
                  connectionLengthOneDirection(board, opp, isSixWinOpp, loc, -adj, isOppLife2) + 1;
  if(myConNum == 5 || (myConNum > 5 && isSixWinMe))
    return GameLogic::MP_SUDDEN_WIN;

  if(oppConNum == 5 || (oppConNum > 5 && isSixWinOpp))
    return GameLogic::MP_ONLY_NONLOSE_MOVES;

  if(myConNum == 4 && isMyLife1 && isMyLife2)
    return GameLogic::MP_WINNING;

  return GameLogic::MP_NORMAL;
}

GameLogic::MovePriority GameLogic::getMovePriorityAssumeLegal(const Board& board, const BoardHistory& hist, Player pla, Loc loc) {
  if(loc == Board::PASS_LOC)
    return MP_NORMAL;
  MovePriority MP = MP_NORMAL;

  bool isSixWinMe = hist.rules.basicRule == Rules::BASICRULE_FREESTYLE  ? true
                    : hist.rules.basicRule == Rules::BASICRULE_STANDARD ? false
                    : hist.rules.basicRule == Rules::BASICRULE_RENJU    ? (pla == C_WHITE)
                                                                  : true;

  bool isSixWinOpp = hist.rules.basicRule == Rules::BASICRULE_FREESTYLE ? true
                     : hist.rules.basicRule == Rules::BASICRULE_STANDARD ? false
                     : hist.rules.basicRule == Rules::BASICRULE_RENJU    ? (pla == C_BLACK)
                                                                   : true;

  int adjs[4] = {1, (board.x_size + 1), (board.x_size + 1) + 1, (board.x_size + 1) - 1};// +x +y +x+y -x+y
  for(int i = 0; i < 4; i++) {
    MovePriority tmpMP = getMovePriorityOneDirectionAssumeLegal(board, pla, isSixWinMe, isSixWinOpp, loc, adjs[i]);
    if(tmpMP < MP)
      MP = tmpMP;
  }

  return MP;
}

GameLogic::MovePriority GameLogic::getMovePriority(const Board& board, const BoardHistory& hist, Player pla, Loc loc) {
  if(loc == Board::PASS_LOC)
    return MP_NORMAL;
  if(!board.isLegal(loc, pla))
    return MP_ILLEGAL;
  MovePriority MP = getMovePriorityAssumeLegal(board, hist, pla, loc);

  if(MP == MP_MYLIFEFOUR && hist.rules.basicRule == Rules::BASICRULE_RENJU && pla == C_BLACK && board.isForbidden(loc))
    return MP_NORMAL;
  return MP;
}

std::vector<Loc> GameLogic::getFourAttackLocs(const Board& board, const Rules& rules, Player pla) {
  vector<Loc> fourLocs;
  Board boardcopy = board;
  bool isSixWinMe = rules.basicRule == Rules::BASICRULE_FREESTYLE  ? true
                    : rules.basicRule == Rules::BASICRULE_STANDARD ? false
                    : rules.basicRule == Rules::BASICRULE_RENJU    ? (pla == C_WHITE)
                                                                        : true;
  bool isSixWinOpp = rules.basicRule == Rules::BASICRULE_FREESTYLE  ? true
                     : rules.basicRule == Rules::BASICRULE_STANDARD ? false
                     : rules.basicRule == Rules::BASICRULE_RENJU    ? (pla == C_BLACK)
                                                                         : true;  // not used
  for(int x0 = 0; x0 < board.x_size; x0++)
    for(int y0 = 0; y0 < board.y_size; y0++)
    {
      Loc loc0 = Location::getLoc(x0, y0, board.x_size);
      if(board.colors[loc0] != C_EMPTY)
        continue;
      bool isFour = false;

      boardcopy.colors[loc0] = pla;

      bool isLife;//not used
      int adjs[4] = {1, (board.x_size + 1), (board.x_size + 1) + 1, (board.x_size + 1) - 1};  // +x +y +x+y -x+y
      for(int i = 0; i < 4; i++) {
        Loc loc1 = loc0;
        for (int j = 0; j < 4; j++)
        {
          loc1 += adjs[i];
          if(!boardcopy.isOnBoard(loc1))
            break;
          if(boardcopy.colors[loc1] != C_EMPTY)
            continue;
          int length = connectionLengthOneDirection(boardcopy, pla, isSixWinMe, loc1, adjs[i], isLife) +
                       connectionLengthOneDirection(boardcopy, pla, isSixWinMe, loc1, -adjs[i], isLife) +
                       1;
          if(length == 5 || (length > 5 && isSixWinMe))
            isFour = true;
        }
        loc1 = loc0;
        for(int j = 0; j < 4; j++) {
          loc1 -= adjs[i];
          if(!boardcopy.isOnBoard(loc1))
            break;
          if(boardcopy.colors[loc1] != C_EMPTY)
            continue;
          int length = connectionLengthOneDirection(boardcopy, pla, isSixWinMe, loc1, adjs[i], isLife) +
                       connectionLengthOneDirection(boardcopy, pla, isSixWinMe, loc1, -adjs[i], isLife) + 1;
          if(length == 5 || (length > 5 && isSixWinMe))
            isFour = true;
        }
      }
      if(isFour)
        fourLocs.push_back(loc0);
      boardcopy.colors[loc0] = C_EMPTY;
    }


  return fourLocs;
}

bool Board::isForbidden(Loc loc) const {
  if(loc == PASS_LOC) {
    return false;
  }
  if(!(loc >= 0 && loc < MAX_ARR_SIZE && (colors[loc] == C_EMPTY)))
    return false;
  if(x_size == y_size) {
    int x = Location::getX(loc, x_size);
    int y = Location::getY(loc, x_size);
    int nearbyBlack = 0;
    // x++; y++;
    for(int i = std::max(x - 2, 0); i <= std::min(x + 2, x_size - 1); i++)
      for(int j = std::max(y - 2, 0); j <= std::min(y + 2, y_size - 1); j++) {
        int xd = i - x;
        int yd = j - y;
        xd = xd > 0 ? xd : -xd;
        yd = yd > 0 ? yd : -yd;
        if(((xd + yd) != 3) && (colors[Location::getLoc(i, j, x_size)] == C_BLACK))
          nearbyBlack++;
      }

    if(nearbyBlack >= 2) {
      CForbiddenPointFinder fpf(x_size);
      for(int x = 0; x < x_size; x++)
        for(int y = 0; y < y_size; y++) {
          fpf.SetStone(x, y, colors[Location::getLoc(x, y, x_size)]);
        }
      if(fpf.isForbiddenNoNearbyCheck(Location::getX(loc, x_size), Location::getY(loc, x_size)))
        return true;
    }
  }
  return false;
}
bool Board::isForbiddenAlreadyPlayed(Loc loc) const {
  if(loc == PASS_LOC) {
    return false;
  }
  if(!(loc >= 0 && loc < MAX_ARR_SIZE && (colors[loc] == C_BLACK)))
    return false;
  if(x_size == y_size) {
    int x = Location::getX(loc, x_size);
    int y = Location::getY(loc, x_size);
    int nearbyBlack = 0;
    // x++; y++;
    for(int i = std::max(x - 2, 0); i <= std::min(x + 2, x_size - 1); i++)
      for(int j = std::max(y - 2, 0); j <= std::min(y + 2, y_size - 1); j++) {
        int xd = i - x;
        int yd = j - y;
        xd = xd > 0 ? xd : -xd;
        yd = yd > 0 ? yd : -yd;
        if(((xd + yd) != 3) && (colors[Location::getLoc(i, j, x_size)] == C_BLACK))
          nearbyBlack++;
      }

    if(nearbyBlack >= 3) {
      CForbiddenPointFinder fpf(x_size);
      for(int x = 0; x < x_size; x++)
        for(int y = 0; y < y_size; y++) {
          fpf.SetStone(x, y, colors[Location::getLoc(x, y, x_size)]);
        }
      fpf.SetStone(Location::getX(loc, x_size), Location::getY(loc, x_size), C_EMPTY);
      if(fpf.isForbiddenNoNearbyCheck(Location::getX(loc, x_size), Location::getY(loc, x_size)))
        return true;
    }
  }
  return false;
}




Color GameLogic::checkWinnerAfterPlayed(
  const Board& board,
  const BoardHistory& hist,
  Player pla,
  Loc loc) {



  
  if((hist.rules.maxMoves != 0 || hist.rules.VCNRule != Rules::VCNRULE_NOVC) && hist.rules.firstPassWin) {
    throw StringError("GameLogic::checkWinnerAfterPlayed This rule is not supported");
  }

  Player opp = getOpp(pla);
  int myPassNum = pla == C_BLACK ? board.blackPassNum : board.whitePassNum;
  int oppPassNum = pla == C_WHITE ? board.blackPassNum : board.whitePassNum;

  if(loc == Board::PASS_LOC) {
    if(hist.rules.VCNRule == Rules::VCNRULE_NOVC) {
      if(oppPassNum > 0) {
        if(!hist.rules.firstPassWin)  // �������
        {
          return C_EMPTY;
        } else  // �Է���pass
        {
          return opp;
        }
      }
    } else {
      Color VCside = hist.rules.vcSide();
      int VClevel = hist.rules.vcLevel();

      if(VCside == pla)  // VCN��������pass
      {
        return opp;
      } else  // pass�����㹻����ʤ
      {
        if(myPassNum >= 6 - VClevel) {
          return pla;
        }
      }
    }
  }

  if(hist.rules.basicRule == Rules::BASICRULE_RENJU && pla == C_BLACK)  // �����ж�
  {
    if(board.isForbiddenAlreadyPlayed(loc)) {
      return opp;
    }
  }

  // �����ж�
  if(getMovePriorityAssumeLegal(board, hist, pla, loc) == MP_SUDDEN_WIN) {
    return pla;
  }

  // maxmoves�ж�
  if(hist.rules.maxMoves != 0 && board.movenum >= hist.rules.maxMoves) {
    if(hist.rules.VCNRule == Rules::VCNRULE_NOVC) {
      return C_EMPTY;
    } else  // �����н�������
    {
      static_assert(Rules::VCNRULE_VC1_W == Rules::VCNRULE_VC1_B + 10, "Ensure VCNRule%10==N, VCNRule/10+1==color");
      Color VCside = hist.rules.vcSide();
      return getOpp(VCside);
    }
  }

  return C_WALL;
}

GameLogic::ResultsBeforeNN::ResultsBeforeNN() {
  inited = false;
  calculatedVCF = false;
  winner = C_WALL;
  myOnlyLoc = Board::NULL_LOC;
  myVCFresult = 0;
  oppVCFresult = 0;
}

void GameLogic::ResultsBeforeNN::init(const Board& board, const BoardHistory& hist, Color nextPlayer, bool hasVCF) {
  if(hist.rules.VCNRule != Rules::VCNRULE_NOVC && hist.rules.maxMoves != 0)
    throw StringError("ResultBeforeNN::init() can not support VCN and maxMoves simutaneously");
  bool willCalculateVCF = hasVCF && hist.rules.maxMoves == 0;
  if(inited && (calculatedVCF || (!willCalculateVCF)))
    return;
  inited = true;

  Color opp = getOpp(nextPlayer);

  // check five and four
  bool oppHasFour = false;
  bool IHaveLifeFour = false;
  Loc myLifeFourLoc = Board::NULL_LOC;
  for(int x = 0; x < board.x_size; x++)
    for(int y = 0; y < board.y_size; y++) {
      Loc loc = Location::getLoc(x, y, board.x_size);
      MovePriority mp = getMovePriority(board, hist, nextPlayer, loc);
      if(mp == MP_FIVE) {
        winner = nextPlayer;
        myOnlyLoc = loc;
        return;
      } else if(mp == MP_OPPOFOUR) {
        oppHasFour = true;
        myOnlyLoc = loc;
      } else if(mp == MP_MYLIFEFOUR) {
        IHaveLifeFour = true;
        myLifeFourLoc = loc;
      }
    }

  if(hist.rules.VCNRule != Rules::VCNRULE_NOVC) {
    int vcLevel = hist.rules.vcLevel() + board.blackPassNum + board.whitePassNum;
    
    Color vcSide = hist.rules.vcSide();
    if(vcSide == nextPlayer) {
      if(vcLevel == 5) {
        winner = opp;
        myOnlyLoc = Board::NULL_LOC;
        return;
      }
    } else if(vcSide == opp) {
      if(vcLevel == 5) {
        winner = nextPlayer;
        myOnlyLoc = Board::PASS_LOC;
        return;
      } else if(vcLevel == 4) {
        if(!oppHasFour) {
          winner = nextPlayer;
          myOnlyLoc = Board::PASS_LOC;
          return;
        }
      }
    }
  }

  // opp has four
  if(oppHasFour)
    return;

  // I have life four, opp has no four
  if(IHaveLifeFour && (!oppHasFour)) {
    int remainMovenum = hist.rules.maxMoves == 0 ? 10000 : hist.rules.maxMoves - board.movenum;
    if(remainMovenum >= 3)
      winner = nextPlayer;
    myOnlyLoc = myLifeFourLoc;
    return;
  }

  if(!willCalculateVCF)
    return;

  // check VCF
  calculatedVCF = true;
  uint16_t oppvcfloc;
  VCFsolver::run(board, hist.rules, getOpp(nextPlayer), oppVCFresult, oppvcfloc);

  uint16_t myvcfloc;
  VCFsolver::run(board, hist.rules, nextPlayer, myVCFresult, myvcfloc);
  if(myVCFresult == 1) {
    winner = nextPlayer;
    myOnlyLoc = myvcfloc;
    return;
  }
}
