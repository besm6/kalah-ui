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
        .target(
            name: "KalahCore",
            dependencies: ["KalahEngine"],
            path: "ui",
            exclude: ["KalahApp.swift"],
            swiftSettings: [.interoperabilityMode(.Cxx)]
        ),
        .executableTarget(
            name: "Kalah",
            dependencies: ["KalahCore"],
            path: "app",
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
