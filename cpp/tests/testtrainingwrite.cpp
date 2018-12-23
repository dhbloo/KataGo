#include "../tests/tests.h"
using namespace TestCommon;

#include "../neuralnet/nneval.h"
#include "../dataio/trainingwrite.h"
#include "../program/play.h"

static NNEvaluator* startNNEval(
  const string& seed, Logger& logger,
  int defaultSymmetry, bool inputsUseNHWC, bool cudaUseNHWC, bool cudaUseFP16
) {
  //Placeholder, doesn't actually do anything since we have debugSkipNeuralNet = true
  string modelFile = "/dev/null";
  int modelFileIdx = 0;
  int maxBatchSize = 16;
  int maxConcurrentEvals = 1024;
  int posLen = NNPos::MAX_BOARD_LEN;
  bool requireExactPosLen = false;
  int nnCacheSizePowerOfTwo = 16;
  int nnMutexPoolSizePowerOfTwo = 12;
  bool debugSkipNeuralNet = true;
  double nnPolicyTemperature = 1.0;
  NNEvaluator* nnEval = new NNEvaluator(
    modelFile,
    modelFileIdx,
    maxBatchSize,
    maxConcurrentEvals,
    posLen,
    requireExactPosLen,
    inputsUseNHWC,
    nnCacheSizePowerOfTwo,
    nnMutexPoolSizePowerOfTwo,
    debugSkipNeuralNet,
    nnPolicyTemperature
  );
  (void)inputsUseNHWC;

  int numNNServerThreadsPerModel = 1;
  bool nnRandomize = false;
  vector<int> cudaGpuIdxByServerThread = {0};

  nnEval->spawnServerThreads(
    numNNServerThreadsPerModel,
    nnRandomize,
    seed,
    defaultSymmetry,
    logger,
    cudaGpuIdxByServerThread,
    cudaUseFP16,
    cudaUseNHWC
  );

  return nnEval;
}

