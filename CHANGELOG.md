# Change Log

## [0.3.0] - 2017-12-11
### Added
- Support for reading configuration from file (-c).
- Systemd support (thanks to Luuk Loeffen <luukloeffen@hotmail.com>).
- Sysfs gpio RS-485 directional control (-y, -Y, thanks to James Jarvis <jj@aprsworld.com>).

### Changed
- Migrated from Autotools to CMake (thanks to Nick Mayerhofer <nick.mayerhofer@enchant.at>).

## [0.2.3] - 2016-04-25
### Fixed
- tty mode setup on platforms with cfmakeraw available (fixes #7).

### Changed
- RTU response wait time raised to 500 ms.
- Allow 8 bit only character size in tty mode string.

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

[0.3.0]: https://github.com/3cky/mbusd/compare/v0.2.3...v0.3.0
[0.2.3]: https://github.com/3cky/mbusd/compare/v0.2.2...v0.2.3
[0.2.2]: https://github.com/3cky/mbusd/compare/v0.2.1...v0.2.2