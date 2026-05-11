import SwiftUI

struct ContentView: View {
    @Environment(KalahViewModel.self) private var vm

    var body: some View {
        ZStack {
            Color.bgWindow.ignoresSafeArea()
            Group {
                switch vm.appState {
                case .welcome:          WelcomeView()
                case .enterName:        NameView()
                case .selectGender:     GenderView()
                case .selectDifficulty: DifficultyView()
                case .playing:          GameView()
                }
            }
            .transition(.asymmetric(
                insertion: .move(edge: .trailing),
                removal:   .move(edge: .leading)
            ))
            .animation(.easeInOut(duration: 0.2), value: vm.appState)
        }
        .frame(minWidth: 720, idealWidth: 720, minHeight: 400, idealHeight: 400)
    }
}
