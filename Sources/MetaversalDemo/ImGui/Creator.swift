/* --------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                          ::
 * --------------------------------------------------------------
 * This program is free software; you can redistribute it, and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. Check out
 * the GNU General Public License for more details.
 *
 * You should have received a copy for this software license, the
 * GNU General Public License along with this program; or, if not
 * write to the Free Software Foundation, Inc., to the address of
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *       Copyright (C) 2023 Wabi Foundation. All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

import ImGui
import Python
import PyBundle

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
  #endif // canImport(UIKit)
#endif // canImport(AppKit) || canImport(UIKit)

@main
class Creator
{
  let app = OSApp.shared
  let delegate = AppDelegate()

  public init()
  {
    app.delegate = self.delegate
  }

  static func main()
  {
    let C: Creator

    /* embed & init python. */
    PyBundle.shared.pyInit()
    PyBundle.shared.pyInfo()

    C = Creator()
    
    C.app.run()
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
}

#if canImport(AppKit)
  public class AppDelegate: OSDelegate
  {
    let app = OSApp.shared
    let viewController = ViewController()
    let ctx = ImGui.CreateContext(nil)

    var window: OSWindow?

    public func applicationDidFinishLaunching(_ notification: Notification)
    {
      print("MetaversalDemo has launched.")

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
  @main
  class AppDelegate: OSDelegate
  {
    let app = OSApp.shared
    let viewController = ViewController()
    let ctx = ImGui.CreateContext(nil)

    var window: OSWindow?

    func application(_: OSApp, didFinishLaunchingWithOptions _: [UIApplication.LaunchOptionsKey: Any]?) -> Bool
    {
      Creator.setupGuiStyle(with: ctx)

      let window = UIWindow(frame: UIScreen.main.bounds)
      window.rootViewController = ViewController()
      window.makeKeyAndVisible()
      self.window = window
    }

    func applicationWillTerminate(_: OSApp)
    {
      ImGui.DestroyContext(ctx)
    }
  }
#endif /* !canImport(AppKit) || !canImport(UIKit) */