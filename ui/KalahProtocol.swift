protocol KalahProtocol: AnyObject {
    func destroy()
    func proceedFromWelcome()
    func submitName(_ name: String)
    func selectGender(_ g: Int)
    func selectLevel(_ l: Int)
    func sow(pit: Int) -> Int       // 0=INVALID 1=SWITCH_TURN 2=EXTRA_TURN
    func selectAIMove() -> Int      // best pit index for JINN, -1 if none
    func doAIMove(pit: Int) -> Int
    func getPits() -> [Int]         // 14 elements: [0-5]=USER, [6]=USER kalah, [7-12]=JINN, [13]=JINN kalah
    func currentPlayer() -> Int     // 0=USER, 1=JINN
    func isGameOver() -> Bool
    func collectRemaining()
    func winner() -> Int            // 0=user wins, 1=jinn wins, -1=tie
    func newGame()
    func getUserName() -> String
}
