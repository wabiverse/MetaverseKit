<div align="center">

<h1 align="center">
  MetaverseKit
</h1>

<p align="center">
  <i align="center">Monolithic dependency encapsulating many <a href="https://landscape.aswf.io"><b>ASWF</b></a> projects to <b>swiftly</b> architect the <b>metaverse</b>.</i>
</p>

<h4 align="center">
  <a href="https://github.com/wabiverse/MetaverseKit/actions/workflows/swift-ubuntu.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/wabiverse/MetaverseKit/swift-ubuntu.yml?style=flat-square&label=ubuntu%20&labelColor=E95420&logoColor=FFFFFF&logo=ubuntu">
  </a>
  <a href="https://github.com/wabiverse/MetaverseKit/actions/workflows/swift-macos.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/wabiverse/MetaverseKit/swift-macos.yml?style=flat-square&label=macOS&labelColor=000000&logo=apple">
  </a>
  <a href="https://github.com/wabiverse/MetaverseKit/actions/workflows/swift-debug-windows.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/wabiverse/MetaverseKit/swift-debug-windows.yml?style=flat-square&label=windows&labelColor=357EC7&logo=gitforwindows">
  </a>
</h4>

<image src="https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExb2pxN2Q4Y29kdmtuaGp5b3JwMDRhdmtmcHJ0c3VzZXhrc25hdnV1ZiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/f9m9SX5FUGIUImpaSa/giphy.gif">

### Embark on the Metaversal Frontier 

Swiftly integrate the [**ASWF Landscape**](https://landscape.aswf.io) into mighty apps, plugins, and games with seamless simplicity. 
**MetaverseKit** is a monolithic dependency target of the [**SwiftUSD**](https://github.com/wabiverse/SwiftUSD) package, and will
continue to evolve with additional plugins, tools, and developer resources overtime. Once industry leaders adopt official support
of these libraries as Swift Packages, the **MetaverseKit** package will begin to pull in these official git remotes through versioned
releases, and we will provide monolithic releases in accordance with the [**VFX Reference Platform**](https://vfxplatform.com).

<br/>

</div>

# Usage
##### To use MetaverseKit with Swift, add **MetaverseKit** as a **Package** dependency in your Package.swift file.
```swift
dependencies: [
  .package(url: "https://github.com/wabiverse/MetaverseKit.git", from: "1.8.7"),
]
```

<hr/>

### Swift Targets

| Swift Target                                                               | Version           |
| -------------------------------------------------------------------------- | ----------------- |
| OneTBB ([in progress](https://github.com/oneapi-src/oneTBB/issues/1244))   | 2021.10.0         |
| Python                                                                     | 3.11.6            |
| PyBind11                                                                   | 2.11.1            |
| Boost                                                                      | 1.81.2            |
| OpenSubdiv                                                                 | 3.6.0             |
| OpenImageIO                                                                | master [a2f044a](https://github.com/AcademySoftwareFoundation/OpenImageIO/commit/a2f044a34c70b361d4927728b67714517cfd59ed)           |
| OpenColorIO                                                                | 2.3.0             |
| OpenTimelineIO                                                             | 0.17.0            |
| OSL                                                                        | 1.12.14.0 (TODO)  |
| Ptex                                                                       | 2.4.2             |
| ImGui                                                                      | 1.89.9            |
| Embree                                                                     | ??? (TODO)        |
| Alembic                                                                    | 1.8.5             |
| OpenEXR                                                                    | 3.2.1             |
| IMath                                                                      | 3.1.9             |
| MaterialX                                                                  | 1.38.8            |
| OpenVDB                                                                    | 10.1.0            |
| HDF5                                                                       | 1.10.0            |
| Blosc                                                                      | 1.21.5            |
| Eigen                                                                      | 3.4.0             |
| Draco                                                                      | 1.5.6             |
| TurboJPEG                                                                  | 3.0.1             |
| TIFF                                                                       | 4.6.0             |
| LibPNG                                                                     | 1.6.40            |
| WebP                                                                       | 1.3.2             |
| RAW                                                                        | 0.21.1            |
| ZStandard                                                                  | 1.5.5             |
| LZMA2                                                                      | 5.4.4             |
| MiniZip                                                                    | 4.0.1             |
| Yaml                                                                       | 0.8.0             |

### Platform Independent
| Swift Target                                                                                                                                           | Version                              |
| ------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------ |
| Apple ([Metal-cpp]([https://github.com/oneapi-src/oneTBB/issues/1244](https://developer.apple.com/metal/cpp/files/metal-cpp_macOS14.2_iOS17.2.zip)))   | macOS 14.2 / iOS 17.2 / visionOS 1.0 |

> [!NOTE]
> Swift is an open source programming language that is fully
> supported across **Linux** and [**Swift on Server**](https://www.swift.org/server/),
> the entire **Apple** family of devices: **macOS**, **visionOS**, **iOS**, **tvOS**, **watchOS**,
> as well as support for **Microsoft Windows**. To learn more about Swift, please visit [swift.org](https://www.swift.org).
<br>