void Tests::runTrainingWriteTests() {
  cout << "Running training write tests" << endl;
  string tensorflowGpuVisibleDeviceList = "";
  double tensorflowPerProcessGpuMemoryFraction = 0.3;
  NeuralNet::globalInitialize(tensorflowGpuVisibleDeviceList,tensorflowPerProcessGpuMemoryFraction);

  int inputsVersion = 3;
  int maxRows = 256;
  int posLen = 5;
  double firstFileMinRandProp = 1.0;
  int debugOnlyWriteEvery = 5;

  ostringstream out;
  Logger logger;
  logger.setLogToStdout(false);
  logger.setLogTime(false);
  logger.addOStream(out);

  auto run = [&](const string& seedBase, const Rules& rules) {
    TrainingDataWriter dataWriter(&out,inputsVersion, maxRows, firstFileMinRandProp, posLen, debugOnlyWriteEvery, seedBase+"dwriter");

    NNEvaluator* nnEval = startNNEval(seedBase+"nneval",logger,0,true,false,false);

    SearchParams params;
    params.maxVisits = 100;

    MatchPairer::BotSpec botSpec;
    botSpec.botIdx = 0;
    botSpec.botName = string("test");
    botSpec.nnEval = nnEval;
    botSpec.baseParams = params;

    Board initialBoard(5,5);
    Player initialPla = P_BLACK;
    int initialEncorePhase = 0;
    BoardHistory initialHist(initialBoard,initialPla,rules,initialEncorePhase);

    int numExtraBlack = 0;
    bool doEndGameIfAllPassAlive = true;
    bool clearBotAfterSearch = true;
    int maxMovesPerGame = 40;
    vector<std::atomic<bool>*> stopConditions;
    FancyModes fancyModes;
    fancyModes.initGamesWithPolicy = true;
    fancyModes.forkSidePositionProb = 0.10;
    bool recordFullData = true;
    Rand rand(seedBase+"play");
    FinishedGameData* gameData = Play::runGame(
      initialBoard,initialPla,initialHist,numExtraBlack,
      botSpec,botSpec,
      seedBase+"search",
      doEndGameIfAllPassAlive, clearBotAfterSearch,
      logger, false, false,
      maxMovesPerGame, stopConditions,
      fancyModes, recordFullData, posLen,
      rand
    );

    out << gameData->startHist.getRecentBoard(0) << endl;
    gameData->endHist.printDebugInfo(out,gameData->endHist.getRecentBoard(0));

    dataWriter.writeGame(*gameData);
    delete gameData;

    dataWriter.flushIfNonempty();

    delete nnEval;
  };

  run("testtrainingwrite-tt",Rules::getTrompTaylorish());

  Rules rules;
  rules.koRule = Rules::KO_SIMPLE;
  rules.scoringRule = Rules::SCORING_TERRITORY;
  rules.multiStoneSuicideLegal = false;
  rules.komi = 5;
  run("testtrainingwrite-jp",rules);

  string expected = R"%%(
HASH: 8333137CA06AB48A180FF32D05FA698B
   A B C D E
 5 . . . . .
 4 . . . . .
 3 . . . . .
 2 . . . . .
 1 . . . X .


HASH: 79ECD036CB88DEA251B85C313D48B785
   A B C D E
 5 X X X . O
 4 X O O O O
 3 X O . O .
 2 O . O O .
 1 O O O . .


Encore phase 0
Rules koPOSITIONALscoreAREAsui1komi7.5
Ko prohib hash 00000000000000000000000000000000
White bonus score 0
Game result 1 White 32.5 0 0
Last moves D1 C4 C1 D3 C3 E5 B2 pass E2 B3 C5 A2 A4 A1 B5 B4 A5 B1 E3 D2 pass E4 E1 D4 C2 C2 A3 C1
binaryInputNCHWPacked
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|u1','fortran_order':False,'shape':(6,22,4)}
FFFFFF80000000000000010000000000000000000000010000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000FFFFFF800000000000000000
FFFFFF80000803000904000000000000080800000104000000000000000000000000000008000000000800000004000000000200010000000000000000000000000000000000000000080300090400000000000000000000
FFFFFF80091400002008930000080000291000000004900000000000000000000000000020000000001000000000100000000000000080002800000000000000000000005000000009140000200893800000000000000000
FFFFFF80640893000B150800000800006F1188000004100000000000000000000000000002000000400000000000080004000000000100006C010800080108002801080000600400E40893800B1508000000000000000000
FFFFFF800B152C00E40A930000098C00EF163300000000000000000000000000000000000000000000002000000200000000040080000000EC0A1300EC0A1300EC000000104040000B152C00E40A93800000000000000000
FFFFFF80E40000000BD56C0000000000E400000000010C000000000000000000000000000000400000004000008000000000008000400000E4000000E4000000E40A138000000000E40000000BDFFF800000000000000000

globalInputNC
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<f4','fortran_order':False,'shape':(6,14)}
0 0 0 0 0 0.5 1 0.5 1 0 0 0 0 0.5
0 0 0 0 0 -0.5 1 0.5 1 0 0 0 0 -0.5
0 0 0 1 0 0.5 1 0.5 1 0 0 0 0 0.5
0 0 0 0 0 -0.5 1 0.5 1 0 0 0 0 -0.5
1 0 0 0 0 0.5 1 0.5 1 0 0 0 1 0.5
0 0 0 0 0 -0.5 1 0.5 1 0 0 0 0 -0.5

policyTargetsNCMove
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<i2','fortran_order':False,'shape':(6,1,26)}
8 0 0 11 2 0 0 13 0 6 11 3 8 0 4 3 9 2 5 0 0 0 7 0 0 7
9 0 5 0 0 2 8 0 0 10 1 0 0 0 6 0 17 0 0 11 12 2 0 0 16 0
12 5 0 7 0 0 3 0 14 2 13 0 0 0 8 16 0 7 0 0 0 7 0 0 0 5
22 0 0 14 0 0 0 0 14 10 0 0 0 0 5 0 0 3 0 0 0 0 0 0 10 21
0 0 0 10 0 0 0 0 0 56 3 0 0 0 0 0 0 30 0 0 0 0 0 0 0 0
0 0 0 9 0 0 0 0 0 0 31 0 0 0 3 0 0 0 0 17 0 0 6 13 17 3

globalTargetsNC
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<f4','fortran_order':False,'shape':(6,54)}
1 0 0 32.5 0.649378 0.177037 0 15.1898 0.402589 0.301095 0 3.10165 0.337754 0.333478 0 0.0137574 0.343373 0.331872 0 0 32.5 0.00213663 0.00154232 3.79391e-07 0.000866484 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 7.5 1 0 0 0 0 1 0 0 0 0 0
0 1 0 -32.5 0.153417 0.69667 0 -17.4874 0.28354 0.438327 0 -4.7922 0.332969 0.336856 0 -0.0579737 0.341467 0.325192 0 -0 -32.5 0.00116138 0.00558544 1.09328e-05 0.000204232 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 -7.5 1 0 0 5 0 1 0 0 0 0 0
1 0 0 32.5 0.751088 0.126387 0 20.1325 0.493964 0.256917 0 7.40419 0.345819 0.331583 0 0.244301 0.335924 0.333155 0 0 32.5 0.00858259 7.88927e-05 5.65903e-05 6.99075e-05 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 7.5 1 0 0 10 0 1 0 0 0 0 0
0 1 0 -32.5 0.0948651 0.813353 0 -23.1777 0.214486 0.57826 0 -11.4399 0.321543 0.368181 0 -1.02948 0.332611 0.337717 0 -0 -32.5 0.00314418 1.85624e-05 0.000316598 7.09468e-05 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 -7.5 1 0 0 15 0 1 0 0 0 0 0
1 0 0 32.5 0.883627 0.0588759 0 26.6835 0.703717 0.149932 0 17.6752 0.439904 0.283683 0 4.33823 0.367596 0.321518 0 0 32.5 0.0576059 0.00422065 0.00013395 0.00125083 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 7.5 1 0 0 20 0 1 0 0 0 0 0
0 1 0 -32.5 0.017957 0.96472 0 -30.7195 0.052367 0.897181 0 -27.309 0.143564 0.718724 0 -18.2812 0.330602 0.364138 0 -0 -32.5 0.0256414 0.0131855 2.58075e-05 0.00201715 1 1 1 1 0 0 1 1 1 1 1 2.87864e+06 1.76596e+06 1.00937e+06 980071 3.88882e+06 1.02858e+06 -7.5 1 0 0 25 0 1 0 0 0 0 0

scoreDistrN
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(6,170)}
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

