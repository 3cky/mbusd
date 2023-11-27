About mbusd
===========

[![mbusd](https://github.com/3cky/mbusd/actions/workflows/build.yml/badge.svg)](https://github.com/3cky/mbusd/actions/workflows/build.yml)

**mbusd** is open-source [Modbus TCP to Modbus RTU (RS-232/485)](https://en.wikipedia.org/wiki/Modbus)
gateway. It presents a network of RTU slaves as single TCP slave.

That is a TCP-Slave (or server) which acts as a RTU-master to get data from Modbus RTU-slave devices.

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

       mbusd [-h] [-d] [-L logfile] [-v level] [-c cfgfile] 
             [-p device] [-s speed] [-m mode] [-S]
             [-t] [-r] [-y sysfsfile] [-Y sysfsfile] 
             [-A address] [-P port] [-C maxconn] [-N retries]
             [-R pause] [-W wait] [-T timeout] [-b]

       -h     Usage help.
       -d     Instruct mbusd not to fork itself (non-daemonize).
       -L logfile
              Specifies log file name ('-' for logging to STDOUT only, relative path or bare filename
              will be stored at /var/log, default is /var/log/mbusd.log).
       -v level
              Specifies log verbosity level (0 for errors only, 1 for warnings and 2 for informational 
              messages also). If mbusd was compiled in debug mode, valid log levels are up to 9, 
              where log levels above 2 adds logging of information about additional internal events.
       -c cfgfile
              Read configuration from cfgfile.
       -p device
              Specifies serial port device name.
       -s speed
              Specifies serial port speed.
       -m mode
              Specifies serial port mode (like 8N1).
       -S     Enable RS-485 support for given serial port device (Linux only)
       -t     Enable RTS RS-485 data direction control using RTS, active transmit.
       -r     Enable RTS RS-485 data direction control using RTS, active receive.
       -y file
              Enable RS-485 direction data direction control by writing '1' to file
              for transmitter enable and '0' to file for transmitter disable.
       -Y file
              Enable RS-485 direction data direction control by writing '0' to file
              for transmitter enable and '1' to file for transmitter disable.
       -A address
             Specifies TCP server address to bind (default is 0.0.0.0).
       -P port
              Specifies TCP server port number (default is 502).
       -C maxconn
              Specifies maximum number of simultaneous TCP connections (default is 32).
       -N retries
              Specifies maximum number of request retries (0 disables retries, default is 3).
       -R pause
              Specifies pause between requests in milliseconds (default is 100ms).
       -W wait
              Specifies response wait time in milliseconds (default is 500ms).
       -T timeout
              Specifies connection timeout value in seconds (0 disables timeout, default is 60).
       -b
              Instructs mbusd to reply on a broadcast.

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

where `<serial port>` is escaped serial port device short name (like `ttyUSB0` for `/dev/ttyUSB0` device name or `serial-rs485` for `/dev/serial/rs485` device name).

**mbusd** started by systemd will read its configuration from file named `/etc/mbusd/mbusd-<serial port>.conf`.
This way it's possible to run multiple **mbusd** instances with different configurations.

To see the **mbusd** service status:

	# systemctl status mbusd@<serial port>.service

To monitor the **mbusd** service:

	# journalctl -u mbusd@<serial port>.service -f -n 10

To start the **mbusd** service on system boot:

	# systemctl enable mbusd@<serial port>.service

Please check systemd documentation for other usefull systemd [commands](https://wiki.archlinux.org/index.php/systemd)

Contributing:
-------------

### Reporting bugs

Please file [issue](https://github.com/3cky/mbusd/issues) with attached debug log in verbose (`-v9`) mode, i.e.:

       # mbusd -L/tmp/mbusd.log -p /dev/ttyUSB0 -s 9600 -P 502 -d -v9

Unless you were prompted so or there is another pertinent reason (e.g. GitHub fails to accept the bug report),
please do not send bug reports via personal email.

### Workflow for code contributions

1. Fork it and clone forked repository
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Make your changes
4. Commit your changes (`git commit -am 'Add some feature'`)
5. Push to the branch (`git push origin my-new-feature`)
6. Create new Pull Request

### Building and Testing

Dependencies: please see the correct OS-distribution section in the
 [.gitlab-ci.yml](https://github.com/3cky/mbusd/blob/master/.gitlab-ci.yml)

With all dependencies met, one is able to *build and execute tests*
issuing the following *bash* commands:
```
# build
mkdir output.dir/ && cd $_
cmake ../ && make
# execute all tests
(cd ../ && python tests/run_itests.py output.dir/mbusd)
```

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
