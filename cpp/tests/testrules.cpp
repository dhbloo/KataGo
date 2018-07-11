#include "../tests/tests.h"
using namespace TestCommon;

void Tests::runRulesTests() {
  cout << "Running rules tests" << endl;
  ostringstream out;

  {
    const char* name = "Basic area rules";
    Board board = parseBoard(4,4,R"%%(
....
....
....
....
)%%");
    Rules rules;
    rules.koRule = Rules::KO_POSITIONAL;
    rules.scoringRule = Rules::SCORING_AREA;
    rules.komi = 0.5f;
    rules.multiStoneSuicideLegal = true;
    BoardHistory hist(board,P_BLACK,rules);

    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,1,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,2,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,2,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,1,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,3,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,3,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_WHITE);
    testAssert(hist.finalWhiteMinusBlackScore == 0.5f);
    //Resurrecting the board after game over with another pass
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_WHITE);
    testAssert(hist.finalWhiteMinusBlackScore == 0.5f);
    //And then some real moves followed by more passes
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_WHITE, NULL);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_WHITE);
    testAssert(hist.finalWhiteMinusBlackScore == 0.5f);
    out << board << endl;
    string expected = R"%%(
HASH: 551911C639136FD87CFD8C126ABC2737
. X O .
. X O .
. X O O
. X O .
)%%";
    expect(name,out,expected);
    out.str("");
    out.clear();
  }

  {
    const char* name = "Basic territory rules";
    Board board = parseBoard(4,4,R"%%(
....
....
....
....
)%%");
    Rules rules;
    rules.koRule = Rules::KO_POSITIONAL;
    rules.scoringRule = Rules::SCORING_TERRITORY;
    rules.komi = 0.5f;
    rules.multiStoneSuicideLegal = true;
    BoardHistory hist(board,P_BLACK,rules);

    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,1,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,2,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,2,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,1,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,3,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,3,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.encorePhase == 0);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    testAssert(hist.encorePhase == 1);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.encorePhase == 1);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    testAssert(hist.encorePhase == 2);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.encorePhase == 2);
    testAssert(hist.isGameOver() == false);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    testAssert(hist.encorePhase == 2);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_WHITE);
    testAssert(hist.finalWhiteMinusBlackScore == 3.5f);
    out << board << endl;

    //Resurrecting the board after pass to have black throw in a dead stone, since second encore, should make no difference
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,1,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    testAssert(hist.encorePhase == 2);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_WHITE);
    testAssert(hist.finalWhiteMinusBlackScore == 3.5f);
    out << board << endl;

    //Resurrecting again to have black solidfy his group and prove it pass-alive
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,0,board.x_size), P_WHITE, NULL);
    hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,1,board.x_size), P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
    hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
    //White claimed 3 points pre-second-encore, while black waited until second encore, so black gets 4 points and wins by 0.5.
    testAssert(hist.encorePhase == 2);
    testAssert(hist.isGameOver() == true);
    testAssert(hist.winner == P_BLACK);
    testAssert(hist.finalWhiteMinusBlackScore == -0.5f);
    out << board << endl;

    string expected = R"%%(
HASH: 551911C639136FD87CFD8C126ABC2737
. X O .
. X O .
. X O O
. X O .


HASH: 4234472D4CF6700889EE541B518C2FF9
. X O .
. X O X
. X O O
. X O .


