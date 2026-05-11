import SwiftUI

// MARK: - Color palette

extension Color {
    init(hex: String) {
        let h = hex.hasPrefix("#") ? String(hex.dropFirst()) : hex
        let v = UInt64(h, radix: 16) ?? 0
        let r = Double((v >> 16) & 0xFF) / 255
        let g = Double((v >> 8)  & 0xFF) / 255
        let b = Double( v        & 0xFF) / 255
        self.init(red: r, green: g, blue: b)
    }

    static let bgWindow   = Color(hex: "#1a0f05")
    static let bgSetup    = Color(hex: "#2a1e0e")
    static let bgGame     = Color(hex: "#3b2a14")
    static let gold       = Color(hex: "#d4a84b")
    static let tan        = Color(hex: "#c8b89a")
    static let muted      = Color(hex: "#a09070")
    static let pitPlay    = Color(hex: "#d8b48a")
    static let pitIdle    = Color(hex: "#b8946a")
    static let pitPressed = Color(hex: "#ffe0a0")
    static let pitBorder  = Color(hex: "#5a3a1a")
    static let storeFill  = Color(hex: "#a07a4c")
    static let stoneText  = Color(hex: "#2a1a08")
    static let jinnLabel  = Color(hex: "#a08060")
}

// MARK: - Button styles

struct GoldButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.custom("Georgia", size: 16)).bold()
            .foregroundStyle(Color(hex: "#2a1a08"))
            .padding(.vertical, 8)
            .padding(.horizontal, 24)
            .background(
                configuration.isPressed
                    ? Color(hex: "#b88a30")
                    : Color(hex: "#d4a84b")
            )
            .clipShape(RoundedRectangle(cornerRadius: 8))
    }
}

struct ChoiceButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.custom("Georgia", size: 18))
            .foregroundStyle(Color(hex: "#d4c8b0"))
            .frame(maxWidth: 280)
            .padding(.vertical, 10)
            .padding(.horizontal, 20)
            .background(Color(hex: "#2a1a08").opacity(configuration.isPressed ? 0.8 : 1))
            .clipShape(RoundedRectangle(cornerRadius: 10))
            .overlay(
                RoundedRectangle(cornerRadius: 10)
                    .strokeBorder(
                        configuration.isPressed ? Color.gold : Color(hex: "#5a4030"),
                        lineWidth: configuration.isPressed ? 2 : 1
                    )
            )
    }
}