selfBonusScoreN
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(6,61)}
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

valueTargetsNCHW
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(6,1,5,5)}
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1

HASH: 6C551B368DDA0C4D7310D9E43A2B282D
   A B C D E
 5 . . . . .
 4 . . . . .
 3 . . . . .
 2 . . . . .
 1 . . . . .


HASH: 608F0ED89B17F3FAC8A7CFC1163503F7
   A B C D E
 5 X . X O O
 4 . X O O O
 3 O X O O O
 2 . O O O O
 1 O O O . O


Encore phase 0
Rules koSIMPLEscoreTERRITORYsui0komi5
Ko prohib hash 00000000000000000000000000000000
White bonus score 1
Game result 0 Empty 0 0 0
Last moves E5 C3 A2 C1 B5 B1 B3 E1 B4 E3 C5 D4 D3 E4 A3 E2 C2 D2 pass D3 A5 pass A1 pass A4 B2 D5 C4 pass A3 B4 E5 A5 pass B3 C2 C5 D5 A2 A1
binaryInputNCHWPacked
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|u1','fortran_order':False,'shape':(9,22,4)}
FFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
FFFFFF80000802004801000000000000080000004001020000000000000000000000000040000000000002000001000000080000080000000800000008000000080000001040000000000000000000000000000000000000
FFFFFF804A110000000A06800000000008000080000B000000000000000000000000000000020000020000000000008000100000000004000800000008000000080000000000000000000000000000000000000000000000
FFFFFF8000CA06806A350000080400000008008000C2000000000000000000000000000000200000004000000004000000800000200000000000000000000000080000000000000000000000000000000000000000000000
FFFFFF806A31400000CE3680080040000000000000CE368000000000000000000000000000040000000000000000200000004000000010000000000000000000000000000000000000000000000000000000000000000000
FFFFFF8000CE3680EE3148000800400000000600E6FF388000000000000000000000000004000000000000000000080000000000800000000800000000000600000006000000000000000000000000000000000000000000
FFFFFF800000000001EEB6800000000000000000002000000000000000000000000000000020000000000000010000001000000000008000000000000000000000000000000000000000000001CEF7800000000000000000
FFFFFF8009EEB6808210000000000000823000000000000000000000000000000000000000100000000000008000000008000000020000000000000080000000800000000000000009CEF780000000000000000000000000
FFFFFF80080100008008000000000000880000000001000000000000000000000000000080000000000100000008000008000000000000008800000008000000080000004400000000000000000000000000000000000000

