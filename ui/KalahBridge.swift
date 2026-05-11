import KalahEngine

final class KalahBridge: KalahProtocol {
    private var engine: KalahSwift

    init() {
        engine = KalahSwift()
    }

    func destroy() {}

    func proceedFromWelcome() {
        engine.proceedFromWelcome()
    }

    func submitName(_ name: String) {
        name.withCString { engine.submitName($0) }
    }

    func selectGender(_ g: Int) {
        engine.selectGender(Int32(g))
    }

    func selectLevel(_ l: Int) {
        engine.selectLevel(Int32(l))
    }

    func sow(pit: Int) -> Int {
        Int(engine.sow(Int32(pit)))
    }

    func selectAIMove() -> Int {
        Int(engine.selectAIMove())
    }

    func doAIMove(pit: Int) -> Int {
        Int(engine.doAIMove(Int32(pit)))
    }

    func getPits() -> [Int] {
        var result = [Int](repeating: 0, count: 14)
        for i in 0...6 {
            result[i]     = Int(engine.getPitAt(1, Int32(i)))  // USER = 1
        }
        for i in 0...6 {
            result[7 + i] = Int(engine.getPitAt(0, Int32(i)))  // JINN = 0
        }
        return result
    }

    func currentPlayer() -> Int {
        Int(engine.currentPlayer())
    }

    func isGameOver() -> Bool {
        engine.isGameOver()
    }

    func collectRemaining() {
        engine.collectRemaining()
    }

    func winner() -> Int {
        Int(engine.winner())
    }

    func newGame() {
        engine.newGame()
    }

    func getUserName() -> String {
        String(cString: engine.getUserName())
    }
}
