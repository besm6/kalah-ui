import Foundation
import Observation
import KalahEngine

@MainActor
@Observable
final class KalahViewModel {
    enum AppState: Int {
        case welcome, enterName, selectGender, selectDifficulty, playing
    }

    private(set) var appState: AppState = .welcome
    private(set) var pits: [Int] = Array(repeating: 0, count: 14)
    private(set) var currentPlayer: Int = 0   // 0 = USER, 1 = JINN
    private(set) var isGameOver: Bool = false
    private(set) var winner: Int = -1         // 0=user 1=jinn -1=tie
    private(set) var isAIThinking: Bool = false
    private(set) var userName: String = "Player"

    @ObservationIgnored
    private var handle: UnsafeMutableRawPointer

    init() {
        handle = kalah_create()!
    }

    deinit {
        kalah_destroy(handle)
    }

    // MARK: - Navigation actions

    func proceedFromWelcome() {
        kalah_proceed_from_welcome(handle)
        appState = .enterName
    }

    func submitName(_ name: String) {
        name.withCString { kalah_submit_name(handle, $0) }
        userName = name.isEmpty ? "Player" : name
        appState = .selectGender
    }

    func selectGender(_ g: Int) {
        kalah_select_gender(handle, Int32(g))
        appState = .selectDifficulty
    }

    func selectLevel(_ l: Int) {
        kalah_select_level(handle, Int32(l))
        appState = .playing
        syncBoard()
    }

    // MARK: - Game actions

    func sow(_ pit: Int) {
        guard !isAIThinking, !isGameOver, currentPlayer == 0 else { return }
        let result = Int(kalah_sow(handle, Int32(pit)))
        guard result != 0 else { return }
        handleMoveResult(result)
    }

    func newGame() {
        kalah_new_game(handle)
        isGameOver = false
        winner = -1
        isAIThinking = false
        syncBoard()
    }

    // MARK: - Internal

    private func syncBoard() {
        var buf = [Int32](repeating: 0, count: 14)
        buf.withUnsafeMutableBufferPointer { ptr in
            kalah_get_pits(handle, ptr.baseAddress)
        }
        pits = buf.map { Int($0) }
        currentPlayer = Int(kalah_current_player(handle))
        userName = String(cString: kalah_get_user_name(handle))
    }

    private func handleMoveResult(_ result: Int) {
        if kalah_is_game_over(handle) != 0 {
            kalah_collect_remaining(handle)
            syncBoard()
            isGameOver = true
            winner = Int(kalah_winner(handle))
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
        try? await Task.sleep(nanoseconds: 700_000_000)
        let pit = kalah_select_ai_move(handle)
        guard pit >= 0 else {
            isAIThinking = false
            return
        }
        let result = Int(kalah_do_ai_move(handle, pit))
        if kalah_is_game_over(handle) != 0 {
            kalah_collect_remaining(handle)
            syncBoard()
            isAIThinking = false
            isGameOver = true
            winner = Int(kalah_winner(handle))
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
