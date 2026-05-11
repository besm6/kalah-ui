import Foundation
import Observation

@MainActor
@Observable
public final class KalahViewModel {
    public enum AppState: Int {
        case welcome, enterName, selectGender, selectDifficulty, playing
    }

    public private(set) var appState: AppState = .welcome
    public private(set) var pits: [Int] = Array(repeating: 0, count: 14)
    public private(set) var currentPlayer: Int = 0   // 0 = USER, 1 = JINN
    public private(set) var isGameOver: Bool = false
    public private(set) var winner: Int = -1         // 0=user 1=jinn -1=tie
    public private(set) var isAIThinking: Bool = false
    public private(set) var userName: String = "Player"

    @ObservationIgnored
    private var engine: any KalahEngineProtocol
    @ObservationIgnored
    private let aiDelay: Duration

    public init() {
        self.engine = KalahEngineBridge()
        self.aiDelay = .milliseconds(700)
    }

    init(engine: some KalahEngineProtocol, aiDelay: Duration = .milliseconds(700)) {
        self.engine = engine
        self.aiDelay = aiDelay
    }

    deinit {
        engine.destroy()
    }

    // MARK: - Navigation actions

    public func proceedFromWelcome() {
        engine.proceedFromWelcome()
        appState = .enterName
    }

    public func submitName(_ name: String) {
        engine.submitName(name)
        userName = name.isEmpty ? "Player" : name
        appState = .selectGender
    }

    public func selectGender(_ g: Int) {
        engine.selectGender(g)
        appState = .selectDifficulty
    }

    public func selectLevel(_ l: Int) {
        engine.selectLevel(l)
        appState = .playing
        syncBoard()
    }

    // MARK: - Game actions

    public func sow(_ pit: Int) {
        guard !isAIThinking, !isGameOver, currentPlayer == 0 else { return }
        let result = engine.sow(pit: pit)
        guard result != 0 else { return }
        handleMoveResult(result)
    }

    public func newGame() {
        engine.newGame()
        isGameOver = false
        winner = -1
        isAIThinking = false
        syncBoard()
    }

    // MARK: - Internal

    private func syncBoard() {
        pits = engine.getPits()
        currentPlayer = engine.currentPlayer()
        userName = engine.getUserName()
    }

    private func handleMoveResult(_ result: Int) {
        if engine.isGameOver() {
            engine.collectRemaining()
            syncBoard()
            isGameOver = true
            winner = engine.winner()
            return
        }
        syncBoard()
        if result == 2 { return }  // EXTRA_TURN — same player continues
        if currentPlayer == 1 {
            scheduleAIMove()
        }
    }

    private func scheduleAIMove() {
        isAIThinking = true
        Task { @MainActor in
            await performAITurn()
        }
    }

    private func performAITurn() async {
        try? await Task.sleep(for: aiDelay)
        let pit = engine.selectAIMove()
        guard pit >= 0 else {
            isAIThinking = false
            return
        }
        let result = engine.doAIMove(pit: pit)
        if engine.isGameOver() {
            engine.collectRemaining()
            syncBoard()
            isAIThinking = false
            isGameOver = true
            winner = engine.winner()
            return
        }
        syncBoard()
        if result == 2 {
            // AI got an extra turn — recurse after another delay
            await performAITurn()
        } else {
            isAIThinking = false
        }
    }
}
