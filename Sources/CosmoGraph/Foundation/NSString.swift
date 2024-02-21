/* --------------------------------------------------------------
 * :: :  C  O  S  M  O  G  R  A  P  H  :                       ::
 * --------------------------------------------------------------
 * @wabiverse :: metaversekit :: cosmograph
 *
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
 *                            Copyright (C) 2023 Wabi Foundation.
 *                                           All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

#if os(macOS) || os(visionOS) || os(iOS) || os(watchOS) || os(tvOS)

  import Apple
  import CxxStdlib
  import Foundation

  public extension NS.String
  {
    private borrowing func getUtf8StringCopy() -> std.string
    {
      std.string(__utf8StringUnsafe())
    }

    var utf8String: std.string
    {
      getUtf8StringCopy()
    }
  }

#endif /* os(macOS) || os(visionOS) || os(iOS) || os(watchOS) || os(tvOS) */