globalInputNC
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<f4','fortran_order':False,'shape':(9,14)}
0 0 0 0 0 -0.333333 0 0 0 1 0 0 0 0
0 0 0 0 0 0.4 0 0 0 1 0 0 0 0
0 0 0 0 0 -0.333333 0 0 0 1 0 0 0 0
0 0 0 0 0 0.4 0 0 0 1 0 0 0 0
0 1 0 0 0 -0.266667 0 0 0 1 0 0 0 0
0 1 0 1 0 0.466667 0 0 0 1 0 0 0 0
0 1 0 0 0 -0.333333 0 0 0 1 0 0 0 0
0 1 0 0 0 0.466667 0 0 0 1 0 0 0 0
0 0 0 0 0 -0.333333 0 0 0 1 0 0 0 0

policyTargetsNCMove
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<i2','fortran_order':False,'shape':(9,1,26)}
22 0 0 4 23 5 0 0 7 8 5 0 0 3 10 0 0 0 12 0 0 0 0 0 0 0
0 0 0 1 0 0 7 16 4 11 17 0 0 0 4 0 7 0 0 0 4 18 0 5 0 5
9 0 32 0 0 10 0 8 3 0 26 0 0 0 0 0 0 0 4 0 1 0 0 0 0 6
8 0 0 8 0 2 0 15 0 0 0 0 0 0 0 0 2 9 0 41 0 0 0 7 0 7
40 0 0 5 0 10 0 7 0 0 0 0 0 0 0 0 8 0 0 0 10 0 0 0 0 19
0 0 0 15 0 0 0 13 0 0 0 0 0 0 0 0 35 0 0 0 0 0 0 28 0 8
0 12 9 0 9 2 42 0 0 0 0 4 0 0 0 11 0 0 0 0 10 0 0 0 0 0
0 2 4 11 0 0 0 0 0 0 0 0 0 0 0 13 0 57 0 0 3 0 0 9 0 0
0 1 0 1 0 14 10 0 6 14 10 3 0 8 8 0 0 12 0 9 0 1 0 0 1 1

