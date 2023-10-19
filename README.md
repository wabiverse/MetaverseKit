# MetaverseKit
Dependency Stack for the **Open Source Metaverse**.

Bundling the entirety of the [**ASWF Landscape**](https://landscape.aswf.io) with simple injection into the **Swift** programming language.

> **NOTE:** Swift is an open source programming language that is fully
supported across **Linux** and [**Swift on Server**](https://www.swift.org/server/),
the entire **Apple** family of devices: **macOS**, **visionOS**, **iOS**, **tvOS**, **watchOS**,
as well as support for **Microsoft Windows**. To learn more about Swift, please visit [swift.org](https://www.swift.org).
<br>

# Included
- **Eigen v3.4.0**
- **Draco v1.5.6**
- **Python v3.11.6**
- **Glslang v12.3.1**
- **MoltenVK v1.2.6-rc1** (darwin-based platforms)
- **...growing...**

# Usage
##### To use MetaverseKit with Swift, add **MetaverseKit** as a **Package** dependency in your Package.swift file, [example usage](https://github.com/wabiverse/SwiftUSD/blob/main/Package.swift#L95).
```swift
dependencies: [
  .package(url: "https://github.com/wabiverse/MetaverseKit.git", from: "1.0.3"),
]
```
