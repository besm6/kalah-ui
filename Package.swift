// swift-tools-version: 5.10
import PackageDescription

let package = Package(
    name: "Kalah",
    platforms: [.macOS(.v14)],
    targets: [
        .target(
            name: "KalahEngine",
            path: "src",
            exclude: ["Kalah.md"],
            publicHeadersPath: "."
        ),
        .executableTarget(
            name: "Kalah",
            dependencies: ["KalahEngine"],
            path: "ui"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
