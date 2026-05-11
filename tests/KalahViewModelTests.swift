import XCTest
@testable import KalahCore

@MainActor
final class KalahViewModelTests: XCTestCase {

    var mock: MockKalahEngine!
    var vm: KalahViewModel!

    override func setUp() async throws {
        mock = MockKalahEngine()
        vm = KalahViewModel(engine: mock, aiDelay: .zero)
    }

    // MARK: - TC-01: Initial state

    func test_initialState_isWelcome() {
        XCTAssertEqual(vm.appState, .welcome)
        XCTAssertEqual(mock.proceedFromWelcomeCallCount, 0)
    }

    // MARK: - TC-02: proceedFromWelcome

    func test_proceedFromWelcome_setsEnterName() {
        vm.proceedFromWelcome()
        XCTAssertEqual(vm.appState, .enterName)
        XCTAssertEqual(mock.proceedFromWelcomeCallCount, 1)
    }

    // MARK: - TC-03: submitName with a real name

    func test_submitName_storesNameAndAdvancesToSelectGender() {
        vm.submitName("Alice")
        XCTAssertEqual(vm.appState, .selectGender)
        XCTAssertEqual(vm.userName, "Alice")
        XCTAssertEqual(mock.lastSubmittedName, "Alice")
        XCTAssertEqual(mock.submitNameCallCount, 1)
    }

    // MARK: - TC-04: submitName with empty string falls back to "Player"

    func test_submitEmptyName_fallsBackToPlayer() {
        vm.submitName("")
        XCTAssertEqual(vm.userName, "Player")
        XCTAssertEqual(mock.lastSubmittedName, "")
    }

    // MARK: - TC-05: selectLevel sets playing state and syncs board

    func test_selectLevel_setsPlayingAndSyncsBoard() {
        mock.stubbedPits = Array(0..<14)
        vm.selectLevel(2)
        XCTAssertEqual(vm.appState, .playing)
        XCTAssertEqual(mock.selectLevelCallCount, 1)
        XCTAssertEqual(mock.lastLevel, 2)
        XCTAssertEqual(vm.pits, mock.stubbedPits)
    }

    // MARK: - TC-06: sow calls engine and syncs pits

    func test_sow_callsEngineAndSyncsPits() {
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1  // after move it becomes JINN's turn
        mock.stubbedPits = Array(1...14)
        vm.sow(3)
        XCTAssertEqual(mock.sowCallCount, 1)
        XCTAssertEqual(mock.lastSowPit, 3)
        XCTAssertEqual(vm.pits, mock.stubbedPits)
    }

    // MARK: - TC-07: sow with INVALID result does not sync board

    func test_sow_invalidResult_noSync() {
        mock.nextSowResult = 0
        let pitsBefore = vm.pits
        vm.sow(0)
        XCTAssertEqual(mock.sowCallCount, 1)
        XCTAssertEqual(vm.pits, pitsBefore)
    }

    // MARK: - TC-08: sow is blocked while AI is thinking

    func test_sow_guardBlocksWhenAIThinking() async {
        // Trigger AI scheduling by making first sow return SWITCH_TURN with JINN as next player
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1
        // Give AI move a non-zero delay so it stays in flight
        let slowVM = KalahViewModel(engine: mock, aiDelay: .seconds(60))
        slowVM.sow(0)                    // triggers scheduleAIMove → isAIThinking = true

        let sowCountBefore = mock.sowCallCount
        slowVM.sow(1)                    // should be blocked
        XCTAssertEqual(mock.sowCallCount, sowCountBefore)
    }

    // MARK: - TC-09: extra turn does not schedule AI

    func test_sow_extraTurn_noAIScheduled() {
        mock.nextSowResult = 2            // EXTRA_TURN
        mock.stubbedCurrentPlayer = 0     // still USER
        vm.sow(2)
        XCTAssertFalse(vm.isAIThinking)
        XCTAssertEqual(mock.selectAIMoveCallCount, 0)
    }

    // MARK: - TC-10: SWITCH_TURN to JINN sets isAIThinking

    func test_sow_switchTurnToJinn_setsAIThinking() {
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1    // syncBoard will see JINN as current player
        vm.sow(0)
        XCTAssertTrue(vm.isAIThinking)
    }

    // MARK: - TC-11: AI move completes and clears isAIThinking

    func test_aiMove_completesAndClearsThinking() async {
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1   // first syncBoard sees JINN → triggers AI scheduling
        mock.nextAIMove = 0
        mock.nextAIMoveResults = [1]    // AI move returns SWITCH_TURN

        vm.sow(0)
        XCTAssertTrue(vm.isAIThinking)

        // Switch to USER before the AI task's final syncBoard so it doesn't reschedule
        mock.stubbedCurrentPlayer = 0
        try? await Task.sleep(for: .milliseconds(100))
        XCTAssertFalse(vm.isAIThinking)
    }

    // MARK: - TC-12: AI extra turn causes AI to move again

    func test_aiExtraTurn_aiMovesAgain() async {
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1
        mock.nextAIMove = 0
        // First AI move returns EXTRA_TURN, second returns SWITCH_TURN
        mock.nextAIMoveResults = [2, 1]

        vm.sow(0)
        try? await Task.sleep(for: .milliseconds(100))
        XCTAssertEqual(mock.doAIMoveCallCount, 2)
        XCTAssertFalse(vm.isAIThinking)
    }

    // MARK: - TC-13: user sow triggers game over

    func test_sow_gameOver_setsIsGameOverAndWinner() {
        mock.nextSowResult = 1
        mock.stubbedIsGameOver = true
        mock.stubbedWinner = 0
        vm.sow(0)
        XCTAssertTrue(vm.isGameOver)
        XCTAssertEqual(vm.winner, 0)
        XCTAssertEqual(mock.collectRemainingCallCount, 1)
    }

    // MARK: - TC-14: AI move triggers game over

    func test_aiMove_gameOver_setsWinner() async {
        mock.nextSowResult = 1
        mock.stubbedCurrentPlayer = 1
        mock.nextAIMove = 0
        mock.nextAIMoveResults = [1]
        mock.stubbedIsGameOver = true
        mock.stubbedWinner = 1

        vm.sow(0)
        try? await Task.sleep(for: .milliseconds(100))
        XCTAssertTrue(vm.isGameOver)
        XCTAssertEqual(vm.winner, 1)
        XCTAssertFalse(vm.isAIThinking)
        XCTAssertEqual(mock.collectRemainingCallCount, 1)
    }

    // MARK: - TC-15: newGame resets flags and syncs board

    func test_newGame_resetsAndSyncsBoard() {
        // Simulate a finished game
        mock.stubbedIsGameOver = true
        mock.stubbedWinner = 0
        mock.nextSowResult = 1
        vm.sow(0)
        XCTAssertTrue(vm.isGameOver)

        mock.stubbedIsGameOver = false
        mock.stubbedPits = Array(repeating: 6, count: 14)
        vm.newGame()
        XCTAssertFalse(vm.isGameOver)
        XCTAssertEqual(vm.winner, -1)
        XCTAssertFalse(vm.isAIThinking)
        XCTAssertEqual(mock.newGameCallCount, 1)
        XCTAssertEqual(vm.pits, mock.stubbedPits)
    }
}
