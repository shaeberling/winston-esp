Subset of the tools from nkolban:
https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils

## Changes
- Added `CMakeLists.txt`.
- Changed directory structure to match components.
- Renamed the class to avoid conflicts
- Fixed an issue where the address given in the constructor wasn't used.
- Added now  (IDF 4.3) required `conf.clk_flags` to i2c config
- Fixed some includes.
- Made some logging more verbose.