# MetaverseKit
<image src="https://miro.medium.com/v2/resize:fit:1400/1*1XgNguAZ9mIMPPGdVRsTwQ.gif">

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

## Wabi Swift Targets
- **MetaPy** - Metaversal Python, all the python modules from the below libraries, combined into a single swift target.
- **MXGraphEditor** - MaterialX Graph Editor, visualizing, creating, and editing MaterialX graphs on the flick of a `swift run MXGraphEditor` command.
- **MXResources** - MaterialX Resources, adding this target to your swift app or plugin will automatically install the [**MaterialX Standard Library**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries) and [**MaterialX Resources**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/resources) into
  the `/Resources` directory that is embedded within your swift application bundle or plugin module.
- With many more coming soon...

## Industry Swift Targets
- **OneTBB v2021.10.0** - ([*Official support in progress...*](https://github.com/oneapi-src/oneTBB/issues/1244))
- **Boost v1.81.2**
- **Python v3.11.6**
- **IMath v3.1.9**
- **Alembic v1.8.5**
- **MaterialX v1.38.8**
- **OpenColorIO v2.3.0**
- **Blosc v1.21.5**
- **GLFW v3.4.0**
- **ImGui v1.89.9**
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
##### To use MetaverseKit with Swift, add **MetaverseKit** as a **Package** dependency in your Package.swift file, [example usage](https://github.com/wabiverse/SwiftUSD/blob/main/Package.swift#L85).
```swift
dependencies: [
  .package(url: "https://github.com/wabiverse/MetaverseKit.git", from: "1.1.6"),
]
```

> **NOTE:** Swift is an open source programming language that is fully
supported across **Linux** and [**Swift on Server**](https://www.swift.org/server/),
the entire **Apple** family of devices: **macOS**, **visionOS**, **iOS**, **tvOS**, **watchOS**,
as well as support for **Microsoft Windows**. To learn more about Swift, please visit [swift.org](https://www.swift.org).
<br>