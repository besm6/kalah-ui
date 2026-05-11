import XCTest
import KalahEngine
@testable import KalahCore

final class KalahSwiftTests: XCTestCase {

    var engine: KalahSwift!

    // Player indices for getPitAt / setPit / setKalah / setCurrentPlayer (match Player enum)
    let JINN = Int32(0)
    let USER = Int32(1)
    // Values returned by currentPlayer() (UI convention: 0=USER turn, 1=JINN turn)
    let CURR_USER = Int32(0)
    let CURR_JINN = Int32(1)

    override func setUp() {
        super.setUp()
        engine = KalahSwift()
        engine.newGame()
    }

    // Directly configure board state — mirrors C++ setBoard() helper.
    // currentPlayer: 1=USER (default), 0=JINN
    private func setBoard(userPits: [Int], userKalah: Int,
                          jinnPits: [Int], jinnKalah: Int, currentPlayer: Int = 1) {
        for i in 0..<6 {
            engine.setPit(USER, Int32(i), Int32(userPits[i]))
            engine.setPit(JINN, Int32(i), Int32(jinnPits[i]))
        }
        engine.setKalah(USER, Int32(userKalah))
        engine.setKalah(JINN, Int32(jinnKalah))
        engine.setCurrentPlayer(Int32(currentPlayer))
    }

    // Sum of all 14 pit values — mirrors C++ totalStones().
    private func totalStones() -> Int {
        (0..<2).flatMap { p in (0..<7).map { i in Int(engine.getPitAt(Int32(p), Int32(i))) } }
               .reduce(0, +)
    }

    // Snapshot of all 14 pit values for mutation-check tests.
    private func boardSnapshot() -> [Int] {
        (0..<2).flatMap { p in (0..<7).map { i in Int(engine.getPitAt(Int32(p), Int32(i))) } }
    }

    // MARK: - A: Initialization

    func test_Init_AllPitsAreSix() {
        for i in 0..<6 {
            XCTAssertEqual(engine.getPitAt(USER, Int32(i)), 6, "USER pit \(i)")
            XCTAssertEqual(engine.getPitAt(JINN, Int32(i)), 6, "JINN pit \(i)")
        }
    }

    func test_Init_KalahsAreZero() {
        XCTAssertEqual(engine.getPitAt(USER, 6), 0)
        XCTAssertEqual(engine.getPitAt(JINN, 6), 0)
    }

    func test_Init_TotalStonesIs72() {
        XCTAssertEqual(totalStones(), 72)
    }

    func test_Init_CurrentPlayerIsUser() {
        XCTAssertEqual(engine.currentPlayer(), CURR_USER)
    }

    func test_Init_CanResetDirtyBoard() {
        _ = engine.sow(0)
        engine.newGame()
        for i in 0..<6 {
            XCTAssertEqual(engine.getPitAt(USER, Int32(i)), 6, "USER pit \(i)")
            XCTAssertEqual(engine.getPitAt(JINN, Int32(i)), 6, "JINN pit \(i)")
        }
        XCTAssertEqual(engine.getPitAt(USER, 6), 0)
        XCTAssertEqual(engine.getPitAt(JINN, 6), 0)
        XCTAssertEqual(totalStones(), 72)
    }

    // MARK: - B: Sowing Mechanics

