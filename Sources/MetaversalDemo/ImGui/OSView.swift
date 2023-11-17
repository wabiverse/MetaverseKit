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

#if canImport(AppKit) || canImport(UIKit)
  import Metal
  import MetalKit

  #if canImport(AppKit)
    public typealias ViewControllerAPI = NSViewController
  #elseif canImport(UIKit)
    public typealias ViewControllerAPI = UIViewController
  #endif

  public final class ViewController: ViewControllerAPI
  {
    private var renderer: Renderer?

    public override func viewDidLoad()
    {
      super.viewDidLoad()

      print("view did load")

      guard let metalKitView = view as? MTKView else { return }

      metalKitView.enableSetNeedsDisplay = true
      metalKitView.device = MTLCreateSystemDefaultDevice()
      metalKitView.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0)

      renderer = metalKitView.device.flatMap(Renderer.init)
      metalKitView.delegate = renderer
    }
  }
#else
  open class OSView
  {
    public init()
    {}

    public func viewDidLoad()
    {}
  }

  public typealias ViewControllerAPI = OSView
#endif
