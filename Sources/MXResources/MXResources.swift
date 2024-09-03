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

#if canImport(Metal)
  import Metal
#endif /* canImport(Metal) */

/**
 * # MaterialX Resource Manager
 *
 * The ``MXResourceManager`` is a singleton class that manages all
 * the **resources** and **standard data libraries** for **MaterialX**,
 * providing **declarations** and **graph definitions** for each of the
 * **MaterialX** nodes, and **source code** for all the supported **shader
 * generators**.
 *
 * This singleton is designed for clients to use the ``shared`` instance
 * of this class within their own apps and plugins, usage is as follows:
 * ````
 * let mxResourceManager = MXResourceManager.shared
 * mxResourceManager.loadStandardLibraries()
 * ````
 *
 * - **Notice**: This singleton class is not yet implemented, it currently
 * serves as a stub for the ``MaterialX`` target, which through **SwiftPM**,
 * automatically includes all of these resources since it's a transitive
 * dependency of the ``SwiftUSD` target, when clients add the ``SwiftUSD``
 * package to their own apps & plugins, or other projects, they will also
 * automatically have all of these resources installed to the app's bundle
 * or library module's respective ``Resources`` folder for direct consumption.
 *
 * The **standard data libraries** that are bundled are the sames ones from the
 * [**MaterialX Standard Library**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries).
 *
 *
 * Additionally, we have included the
 * [**MaterialX Resources**](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/resources)
 * as well. */
public class MXResourceManager
{
  public static let shared = MXResourceManager()

  private init()
  {}
}