    func test_Sow_OneStone_LandsInNextPit() {
        setBoard(userPits: [1,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        let r = engine.sow(0)
        XCTAssertEqual(r, 1)  // SWITCH_TURN
        XCTAssertEqual(engine.getPitAt(USER, 0), 0)
        XCTAssertEqual(engine.getPitAt(USER, 1), 1)
    }

    func test_Sow_DefaultBoard_Pit1_SwitchTurn() {
        let r = engine.sow(1)
        XCTAssertEqual(r, 1)  // SWITCH_TURN
        XCTAssertEqual(engine.getPitAt(USER, 1), 0)
        XCTAssertEqual(engine.getPitAt(USER, 2), 7)
        XCTAssertEqual(engine.getPitAt(USER, 5), 7)
        XCTAssertEqual(engine.getPitAt(USER, 6), 1)   // USER kalah
        XCTAssertEqual(engine.getPitAt(JINN, 0), 7)
        XCTAssertEqual(totalStones(), 72)
    }

    func test_Sow_SevenStones_WrapsToJinnSide() {
        setBoard(userPits: [7,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        let r = engine.sow(0)
        XCTAssertEqual(r, 1)  // SWITCH_TURN
        XCTAssertEqual(engine.getPitAt(USER, 0), 0)
        for i in 1..<6 {
            XCTAssertEqual(engine.getPitAt(USER, Int32(i)), 1, "USER pit \(i)")
        }
        XCTAssertEqual(engine.getPitAt(USER, 6), 1)   // USER kalah
        XCTAssertEqual(engine.getPitAt(JINN, 0), 1)
        XCTAssertEqual(engine.getPitAt(JINN, 6), 0)   // JINN kalah untouched
        XCTAssertEqual(totalStones(), 7)
    }

    func test_Sow_TwelveStones_SkipsJinnKalah() {
        setBoard(userPits: [12,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(0)
        XCTAssertEqual(engine.getPitAt(USER, 6), 1)   // USER kalah got 1
        for i in 0..<6 {
            XCTAssertEqual(engine.getPitAt(JINN, Int32(i)), 1, "JINN pit \(i)")
        }
        XCTAssertEqual(engine.getPitAt(JINN, 6), 0)   // JINN kalah skipped
    }

    func test_Sow_JinnSide_SkipsUserKalah() {
        // 8 stones from JINN pit 0: crosses all JINN pits, JINN kalah, USER pits 0-1; USER kalah skipped
        setBoard(userPits: [0,0,0,0,0,0], userKalah: 0,
                 jinnPits: [8,0,0,0,0,0], jinnKalah: 0, currentPlayer: 0)  // 0=JINN
        _ = engine.doAIMove(0)
        XCTAssertEqual(engine.getPitAt(USER, 6), 0)   // USER kalah untouched
        XCTAssertEqual(engine.getPitAt(JINN, 6), 1)   // JINN kalah got 1
    }

    func test_Sow_StoneInvariantAfterMoves() {
        // Alternating USER/JINN moves; stone total must stay 72 after each valid move
        let moves: [(isSow: Bool, pit: Int32)] = [
            (true, 1), (false, 1), (true, 2),
            (false, 2), (true, 3), (false, 3),
        ]
        for (isSow, pit) in moves {
            let r = isSow ? engine.sow(pit) : engine.doAIMove(pit)
            if r != 0 {
                XCTAssertEqual(totalStones(), 72, "after \(isSow ? "sow" : "AI") pit \(pit)")
            }
        }
    }

    // MARK: - C: Extra Turn Rule

    func test_ExtraTurn_Pit0With6Stones() {
        setBoard(userPits: [6,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        XCTAssertEqual(engine.sow(0), 2)  // EXTRA_TURN
        XCTAssertEqual(engine.getPitAt(USER, 6), 1)
    }

    func test_ExtraTurn_Pit5With1Stone() {
        setBoard(userPits: [0,0,0,0,0,1], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        XCTAssertEqual(engine.sow(5), 2)  // EXTRA_TURN
        XCTAssertEqual(engine.getPitAt(USER, 6), 1)
    }

    func test_ExtraTurn_DoesNotChangeCurrentPlayer() {
        setBoard(userPits: [0,0,0,0,0,1], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(5)
        XCTAssertEqual(engine.currentPlayer(), CURR_USER)
    }

    func test_ExtraTurn_NotGranted_WhenNotLandingInKalah() {
        setBoard(userPits: [1,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        XCTAssertEqual(engine.sow(0), 1)  // SWITCH_TURN
    }

    func test_ExtraTurn_JinnSide_Pit5With1Stone() {
        setBoard(userPits: [0,0,0,0,0,0], userKalah: 0,
                 jinnPits: [0,0,0,0,0,1], jinnKalah: 0, currentPlayer: 0)  // 0=JINN
        XCTAssertEqual(engine.doAIMove(5), 2)  // EXTRA_TURN
        XCTAssertEqual(engine.currentPlayer(), CURR_JINN)
    }

    // MARK: - D: Capture Rule

    func test_Capture_Basic() {
        // USER pit 4 has 1 stone → lands in pit 5 (empty); opposite=JINN[0]=9 → captured
        setBoard(userPits: [0,0,0,0,1,0], userKalah: 0, jinnPits: [9,0,0,0,0,0], jinnKalah: 0)
        let r = engine.sow(4)
        XCTAssertEqual(r, 1)  // SWITCH_TURN
        XCTAssertEqual(engine.getPitAt(USER, 6), 10)  // 9 captured + 1 landing stone
        XCTAssertEqual(engine.getPitAt(USER, 5), 0)   // landing pit cleared
        XCTAssertEqual(engine.getPitAt(JINN, 0), 0)   // opposite pit cleared
        XCTAssertEqual(totalStones(), 10)
    }

    func test_Capture_LandingPitNonEmpty_NoCapture() {
        setBoard(userPits: [1,3,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,5,0], jinnKalah: 0)
        _ = engine.sow(0)  // 1 stone → lands USER[1] which had 3 → no capture
        XCTAssertEqual(engine.getPitAt(USER, 6), 0)   // kalah unchanged
        XCTAssertEqual(engine.getPitAt(JINN, 4), 5)   // opposite pit unchanged
    }

    func test_Capture_OppositePitEmpty_NoCapture() {
        setBoard(userPits: [1,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(0)  // lands USER[1], opposite JINN[4]=0 → no capture
        XCTAssertEqual(engine.getPitAt(USER, 6), 0)
    }

    func test_Capture_AllOppositePitPairs() {
        for p in 1...5 {
            var userPits = [Int](repeating: 0, count: 6)
            var jinnPits = [Int](repeating: 0, count: 6)
            userPits[p - 1] = 1          // 1 stone → lands in USER[p]
            let opp = 5 - p              // expected opposite JINN pit
            jinnPits[opp] = 7
            setBoard(userPits: userPits, userKalah: 0, jinnPits: jinnPits, jinnKalah: 0)
            _ = engine.sow(Int32(p - 1))
            XCTAssertEqual(engine.getPitAt(USER, Int32(p)), 0, "landing pit cleared, p=\(p)")
            XCTAssertEqual(engine.getPitAt(JINN, Int32(opp)), 0, "opposite pit cleared, p=\(p)")
            XCTAssertEqual(engine.getPitAt(USER, 6), 8, "kalah=8, p=\(p)")  // 7+1
        }
    }

    func test_Capture_OnOpponentSide_NoCapture() {
        setBoard(userPits: [8,0,0,0,0,0], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(0)  // 8 stones wrap onto JINN side → last stone on JINN side, no capture
        XCTAssertEqual(engine.getPitAt(JINN, 6), 0)
    }

    func test_Capture_JinnCaptures() {
        setBoard(userPits: [9,0,0,0,0,0], userKalah: 0,
                 jinnPits: [0,0,0,0,1,0], jinnKalah: 0, currentPlayer: 0)  // 0=JINN
        let r = engine.doAIMove(4)
        XCTAssertEqual(r, 1)  // SWITCH_TURN
        XCTAssertEqual(engine.getPitAt(JINN, 6), 10)  // 9 + 1
        XCTAssertEqual(engine.getPitAt(JINN, 5), 0)   // landing pit cleared
        XCTAssertEqual(engine.getPitAt(USER, 0), 0)   // opposite pit cleared
    }

    func test_Capture_StoneInvariantPreserved() {
        setBoard(userPits: [0,0,0,0,1,0], userKalah: 0, jinnPits: [9,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(4)
        XCTAssertEqual(totalStones(), 10)
    }

    // MARK: - E: Invalid Moves

    func test_Invalid_NegativeIndex() {
        XCTAssertEqual(engine.sow(-1), 0)  // INVALID
    }

    func test_Invalid_IndexAtOrAboveBoundary() {
        XCTAssertEqual(engine.sow(6), 0)    // INVALID
        XCTAssertEqual(engine.sow(100), 0)  // INVALID
    }

    func test_Invalid_EmptyPit() {
        setBoard(userPits: [0,6,6,6,6,6], userKalah: 0, jinnPits: [6,6,6,6,6,6], jinnKalah: 0)
        XCTAssertEqual(engine.sow(0), 0)        // INVALID
        XCTAssertEqual(engine.getPitAt(USER, 0), 0)  // board unchanged
    }

    func test_Invalid_BoardUnchanged() {
        let before = boardSnapshot()
        _ = engine.sow(-1)
        XCTAssertEqual(boardSnapshot(), before)
    }

    // MARK: - F: Game Over & collectRemaining

    func test_GameOver_FreshBoard_False() {
        XCTAssertFalse(engine.isGameOver())
    }

    func test_GameOver_UserSideEmpty_True() {
        setBoard(userPits: [0,0,0,0,0,0], userKalah: 10, jinnPits: [6,6,6,6,6,6], jinnKalah: 0)
        XCTAssertTrue(engine.isGameOver())
    }

    func test_GameOver_JinnSideEmpty_True() {
        setBoard(userPits: [6,6,6,6,6,6], userKalah: 0, jinnPits: [0,0,0,0,0,0], jinnKalah: 10)
        XCTAssertTrue(engine.isGameOver())
    }

    func test_GameOver_BothRegularSidesEmpty_True() {
        setBoard(userPits: [0,0,0,0,0,0], userKalah: 36, jinnPits: [0,0,0,0,0,0], jinnKalah: 36)
        XCTAssertTrue(engine.isGameOver())
    }

    func test_GameOver_OneStoneEachSide_False() {
        setBoard(userPits: [1,0,0,0,0,0], userKalah: 0, jinnPits: [1,0,0,0,0,0], jinnKalah: 0)
        XCTAssertFalse(engine.isGameOver())
    }

    func test_CollectRemaining_SweepsToKalahs() {
        setBoard(userPits: [3,2,1,4,0,5], userKalah: 7,
                 jinnPits: [6,0,2,0,1,0], jinnKalah: 3)
        engine.collectRemaining()
        XCTAssertEqual(engine.getPitAt(USER, 6), 22)  // 7 + (3+2+1+4+0+5)
        XCTAssertEqual(engine.getPitAt(JINN, 6), 12)  // 3 + (6+0+2+0+1+0)
        for i in 0..<6 {
            XCTAssertEqual(engine.getPitAt(USER, Int32(i)), 0, "USER pit \(i)")
            XCTAssertEqual(engine.getPitAt(JINN, Int32(i)), 0, "JINN pit \(i)")
        }
    }

    func test_CollectRemaining_StoneInvariant() {
        let startTotal = 7 + (3+2+1+4+0+5) + 3 + (6+0+2+0+1+0)
        setBoard(userPits: [3,2,1,4,0,5], userKalah: 7,
                 jinnPits: [6,0,2,0,1,0], jinnKalah: 3)
        engine.collectRemaining()
        XCTAssertEqual(totalStones(), startTotal)
    }

    func test_CollectRemaining_ThenGameOver() {
        setBoard(userPits: [3,2,1,4,0,5], userKalah: 7,
                 jinnPits: [6,0,2,0,1,0], jinnKalah: 3)
        engine.collectRemaining()
        XCTAssertTrue(engine.isGameOver())
    }

    // MARK: - G: Stone Count Invariant

    func test_StoneInvariant_AfterInit() {
        XCTAssertEqual(totalStones(), 72)
    }

    func test_StoneInvariant_AfterCapture() {
        setBoard(userPits: [0,0,0,0,1,0], userKalah: 0, jinnPits: [9,0,0,0,0,0], jinnKalah: 0)
        _ = engine.sow(4)
        XCTAssertEqual(totalStones(), 10)
    }

    func test_StoneInvariant_AfterCollectRemaining() {
        let startTotal = 7 + (3+2+1+4+0+5) + 3 + (6+0+2+0+1+0)
        setBoard(userPits: [3,2,1,4,0,5], userKalah: 7,
                 jinnPits: [6,0,2,0,1,0], jinnKalah: 3)
        engine.collectRemaining()
        XCTAssertEqual(totalStones(), startTotal)
    }

    // MARK: - H: AI Selection

    func test_AI_FreshBoard_ValidIndex() {
        let pit = engine.selectAIMove()
        XCTAssertGreaterThanOrEqual(pit, 0)
        XCTAssertLessThan(pit, 6)
        XCTAssertGreaterThan(engine.getPitAt(JINN, pit), 0)
    }

    func test_AI_OneValidMove_ReturnsThatPit() {
        // Only JINN pit 5 is non-empty
        setBoard(userPits: [6,6,6,6,6,6], userKalah: 0,
                 jinnPits: [0,0,0,0,0,6], jinnKalah: 0, currentPlayer: 0)  // 0=JINN
        XCTAssertEqual(engine.selectAIMove(), 5)
    }

    func test_AI_NoMoves_ReturnsMinus1() {
        setBoard(userPits: [0,0,0,0,0,0], userKalah: 36,
                 jinnPits: [0,0,0,0,0,0], jinnKalah: 36)
        XCTAssertEqual(engine.selectAIMove(), -1)
    }

    func test_AI_DoesNotMutatePosition() {
        let before = boardSnapshot()
        _ = engine.selectAIMove()
        XCTAssertEqual(boardSnapshot(), before)
    }

    func test_AI_AllLevels_ValidIndex() {
        for level in 1...4 {
            engine.selectLevel(Int32(level))  // also calls initializeBoard
            let pit = engine.selectAIMove()
            XCTAssertGreaterThanOrEqual(pit, 0, "level=\(level)")
            XCTAssertLessThan(pit, 6, "level=\(level)")
        }
    }

    // MARK: - I: Settings

    func test_Settings_UserName() {
        "Alice".withCString { engine.submitName($0) }
        XCTAssertEqual(String(cString: engine.getUserName()), "Alice")
    }

    // MARK: - J: App State Machine

    func test_AppState_Initial_IsWelcome() {
        XCTAssertEqual(engine.appState(), 0)
    }

    func test_AppState_ProceedFromWelcome() {
        engine.proceedFromWelcome()
        XCTAssertEqual(engine.appState(), 1)
    }

    func test_AppState_SubmitName() {
        engine.proceedFromWelcome()
        "Bob".withCString { engine.submitName($0) }
        XCTAssertEqual(engine.appState(), 2)
        XCTAssertEqual(String(cString: engine.getUserName()), "Bob")
    }

    func test_AppState_SelectGender() {
        engine.proceedFromWelcome()
        "Test".withCString { engine.submitName($0) }
        engine.selectGender(1)
        XCTAssertEqual(engine.appState(), 3)
    }

    func test_AppState_SelectLevel() {
        engine.proceedFromWelcome()
        "Test".withCString { engine.submitName($0) }
        engine.selectGender(1)
        engine.selectLevel(2)
        XCTAssertEqual(engine.appState(), 4)
    }
}
