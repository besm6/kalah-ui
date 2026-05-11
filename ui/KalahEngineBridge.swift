import KalahEngine

final class KalahEngineBridge: KalahEngineProtocol {
    private var handle: UnsafeMutableRawPointer

    init() {
        handle = kalah_create()!
    }

    func destroy() {
        kalah_destroy(handle)
    }

    func proceedFromWelcome() {
        kalah_proceed_from_welcome(handle)
    }

    func submitName(_ name: String) {
        name.withCString { kalah_submit_name(handle, $0) }
    }

    func selectGender(_ g: Int) {
        kalah_select_gender(handle, Int32(g))
    }

    func selectLevel(_ l: Int) {
        kalah_select_level(handle, Int32(l))
    }

    func sow(pit: Int) -> Int {
        Int(kalah_sow(handle, Int32(pit)))
    }

    func selectAIMove() -> Int {
        Int(kalah_select_ai_move(handle))
    }

    func doAIMove(pit: Int) -> Int {
        Int(kalah_do_ai_move(handle, Int32(pit)))
    }

    func getPits() -> [Int] {
        var buf = [Int32](repeating: 0, count: 14)
        buf.withUnsafeMutableBufferPointer { kalah_get_pits(handle, $0.baseAddress) }
        return buf.map(Int.init)
    }

    func currentPlayer() -> Int {
        Int(kalah_current_player(handle))
    }

    func isGameOver() -> Bool {
        kalah_is_game_over(handle) != 0
    }

    func collectRemaining() {
        kalah_collect_remaining(handle)
    }

    func winner() -> Int {
        Int(kalah_winner(handle))
    }

    func newGame() {
        kalah_new_game(handle)
    }

    func getUserName() -> String {
        String(cString: kalah_get_user_name(handle))
    }
}
