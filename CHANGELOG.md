# Change Log

## [0.2.2] - 2015-12-18
### Fixed
- Function code 6 response reading error #3.

## 0.2.1 - 2015-06-08
### Added
- Serial port mode configuration option (-m).

## 0.2.0 - 2015-02-24
### Added
- RTS RS-485 transmit/receive control (thanks to Andrew Denysenko <nitr0@seti.kr.ua>).
- RTU response receiving by length (thanks to Andrew Denysenko <nitr0@seti.kr.ua>).

### Changed
- Standard error codes for receive error (04) and timeout (0x0B).

## 0.1.3 - 2013-11-18
### Fixed
- Serial interface initialization under Linux.
- x86_64 build.

### Changed
- Default RTU client response wait timeout increased to 50 ms.

### Added
- Eclipse CDT project files.

## 0.1.2 - 2003-09-26
### Fixed
- Compilation under CYGWIN.

## 0.1.1 - 2003-09-13
### Initial release

[0.2.2]: https://github.com/3cky/mbusd/compare/v0.2.1...v0.2.2