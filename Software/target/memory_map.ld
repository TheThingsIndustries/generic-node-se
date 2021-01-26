/** Copyright © 2021 The Things Industries B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/* Memories definition */
MEMORY
{
  BOOTROM  (rx)  : ORIGIN = 0x08000000, LENGTH = 0x00000B000   /* Flash memory dedicated to bootloader */
  APPROM   (rx)  : ORIGIN = 0x0800B000, LENGTH = 0x000035000   /* Flash memory dedicated to application */
  RAM1   (xrw)   : ORIGIN = 0x20000000, LENGTH = 32K
  RAM2   (xrw)   : ORIGIN = 0x20008000, LENGTH = 32K
}

__BOOTROM_START__ = ORIGIN(BOOTROM);
__BOOTROM_SIZE__ = LENGTH(BOOTROM);
__APPROM_START__ = ORIGIN(APPROM);
__APPROM_SIZE__ = LENGTH(APPROM);
