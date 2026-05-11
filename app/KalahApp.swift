import SwiftUI
import KalahCore

@main
struct KalahApp: App {
    @State private var vm = KalahViewModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(vm)
        }
        .windowResizability(.contentSize)
    }
}
