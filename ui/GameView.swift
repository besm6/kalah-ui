import SwiftUI

struct GameView: View {
    @Environment(KalahViewModel.self) private var vm

    var body: some View {
        VStack(spacing: 8) {
            scoreBar
            statusLabel
            board
                .frame(maxHeight: .infinity)
            Button("New Game") { vm.newGame() }
                .buttonStyle(GoldButtonStyle())
                .padding(.bottom, 4)
        }
        .padding(12)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.bgGame)
    }

    // MARK: - Subviews

    private var scoreBar: some View {
        HStack {
            Text(vm.userName)
                .font(.custom("Georgia", size: 16)).bold()
                .foregroundStyle(Color.gold)
                .frame(maxWidth: .infinity, alignment: .leading)
            Text("\(vm.pits[6]) \u{2014} \(vm.pits[13])")
                .font(.custom("Georgia", size: 16))
                .foregroundStyle(Color.tan)
                .frame(maxWidth: .infinity, alignment: .center)
            Text("Jinn")
                .font(.custom("Georgia", size: 16)).bold()
                .foregroundStyle(Color.jinnLabel)
                .frame(maxWidth: .infinity, alignment: .trailing)
        }
        .padding(.horizontal, 4)
    }

    private var statusLabel: some View {
        Text(statusText)
            .font(.custom("Georgia", size: 14))
            .foregroundStyle(statusColor)
            .frame(maxWidth: .infinity, alignment: .center)
    }

    private var board: some View {
        HStack(spacing: 8) {
            StoreView(label: vm.userName, stones: vm.pits[6])
            boardGrid
            StoreView(label: "Jinn", stones: vm.pits[13])
        }
    }

    private var boardGrid: some View {
        let userCanPlay = !vm.isGameOver && !vm.isAIThinking && vm.currentPlayer == 0
        return VStack(spacing: 8) {
            // JINN pits: indices 12,11,10,9,8,7 left-to-right
            HStack(spacing: 8) {
                ForEach(0..<6, id: \.self) { col in
                    PitView(stones: vm.pits[12 - col], isPlayable: false) { }
                }
            }
            // USER pits: indices 0,1,2,3,4,5
            HStack(spacing: 8) {
                ForEach(0..<6, id: \.self) { col in
                    let playable = userCanPlay && vm.pits[col] > 0
                    PitView(stones: vm.pits[col], isPlayable: playable) {
                        vm.sow(col)
                    }
                }
            }
        }
    }

    // MARK: - Status helpers

    private var statusText: String {
        if vm.isGameOver {
            switch vm.winner {
            case 0:  return "\(vm.userName) wins!"
            case 1:  return "Jinn wins!"
            default: return "Tie game!"
            }
        }
        if vm.isAIThinking { return "Jinn is thinking\u{2026}" }
        return "Your turn"
    }

    private var statusColor: Color {
        if vm.isGameOver   { return .gold }
        if vm.isAIThinking { return .muted }
        return .tan
    }
}