HASH: FAE9F3EAAF790C5CF1EC62AFDD264F77
. X O O
X X O .
. X O O
. X O .
)%%";
    expect(name,out,expected);
    out.str("");
    out.clear();
  }


  //Basic ko rule testing
  {
    Board baseBoard = parseBoard(6,5,R"%%(
.o.xxo
oxxxo.
o.x.oo
xxxoo.
oooo.o
)%%");

    Rules baseRules;
    baseRules.koRule = Rules::KO_POSITIONAL;
    baseRules.scoringRule = Rules::SCORING_TERRITORY;
    baseRules.komi = 0.5f;
    baseRules.multiStoneSuicideLegal = false;

    auto printIllegalMoves = [](ostream& o, const Board& board, const BoardHistory& hist, Player pla) {
      for(int y = 0; y<board.y_size; y++) {
        for(int x = 0; x<board.x_size; x++) {
          Loc loc = Location::getLoc(x,y,board.x_size);
          if(board.colors[loc] == C_EMPTY && !board.isIllegalSuicide(loc,pla,hist.rules.multiStoneSuicideLegal) && !hist.isLegal(board,loc,pla)) {
            o << "Illegal: " << Location::toString(loc,board.x_size) << " " << getCharOfColor(pla) << endl;
          }
        }
      }
    };

    {
      const char* name = "Simple ko rules";
      Board board(baseBoard);
      Rules rules(baseRules);
      rules.koRule = Rules::KO_SIMPLE;
      BoardHistory hist(board,P_BLACK,rules);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,1,board.x_size), P_BLACK, NULL);
      out << "After black ko capture:" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
      out << "After black ko capture and one pass:" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      testAssert(hist.encorePhase == 0);
      testAssert(hist.isGameOver() == false);
      out << "After black ko capture and two passes:" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,0,board.x_size), P_WHITE, NULL);
      out << "White recapture:" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_BLACK, NULL);

      out << "Beginning sending two returning one cycle" << endl;
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      printIllegalMoves(out,board,hist,P_BLACK);
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      printIllegalMoves(out,board,hist,P_WHITE);
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_WHITE, NULL);
      printIllegalMoves(out,board,hist,P_BLACK);
      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      printIllegalMoves(out,board,hist,P_WHITE);
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      printIllegalMoves(out,board,hist,P_BLACK);
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      printIllegalMoves(out,board,hist,P_WHITE);
      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_WHITE, NULL);
      printIllegalMoves(out,board,hist,P_BLACK);
      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      printIllegalMoves(out,board,hist,P_WHITE);
      testAssert(hist.encorePhase == 0);
      testAssert(hist.isGameOver() == false);

      string expected = R"%%(
After black ko capture:
Illegal: (5,0) O
After black ko capture and one pass:
After black ko capture and two passes:
White recapture:
Illegal: (5,1) X
Beginning sending two returning one cycle
)%%";
      expect(name,out,expected);
      out.str("");
      out.clear();
    }

    {
      const char* name = "Positional ko rules";
      Board board(baseBoard);
      Rules rules(baseRules);
      rules.koRule = Rules::KO_POSITIONAL;
      BoardHistory hist(board,P_BLACK,rules);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,1,board.x_size), P_BLACK, NULL);
      out << "After black ko capture:" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
      out << "After black ko capture and one pass:" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      //On tmp board and hist, verify that the main phase ends if black passes now
      Board tmpboard(board);
      BoardHistory tmphist(hist);
      tmphist.makeBoardMoveAssumeLegal(tmpboard, Board::PASS_LOC, P_BLACK, NULL);
      testAssert(tmphist.encorePhase == 1);
      testAssert(tmphist.isGameOver() == false);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_BLACK, NULL);
      out << "Beginning sending two returning one cycle" << endl;

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends two?" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      out << "Can white recapture?" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,0,board.x_size), P_WHITE, NULL);
      out << "After white recaptures the other ko instead" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white recaptures the other ko instead and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_WHITE, NULL);
      out << "After white now returns 1" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white now returns 1 and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends 2 again" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);
      testAssert(hist.encorePhase == 0);
      testAssert(hist.isGameOver() == false);

      string expected = R"%%(
After black ko capture:
Illegal: (5,0) O
After black ko capture and one pass:
Beginning sending two returning one cycle
After white sends two?
Can white recapture?
Illegal: (1,0) O
After white recaptures the other ko instead
Illegal: (5,1) X
After white recaptures the other ko instead and black passes
After white now returns 1
Illegal: (5,1) X
After white now returns 1 and black passes
After white sends 2 again
Illegal: (0,0) X
Illegal: (5,1) X
)%%";
      expect(name,out,expected);
      out.str("");
      out.clear();
    }

    {
      const char* name = "Situational ko rules";
      Board board(baseBoard);
      Rules rules(baseRules);
      rules.koRule = Rules::KO_SITUATIONAL;
      BoardHistory hist(board,P_BLACK,rules);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,1,board.x_size), P_BLACK, NULL);
      out << "After black ko capture:" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
      out << "After black ko capture and one pass:" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      //On tmp board and hist, verify that the main phase ends if black passes now
      Board tmpboard(board);
      BoardHistory tmphist(hist);
      tmphist.makeBoardMoveAssumeLegal(tmpboard, Board::PASS_LOC, P_BLACK, NULL);
      testAssert(tmphist.encorePhase == 1);
      testAssert(tmphist.isGameOver() == false);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_BLACK, NULL);
      out << "Beginning sending two returning one cycle" << endl;

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends two?" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      out << "Can white recapture?" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,0,board.x_size), P_WHITE, NULL);
      out << "After white recaptures the other ko instead" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white recaptures the other ko instead and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_WHITE, NULL);
      out << "After white now returns 1" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white now returns 1 and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends 2 again" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);
      testAssert(hist.encorePhase == 0);
      testAssert(hist.isGameOver() == false);

      string expected = R"%%(
