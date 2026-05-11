import SwiftUI

struct WelcomeView: View {
    @Environment(KalahViewModel.self) private var vm

    var body: some View {
        VStack(spacing: 28) {
            Text("Kalah")
                .font(.custom("Georgia", size: 48)).bold()
                .foregroundStyle(Color.gold)
            Text("A classic strategy game of skill and planning")
                .font(.custom("Georgia", size: 18))
                .foregroundStyle(Color.tan)
            Text("Tap anywhere to begin")
                .font(.custom("Georgia", size: 14))
                .foregroundStyle(Color.muted)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .contentShape(Rectangle())
        .onTapGesture { vm.proceedFromWelcome() }
    }
}
