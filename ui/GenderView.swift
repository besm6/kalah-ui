import SwiftUI

struct GenderView: View {
    @Environment(KalahViewModel.self) private var vm

    var body: some View {
        VStack(spacing: 16) {
            Text("Select your gender")
                .font(.custom("Georgia", size: 20))
                .foregroundStyle(Color.tan)
                .padding(.bottom, 8)
            Button("Male")              { vm.selectGender(1) }.buttonStyle(ChoiceButtonStyle())
            Button("Female")            { vm.selectGender(2) }.buttonStyle(ChoiceButtonStyle())
            Button("Prefer not to say") { vm.selectGender(0) }.buttonStyle(ChoiceButtonStyle())
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.bgSetup)
    }
}
