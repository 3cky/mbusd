About mbusd
===========

[![Build Status](https://travis-ci.org/3cky/mbusd.svg?branch=master)](https://travis-ci.org/3cky/mbusd)

**mbusd** is open-source [Modbus TCP to Modbus RTU (RS-232/485)](https://en.wikipedia.org/wiki/Modbus) gateway.

Features:
---------

* Small footprint - suitable to run on embedded devices and SBCs like Raspberry Pi
* Multi-master - multiple TCP masters can access slave devices in RTU network using same gateway
* Robustness - can retry requests with mismatched response CRC
* Flexible RTU modes - speed/parity/stop-bits/timeouts can be configured for RTU network
* Support for both of automatic and manual (using RTS bit) direction control types for RS-485 transceivers

Supported function codes:
-------------------------

* 01: Read coil status
* 02: Read input status
* 03: Read holding registers
* 04: Read input registers
* 05: Force single coil
* 06: Preset single register
* 07: Read exception status
* 15: Force multiple coils
* 16: Preset multiple registers

Please note all other function codes (including vendor-specific extensions) are supported on a "best-effort" basis and most likely will fail.

Installation instructions:
----------------------------

```
$ git clone https://github.com/3cky/mbusd.git mbusd.git
$ cd mbusd.git
$ mkdir -p build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
$ sudo make install
```

***Compile time options***
can be altered in many ways, e.g. by using the following tools in the `build` dir:
* ccmake - usually in the package cmake-curses-gui
* cmake-gui - usually in the package cmake-qt-gui

Usage:
------

       mbusd [-h] [-d] [-L logfile] [-v level] [-c cfgfile] [-p device] [-s speed] [-m mode]
       		[-t] [-y file] [-Y file] [-P port] [-C maxconn] [-N retries]
       		[-R pause] [-W wait] [-T timeout]

       -h     Usage help.
       -d     Instruct mbusd not to fork itself (non-daemonize).
       -L logfile
              Specifies log file name ('-' for logging to STDOUT only, default is /var/log/mbusd.log).
       -v level
              Specifies  log  verbosity level (0 for errors only, 1 for warnings
              and 2 for also information messages.) If mbusd was compiled in debug mode,
              valid log levels are up to 9, where log levels above 2 forces
              logging of information about additional internal events.
       -c cfgfile
              Read configuration from cfgfile.
       -p device
              Specifies serial port device name.
       -s speed
              Specifies serial port speed.
       -m mode
              Specifies serial port mode (like 8N1).
       -t     Enable RTS RS-485 data direction control (if not disabled while compile).
       -y file
              Enable RS-485 direction data direction control by writing '1' to file
              for transmitter enable and '0' to file for transmitter disable
       -Y file
              Enable RS-485 direction data direction control by writing '0' to file
              for transmitter enable and '1' to file for transmitter disable
       -P port
              Specifies TCP port number (default 502).
       -C maxconn
              Specifies maximum number of simultaneous TCP connections.
       -N retries
              Specifies maximum number of request retries (0 disables retries).
       -R pause
              Specifies pause between requests in milliseconds.
       -W wait
              Specifies response wait time in milliseconds.
       -T timeout
              Specifies connection timeout value in seconds (0 disables timeout).

Please note running **mbusd** on default Modbus TCP port (502) requires root privileges!

Configuration file:
-------------------
**mbusd** can read the configuration from a file specified by `-c` command line flag.
Please see [example configuration file](conf/mbusd.conf.example)
for complete list of available configuration options.

systemd:
---------------

**mbusd** has [systemd](https://wiki.archlinux.org/index.php/systemd) support.
The build system detects whether the system has systemd after which `sudo make install`
installs the `mbusd@.service` file on systems with systemd active.

The **mbusd** service can be started via:

	# systemctl start mbusd@<serial port>.service

where `<serial port>` is serial port device name (like `ttyUSB0`).

**mbusd** started by systemd will read its configuration from file named `/etc/mbusd/mbusd-<serial port>.conf`.
This way it's possible to run multiple **mbusd** instances with different configurations.

To see the **mbusd** service status:

	# systemctl status mbusd@<serial port>.service

To monitor the **mbusd** service:

	# journalctl -u mbusd@<serial port>.service -f -n 10

To start the **mbusd** service on system boot:

	# systemctl enable mbusd@<serial port>.service

Please check systemd documentation for other usefull systemd [commands](https://wiki.archlinux.org/index.php/systemd)

Reporting bugs:
---------------

Please file [issue](https://github.com/3cky/mbusd/issues) with attached debug log in verbose (`-v9`) mode, i.e.:

       # mbusd -L/tmp/mbusd.log -p /dev/ttyUSB0 -s 9600 -P 502 -d -v9

Unless you were prompted so or there is another pertinent reason (e.g. GitHub fails to accept the bug report),
please do not send bug reports via personal email.

Contributing:
-------------

1. Fork it and clone forked repository
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Make your changes
4. Commit your changes (`git commit -am 'Add some feature'`)
5. Push to the branch (`git push origin my-new-feature`)
6. Create new Pull Request

Author:
-------

Victor Antonovich (<v.antonovich@gmail.com>)

Contributors:
-------------

Andrew Denysenko (<nitr0@seti.kr.ua>):
 - RTS RS-485 data direction control
 - RTU response receiving by length

James Jarvis (<jj@aprsworld.com>):
 - file based RS-485 data direction control

Luuk Loeffen (<luukloeffen@hotmail.com>):
 - systemd support

Nick Mayerhofer (<nick.mayerhofer@enchant.at>):
 - CMake build system

License:
--------

This project is distributed under the BSD license. See the [LICENSE](LICENSE) file for the full license text.