After black ko capture:
Illegal: (5,0) O
After black ko capture and one pass:
Beginning sending two returning one cycle
After white sends two?
Can white recapture?
After white recaptures the other ko instead
Illegal: (5,1) X
After white recaptures the other ko instead and black passes
After white now returns 1
Illegal: (5,1) X
After white now returns 1 and black passes
After white sends 2 again
Illegal: (0,0) X
)%%";
      expect(name,out,expected);
      out.str("");
      out.clear();
    }

    {
      const char* name = "Spight ko rules";
      Board board(baseBoard);
      Rules rules(baseRules);
      rules.koRule = Rules::KO_SPIGHT;
      BoardHistory hist(board,P_BLACK,rules);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,1,board.x_size), P_BLACK, NULL);
      out << "After black ko capture:" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
      out << "After black ko capture and one pass:" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      //On tmp board and hist, verify that the main phase does not end if black passes now
      Board tmpboard(board);
      BoardHistory tmphist(hist);
      tmphist.makeBoardMoveAssumeLegal(tmpboard, Board::PASS_LOC, P_BLACK, NULL);
      testAssert(tmphist.encorePhase == 0);
      testAssert(tmphist.isGameOver() == false);
      out << "If black were to pass as well??" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(3,2,board.x_size), P_BLACK, NULL);
      out << "Beginning sending two returning one cycle" << endl;

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends two?" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      out << "Can white recapture?" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(5,0,board.x_size), P_WHITE, NULL);
      out << "After white recaptures the other ko instead" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white recaptures the other ko instead and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(1,0,board.x_size), P_WHITE, NULL);
      out << "After white now returns 1" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After white now returns 1 and black passes" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(2,0,board.x_size), P_WHITE, NULL);
      out << "After white sends 2 again" << endl;
      printIllegalMoves(out,board,hist,P_BLACK);

      hist.makeBoardMoveAssumeLegal(board, Location::getLoc(0,0,board.x_size), P_BLACK, NULL);
      out << "Can white recapture?" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);

      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_WHITE, NULL);
      out << "After pass" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);
      testAssert(hist.encorePhase == 0);
      testAssert(hist.isGameOver() == false);

      //This is actually black's second pass in this position!
      hist.makeBoardMoveAssumeLegal(board, Board::PASS_LOC, P_BLACK, NULL);
      out << "After pass" << endl;
      printIllegalMoves(out,board,hist,P_WHITE);
      testAssert(hist.encorePhase == 1);
      testAssert(hist.isGameOver() == false);

      string expected = R"%%(
After black ko capture:
Illegal: (5,0) O
After black ko capture and one pass:
If black were to pass as well??
Beginning sending two returning one cycle
After white sends two?
Can white recapture?
Illegal: (1,0) O
After white recaptures the other ko instead
Illegal: (5,1) X
After white recaptures the other ko instead and black passes
After white now returns 1
After white now returns 1 and black passes
After white sends 2 again
Can white recapture?
Illegal: (1,0) O
After pass
After pass
)%%";
      expect(name,out,expected);
      out.str("");
      out.clear();
    }

  }

  //TODO
  //Hash after suicide and superko
  //Try the eternal fight position
  //Try a triple ko
  //Testing the encore - verify that moves in phase 1 are penalized but not phase 2
  //Testing the encore - test out ko marking rules with pass-for-ko, and that they aren't removed if the board changes
  //Testing the encore - test out ko marking going away when the ko vanishes, such as a two-step ko capture (the bottom of molasses)
  //Testing the encore - test out the only-once rules
  //Testing the encore - test out double seki collapse with only-once rules


}