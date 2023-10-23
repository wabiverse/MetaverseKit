# MetaverseKit
Dependency Stack for the **Open Source Metaverse**.

Bundling the entirety of the [**ASWF Landscape**](https://landscape.aswf.io) with simple injection into the **Swift** programming language.

> **NOTE:** Swift is an open source programming language that is fully
supported across **Linux** and [**Swift on Server**](https://www.swift.org/server/),
the entire **Apple** family of devices: **macOS**, **visionOS**, **iOS**, **tvOS**, **watchOS**,
as well as support for **Microsoft Windows**. To learn more about Swift, please visit [swift.org](https://www.swift.org).
<br>

# Included (names based on Swift library names)
- **OneTBB v2021.10.0**
- **Boost v1.81.2**
- **Python v3.11.6**
- **MetaPy (Metaverse Python Libraries)**
- **IMath v3.1.9**
- **Alembic v1.8.5**
- **OpenColorIO v2.3.0**
- **Blosc v1.21.5**
- **OpenVDB v10.1.0**
- **OpenEXR v3.2.1**
- **OpenImageIO v2.5.4.0**
- **WebP v1.3.2**
- **TurboJPEG v3.0.1**
- **Raw v0.21.1**
- **TIFF v4.6.0**
- **LibPNG v1.6.40**
- **Ptex v2.4.2**
- **Eigen v3.4.0**
- **Draco v1.5.6**
- **Glslang v12.3.1**
- **MoltenVK v1.2.6-rc1** (darwin-based platforms)
- **Shaderc v2023.2**
- **SpirvCross v1.3.261.1**
- **PyBind11 v2.11.1**
- **HDF5 v1.10.0**
- **DEFLATE v1.19**
- **ZStandard v1.5.5**
- **ZLibDataCompression v1.2.11**
- **LZMA2 v5.4.4**
- **MiniZip v4.0.1**
- **Yaml v0.8.0**
- **OpenSSL v(BoringSSL `abfd5eb`)**
- **...growing...**

# Usage
##### To use MetaverseKit with Swift, add **MetaverseKit** as a **Package** dependency in your Package.swift file, [example usage](https://github.com/wabiverse/SwiftUSD/blob/main/Package.swift#L95).
```swift
dependencies: [
  .package(url: "https://github.com/wabiverse/MetaverseKit.git", from: "1.1.4"),
]
```
