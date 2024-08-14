# MetaverseKit

<image src="https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExb2pxN2Q4Y29kdmtuaGp5b3JwMDRhdmtmcHJ0c3VzZXhrc25hdnV1ZiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/f9m9SX5FUGIUImpaSa/giphy.gif">

> One small step for **swift**, one giant leap for **reality**!

### Embark on the Metaversal Frontier 

Swiftly integrate the [**ASWF Landscape**](https://landscape.aswf.io) into mighty apps, plugins, and games with seamless simplicity. 
**MetaverseKit** is a monolithic dependency target of the [**SwiftUSD**](https://github.com/wabiverse/SwiftUSD) package, and will
continue to evolve with additional plugins, tools, and developer resources overtime. Once industry leaders adopt official support
of these libraries as Swift Packages, the **MetaverseKit** package will begin to pull in these official git remotes through versioned
releases, and we will provide monolithic releases in accordance with the [**VFX Reference Platform**](https://vfxplatform.com).

> [!NOTE]  
> The **Wabi Foundation** remains steadfast in its collaboration with industry leaders, diligently working to secure official support
  for these foundational libraries through the implementation of Swift Packages. This strategic initiative opens the gateway for developers,
  offering unparalleled opportunity to unleash the full power of industry-grade software, and unlocking its utmost potential.

<br/>

### 📰 News

Planning & initial development on [**CosmoGraph**](https://wabi.foundation/cosmograph/documentation/cosmograph/), seamlessly create native
cross-platform applications and games for **ObjCRT**, **GLibRT**, **FreeRT**, **WinRT**, and various graphics frameworks using a unified **Swift** and
**C++** API.

<br/>

### Wabi Targets
| Swift Target  | Description                                                                                                                                                                                                                                                                                                                                                                                                                       |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| MetaPy        | Metaversal Python, all the python modules from the below libraries, combined into a single swift target.                                                                                                                                                                                                                                                                                                                          |
| MXGraphEditor | MaterialX Graph Editor, visualizing, creating, and editing MaterialX graphs on the flick of a `swift run MXGraphEditor` command.                                                                                                                                                                                                                                                                                                  |
| MXResources   | MaterialX Resources, adding this target to your swift app or plugin will automatically install the [**MaterialX Standard Library**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries) and [**MaterialX Resources**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/resources) into the `/Resources` directory that is embedded within your swift application bundle or plugin module. |



| Swift Plugins                                                | Description                                                                                                                                                                                         | Usage                                          |
| ------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| [SwiftBundler](https://github.com/stackotter/swift-bundler)  | Create cross-platform Swift apps, (Xcode optional!). Turn Swift Packages into fully bundled Swift apps from the comfort of the command line - including **macOS**, **iOS**, and even **visionOS**. | `swift bundler run -p visionOS` |


### Industry Swift Targets
> [!IMPORTANT]
> As of **January 10th, 2024**, **Linux** is now supported, in addition to **Apple** platforms,
> you can grab the latest release [here](https://github.com/wabiverse/MetaverseKit/releases/tag/v1.7.7)! 🎉
> And, as of **August 3rd, 2024**, **Microsoft Windows** is now supported.

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


# Usage
##### To use MetaverseKit with Swift, add **MetaverseKit** as a **Package** dependency in your Package.swift file, [example usage](https://github.com/wabiverse/SwiftUSD/blob/main/Package.swift#L85).
```swift
dependencies: [
  .package(url: "https://github.com/wabiverse/MetaverseKit.git", from: "1.7.7"),
]
```

> [!NOTE]
> Swift is an open source programming language that is fully
> supported across **Linux** and [**Swift on Server**](https://www.swift.org/server/),
> the entire **Apple** family of devices: **macOS**, **visionOS**, **iOS**, **tvOS**, **watchOS**,
> as well as support for **Microsoft Windows**. To learn more about Swift, please visit [swift.org](https://www.swift.org).
<br>
