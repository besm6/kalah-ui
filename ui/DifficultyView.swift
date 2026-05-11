import SwiftUI

struct DifficultyView: View {
    @Environment(KalahViewModel.self) private var vm

    private let levels: [(title: String, subtitle: String, value: Int)] = [
        ("Юноша",    "Novice",      1),
        ("Кандидат", "Candidate",   2),
        ("Участник", "Participant", 3),
        ("Эфенди",   "Master",      4),
    ]

    var body: some View {
        VStack(spacing: 14) {
            Text("Select difficulty")
                .font(.custom("Georgia", size: 20))
                .foregroundStyle(Color.tan)
                .padding(.bottom, 8)
            ForEach(levels, id: \.value) { lvl in
                Button { vm.selectLevel(lvl.value) } label: {
                    VStack(spacing: 2) {
                        Text(lvl.title)
                            .font(.custom("Georgia", size: 20)).bold()
                        Text(lvl.subtitle)
                            .font(.custom("Georgia", size: 12))
                            .foregroundStyle(Color.muted)
                    }
                }
                .buttonStyle(ChoiceButtonStyle())
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.bgSetup)
    }
}
