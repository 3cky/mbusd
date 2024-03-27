# Changelog

## [0.5.2] - 2024-03-27
- Make use of Linux RS-485 support (#97).
- Add flag `-r` to support inverted RTS flow control(#98).
- Logging fixes and enhancements (#90).
- Default log file name changed to `/var/log/mbusd.log`.
- Fix building on older gcc versions (#100).
- Add docker image publish github workflow.

## [0.5.1] - 2022-08-18
- Add 'reply on broadcast' feature (#75).
- Fix for "Resource temporarily unavailable" error in tty read() (#78).
- Add simple Dockerfile for running mbusd containerized (#79).
- Add support for more complex serial port device names (#81).
- Do not segfault when closing last connection (#83).
- Fix crash due to missing logw() argument (#84).
## [0.5.0] - 2020-11-28
### Added
- Command-line and config options to set an TCP socket address (#53).
- IPv6 support (#53).
- Support for RTU broadcast messages (#61).
- More baudrates (#62).

### Fixed
- Added online network requirements to the systemd service file (#58).
- Take into account tty parity bit in the timing calculations (#59).
- Show an error when trying to use unsupported baudrate (#63).

## [0.4.0] - 2019-07-08
### Added
- Modbus RTU Exception Forwarding (#29).
- Handle incorrect Modbus/TCP data length header field (#31).
- tty reconnect with an exponential backoff.

### Fixed
- Modbus: read only a single frame at a time (#43).
- Readable socket never gets read, conn_loop degrades into busy loop (#52).

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

[0.5.2]: https://github.com/3cky/mbusd/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/3cky/mbusd/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/3cky/mbusd/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/3cky/mbusd/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/3cky/mbusd/compare/v0.2.3...v0.3.0
[0.2.3]: https://github.com/3cky/mbusd/compare/v0.2.2...v0.2.3
[0.2.2]: https://github.com/3cky/mbusd/compare/v0.2.1...v0.2.2

