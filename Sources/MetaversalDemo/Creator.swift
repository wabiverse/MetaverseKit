/* ----------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                            ::
 * ----------------------------------------------------------------
 * This software is Licensed under the terms of the Apache License,
 * version 2.0 (the "Apache License") with the following additional
 * modification; you may not use this file except within compliance
 * of the Apache License and the following modification made to it.
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * Trademarks. This License does not grant permission to use any of
 * its trade names, trademarks, service marks, or the product names
 * of this Licensor or its affiliates, except as required to comply
 * with Section 4(c.) of this License, and to reproduce the content
 * of the NOTICE file.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND without even an
 * implied warranty of MERCHANTABILITY, or FITNESS FOR A PARTICULAR
 * PURPOSE. See the Apache License for more details.
 *
 * You should have received a copy for this software license of the
 * Apache License along with this program; or, if not, please write
 * to the Free Software Foundation Inc., with the following address
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *         Copyright (C) 2024 Wabi Foundation. All Rights Reserved.
 * ----------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ---------------------------------------------------------------- */

import Draco
import Imath
import ImGui
import MaterialX
import OCIOBundle
import OpenColorIO
import OpenEXR
import OpenImageIO
import OpenSubdiv
import OpenTime
import OpenTimelineIO
import OpenVDB
import Ptex
#if canImport(Python) && (os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS))
  import PyBundle
  import Python
#endif // canImport(Python) && os(macOS)

/* --- xxx --- */

public typealias OCIO = OpenColorIO_v2_3
public typealias OIIO = OpenImageIO_v2_5

/* --- xxx --- */

#if canImport(AppKit) || canImport(UIKit)
  import Metal
  import MetalKit

  #if canImport(AppKit)
    public typealias OSWindow = NSWindow
    public typealias OSDelegate = NSApplicationDelegate & NSResponder
    public typealias OSApp = NSApplication
  #elseif canImport(UIKit)
    public typealias OSWindow = UIWindow
    public typealias OSDelegate = UIApplicationDelegate & UIResponder
    public typealias OSApp = UIApplication
  #endif /* canImport(UIKit) */
#else // !canImport(AppKit) || !canImport(UIKit)
  public class OSApp
  {
    public static let shared = OSApp()

    private init()
    {
      print("OSApp is not supported on this platform.")
    }

    public var delegate = AppDelegate()

    public func run()
    {
      print("OSApp is not supported on this platform.")
    }
  }

  public struct OSWindow
  {}

  public struct OSDelegate
  {}
#endif

@main
class Creator
{
  let app = OSApp.shared
  let delegate = AppDelegate()

  public init()
  {
    app.delegate = delegate
  }

  static func main()
  {
    let C: Creator

    #if canImport(Python) && (os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS))
      /* embed & init python. */
      PyBundler.shared.pyInit()
      PyBundler.shared.pyInfo()
    #endif // canImport(Python)
    print("app launched.")

    C = Creator()

    #if canImport(AppKit)
      C.app.run()
    #endif /* canImport(AppKit) */
  }

  static func setupGuiStyle(with context: UnsafeMutablePointer<ImGuiContext>? = nil)
  {
    assert(context != nil, "ImGui context is nil")

    let io = ImGui.GetIO()
    io.pointee.IniFilename = nil
    io.pointee.LogFilename = nil
    io.pointee.Fonts.pointee.Build()

    ImGui.StyleColorsDark(nil)
  }

  /* -------------------------------------------------------- */

  static func configColor()
  {
    OCIOBundler.shared.ocioInit(config: .aces)

    OCIO.GetCurrentConfig()
  }

  static func configImaging()
  {
    /* create some exr images. */
    let dtp = OIIO.TypeDesc(.init("float"))
    let fmt = OIIO.ImageSpec(512, 89, 4, dtp)

    var fg = OIIO.ImageBuf(.init(std.string("fg.exr")), fmt, OIIO.InitializePixels.Yes)
    let bg = OIIO.ImageBuf(.init(std.string("bg.exr")), fmt, OIIO.InitializePixels.Yes)

    fg.set_origin(0, 0, 0)

    if OIIO.ImageBufAlgo.fill(&fg, .init(Float(0.01)), .init(Float(0.01)), .init(Float(1.0)), .init(Float(0.01)), .init(), 0)
    {
      let comp = OIIO.ImageBufAlgo.over(fg, bg, .init(), 0)
      if !comp.has_error()
      {
        if comp.write(.init(std.string("composite.exr")), OIIO.TypeDesc(.init("UNKNOWN")), .init(""), nil, nil) == false || comp.has_error()
        {
          print("Error writing image: \(comp.geterror(true))")
        }
      }
      else
      {
        print("Error writing image: \(comp.geterror(true))")
      }
    } 
    else 
    {
      print("Error filling fg image.")
    }
  }

  /* -------------------------------------------------------- */
}

#if canImport(AppKit)
  public class AppDelegate: OSDelegate
  {
    let app = OSApp.shared
    let viewController = ViewController()
    let ctx = ImGui.CreateContext(nil)

    public var window: OSWindow?

    public func applicationDidFinishLaunching(_: Notification)
    {
      print("MetaversalDemo has launched.")

      Creator.configColor()
      Creator.configImaging()
      Creator.setupGuiStyle(with: ctx)

      let window = NSWindow(
        contentRect: NSRect(x: 0, y: 0, width: 1280, height: 720),
        styleMask: [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView],
        backing: .buffered,
        defer: false
      )
      window.title = "MetaverseKit"
      window.contentViewController = viewController
      window.makeKeyAndOrderFront(nil)

      self.window = window
    }

    public func applicationWillTerminate(_: Notification)
    {
      ImGui.DestroyContext(ctx)
    }

    public func applicationShouldTerminateAfterLastWindowClosed(_: OSApp) -> Bool
    {
      false
    }
  }

#elseif canImport(UIKit)
  public class AppDelegate: OSDelegate
  {
    let app = OSApp.shared
    let viewController = ViewController()
    let ctx = ImGui.CreateContext(nil)

    public var window: OSWindow?

    public func application(_: OSApp, didFinishLaunchingWithOptions _: [UIApplication.LaunchOptionsKey: Any]?) -> Bool
    {
      Creator.configColor()
      Creator.configImaging()
      Creator.setupGuiStyle(with: ctx)

      let window = UIWindow()
      window.makeKeyAndVisible()

      self.window = window

      return true
    }

    public func applicationWillTerminate(_: OSApp)
    {
      ImGui.DestroyContext(ctx)
    }
  }
#else /* !canImport(AppKit) && !canImport(UIKit) */
  public class AppDelegate
  {
    let app = OSApp.shared
    let ctx = ImGui.CreateContext(nil)

    public var window: OSWindow?

    public func application(_: OSApp, didFinishLaunchingWithOptions _: [String: Any]?) -> Bool
    {
      Creator.configColor()
      Creator.configImaging()
      Creator.setupGuiStyle(with: ctx)

      window = .init()

      print("app launched succesfully.")

      return true
    }

    public func applicationWillTerminate(_: OSApp)
    {
      ImGui.DestroyContext(ctx)
    }
  }
#endif /* !canImport(AppKit) && !canImport(UIKit) */
