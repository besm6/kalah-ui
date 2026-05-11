// swift-tools-version: 5.10
import PackageDescription

let package = Package(
    name: "Kalah",
    platforms: [.macOS(.v14)],
    targets: [
        .target(
            name: "KalahEngine",
            path: "src",
            publicHeadersPath: "."
        ),
        .target(
            name: "KalahCore",
            dependencies: ["KalahEngine"],
            path: "ui",
            swiftSettings: [.interoperabilityMode(.Cxx)]
        ),
        .executableTarget(
            name: "Kalah",
            dependencies: ["KalahCore"],
            path: "app",
            exclude: ["Info.plist"],
            swiftSettings: [.interoperabilityMode(.Cxx)]
        ),
        .testTarget(
            name: "KalahTests",
            dependencies: ["KalahCore"],
            path: "tests",
            swiftSettings: [.interoperabilityMode(.Cxx)]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
