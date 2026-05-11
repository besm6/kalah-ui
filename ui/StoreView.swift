import SwiftUI

struct StoreView: View {
    let label: String
    let stones: Int

    var body: some View {
        ZStack {
            Capsule()
                .fill(Color.storeFill)
                .overlay(Capsule().strokeBorder(Color.pitBorder, lineWidth: 2))
            VStack(spacing: 6) {
                Text(label)
                    .font(.custom("Georgia", size: 12))
                    .foregroundStyle(Color.stoneText)
                Text("\(stones)")
                    .font(.custom("Georgia", size: 28)).bold()
                    .foregroundStyle(Color.stoneText)
            }
        }
        .frame(width: 80, height: 200)
    }
}
