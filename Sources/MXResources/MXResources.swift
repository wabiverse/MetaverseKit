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

import Metal

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

  public init()
  {}
}
