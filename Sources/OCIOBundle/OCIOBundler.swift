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

import Foundation
import OpenColorIO

public class OCIOBundler
{
  public static var shared = OCIOBundler()

  private init()
  {
    ocioInit()
  }

  public var config: OpenColorIO_v2_3.ConstConfigRcPtr?

  public func ocioInit(config ocio: OCIOConfigProfileType = .aces)
  {
    guard let ocioConfig = ocio.config
    else { return print("Could not find OCIO config.") }

    /* setup ocio color config. */
    setenv("OCIO", ocioConfig, 1)

    config = OpenColorIO_v2_3.GetCurrentConfig()
  }

  public func ocioInfo()
  {
    guard let ocioConfig = config
    else { return }

    print(ocioConfig)
  }
}

public enum OCIOConfigProfileType
{
  case aces
  case agx

  public var config: String?
  {
    switch self
    {
      case .aces: Bundle.module.path(forResource: "colormanagement/ACES/studio-config-v2.1.0_aces-v1.3_ocio-v2.3.ocio", ofType: nil)
      case .agx: Bundle.module.path(forResource: "colormanagement/AgX/config.ocio", ofType: nil)
    }
  }
}
