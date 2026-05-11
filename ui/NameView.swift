import SwiftUI

struct NameView: View {
    @Environment(KalahViewModel.self) private var vm
    @State private var name: String = ""

    var body: some View {
        VStack(spacing: 24) {
            Text("Enter your name")
                .font(.custom("Georgia", size: 20))
                .foregroundStyle(Color.tan)
            TextField("Your name", text: $name)
                .font(.custom("Georgia", size: 20))
                .textFieldStyle(.roundedBorder)
                .frame(maxWidth: 320)
                .onChange(of: name) { _, new in
                    if new.count > 24 { name = String(new.prefix(24)) }
                }
                .onSubmit { submit() }
            Button("Continue", action: submit)
                .buttonStyle(GoldButtonStyle())
                .disabled(name.trimmingCharacters(in: .whitespaces).isEmpty)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.bgSetup)
    }

    private func submit() {
        let trimmed = name.trimmingCharacters(in: .whitespaces)
        guard !trimmed.isEmpty else { return }
        vm.submitName(trimmed)
    }
}
