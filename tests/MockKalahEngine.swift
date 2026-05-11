@testable import KalahCore

final class MockKalahEngine: KalahProtocol {

    // MARK: - Recorded calls
    var proceedFromWelcomeCallCount = 0
    var submitNameCallCount = 0
    var lastSubmittedName: String?
    var selectGenderCallCount = 0
    var lastGender: Int?
    var selectLevelCallCount = 0
    var lastLevel: Int?
    var sowCallCount = 0
    var lastSowPit: Int?
    var selectAIMoveCallCount = 0
    var doAIMoveCallCount = 0
    var collectRemainingCallCount = 0
    var newGameCallCount = 0

    // MARK: - Configurable return values
    var nextSowResult = 1           // default: SWITCH_TURN
    var nextAIMove = 0              // pit index AI will choose
    var nextAIMoveResults: [Int] = [1]  // queue of results for successive doAIMove calls
    var stubbedPits = Array(repeating: 4, count: 14)
    var stubbedCurrentPlayer = 0
    var stubbedIsGameOver = false
    var stubbedWinner = -1
    var stubbedUserName = "TestPlayer"

    // MARK: - KalahProtocol

    func destroy() {}

    func proceedFromWelcome() {
        proceedFromWelcomeCallCount += 1
    }

    func submitName(_ name: String) {
        submitNameCallCount += 1
        lastSubmittedName = name
    }

    func selectGender(_ g: Int) {
        selectGenderCallCount += 1
        lastGender = g
    }

    func selectLevel(_ l: Int) {
        selectLevelCallCount += 1
        lastLevel = l
    }

    func sow(pit: Int) -> Int {
        sowCallCount += 1
        lastSowPit = pit
        return nextSowResult
    }

    func selectAIMove() -> Int {
        selectAIMoveCallCount += 1
        return nextAIMove
    }

    func doAIMove(pit: Int) -> Int {
        let result = nextAIMoveResults.isEmpty ? 1 : nextAIMoveResults.removeFirst()
        doAIMoveCallCount += 1
        return result
    }

    func getPits() -> [Int] { stubbedPits }
    func currentPlayer() -> Int { stubbedCurrentPlayer }
    func isGameOver() -> Bool { stubbedIsGameOver }
    func collectRemaining() { collectRemainingCallCount += 1 }
    func winner() -> Int { stubbedWinner }
    func newGame() { newGameCallCount += 1 }
    func getUserName() -> String { stubbedUserName }
}
