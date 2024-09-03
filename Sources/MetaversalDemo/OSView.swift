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

#if os(visionOS) || !canImport(AppKit) && !canImport(UIKit)
  open class OSView
  {
    public init()
    {}

    public func viewDidLoad()
    {}
  }

  public typealias ViewControllerAPI = OSView

  public final class ViewController: ViewControllerAPI
  {
    private var renderer: Renderer?

    override public func viewDidLoad()
    {}
  }

#elseif canImport(Metal)
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

    override public func viewDidLoad()
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
#endif
