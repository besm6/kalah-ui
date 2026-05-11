import SwiftUI

struct PitView: View {
    let stones: Int
    let isPlayable: Bool
    let onTap: () -> Void

    @State private var isPressed = false

    var body: some View {
        ZStack {
            Circle()
                .fill(fillColor)
                .overlay(Circle().strokeBorder(Color.pitBorder, lineWidth: 2))
            Text("\(stones)")
                .font(.custom("Georgia", size: 22)).bold()
                .foregroundStyle(Color.stoneText)
        }
        .frame(width: 80, height: 80)
        .scaleEffect(isPressed ? 0.92 : 1.0)
        .animation(.easeInOut(duration: 0.08), value: isPressed)
        .onTapGesture {
            guard isPlayable && stones > 0 else { return }
            isPressed = true
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.12) {
                isPressed = false
            }
            onTap()
        }
    }

    private var fillColor: Color {
        if isPressed    { return .pitPressed }
        if isPlayable   { return .pitPlay }
        return .pitIdle
    }
}