globalTargetsNC
-109 78 85 77 80 89 1 0 -10 0 {'descr':'<f4','fortran_order':False,'shape':(9,54)}
0 1 0 -19 0.223783 0.549896 0 -6.15709 0.32145 0.355055 0 -0.585074 0.333119 0.336697 0 -0.000191075 0.332415 0.340303 0 -0 -19 0.00691665 0.00267455 0.000492418 2.35216e-05 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 -5 0 0 0 0 1 0 0 0 0 0 0
1 0 0 19 0.581993 0.207249 0 7.08839 0.364395 0.315058 0 0.90397 0.330549 0.33235 0 0.000805189 0.323236 0.335304 0 0 19 0.00785179 0.000119614 0.000416952 0.00062622 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 6 0 0 0 5 1 0 0 0 0 0 0
0 1 0 -19 0.188151 0.619832 0 -8.16056 0.305003 0.382404 0 -1.39668 0.32842 0.333159 0 -0.00339306 0.333288 0.332651 0 -0 -19 0.0146969 0.00143826 1.90991e-05 0.000171888 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 -5 0 0 0 10 1 0 0 0 0 0 0
1 0 0 19 0.663255 0.167141 0 9.3949 0.409455 0.292928 0 2.15794 0.334051 0.330981 0 0.0142983 0.330375 0.332189 0 0 19 0.0135932 0.000429984 0.00045477 0.000466499 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 6 0 0 0 15 1 0 0 0 0 0 0
0 1 0 -19 0.142381 0.712998 0 -10.8159 0.272194 0.450386 0 -3.33413 0.329715 0.333731 0 -0.060253 0.341355 0.322762 0 -0 -19 2.68346e-05 0.0034576 0.00365319 9.27226e-05 1 1 1 1 0 0 0 0 0 0 0 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 -4 0 0 0 20 1 0 0 0 0 0 0
1 0 0 19 0.770588 0.114252 0 12.4519 0.515 0.241244 0 5.1514 0.344537 0.325064 0 0.253906 0.337898 0.332186 0 0 19 0.0489347 0.00196919 0.000220969 0.00029862 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 7 0 0 0 25 1 0 0 0 0 0 0
0 1 0 -19 0.0820171 0.83634 0 -14.3354 0.194234 0.612539 0 -7.95917 0.315751 0.37 0 -1.06996 0.330142 0.331176 0 -0 -19 0.000752337 0.000250344 0.000287059 9.40825e-05 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 -5 0 0 0 30 1 0 0 0 0 0 0
1 0 0 19 0.912516 0.0437893 0 16.5037 0.765202 0.117627 0 12.2973 0.493151 0.254675 0 4.50879 0.342076 0.334891 0 0 19 0.0004603 9.07464e-07 0.00235922 0.000341231 1 1 1 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 7 0 0 0 35 1 0 0 0 0 0 0
0.334854 0.329664 0.335482 0.0910117 0.334854 0.329664 0.335482 0.0910117 0.334854 0.329664 0.335482 0.0910117 0.334854 0.329664 0.335482 0.0910117 0.334854 0.329664 0.335482 0.0910117 0 0.000141881 0.000418388 7.60255e-05 4.58709e-06 1 1 0 1 0 0 1 1 1 1 1 3.401e+06 3.73478e+06 585445 1.45281e+06 3.76859e+06 3244 -5 0 0 0 4 1 0 0 0 0 0 1

scoreDistrN
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(9,170)}
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 50 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

selfBonusScoreN
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(9,61)}
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

valueTargetsNCHW
-109 78 85 77 80 89 1 0 -10 0 {'descr':'|i1','fortran_order':False,'shape':(9,1,5,5)}
1 0 1 -1 -1 0 1 -1 -1 -1 -1 1 -1 -1 -1 0 -1 -1 -1 -1 -1 -1 -1 0 -1
-1 0 -1 1 1 0 -1 1 1 1 1 -1 1 1 1 0 1 1 1 1 1 1 1 0 1
1 0 1 -1 -1 0 1 -1 -1 -1 -1 1 -1 -1 -1 0 -1 -1 -1 -1 -1 -1 -1 0 -1
-1 0 -1 1 1 0 -1 1 1 1 1 -1 1 1 1 0 1 1 1 1 1 1 1 0 1
1 0 1 -1 -1 0 1 -1 -1 -1 -1 1 -1 -1 -1 0 -1 -1 -1 -1 -1 -1 -1 0 -1
-1 0 -1 1 1 0 -1 1 1 1 1 -1 1 1 1 0 1 1 1 1 1 1 1 0 1
1 0 1 -1 -1 0 1 -1 -1 -1 -1 1 -1 -1 -1 0 -1 -1 -1 -1 -1 -1 -1 0 -1
-1 0 -1 1 1 0 -1 1 1 1 1 -1 1 1 1 0 1 1 1 1 1 1 1 0 1
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0


)%%";
  expect("Test training write",out,expected);



  NeuralNet::globalCleanup();
}
