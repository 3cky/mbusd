About mbusd
===========

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

Configuring and compilation:
----------------------------

<pre>
$ git clone https://github.com/3cky/mbusd.git mbusd.git
$ cd mbusd.git
$ ./configure
$ make
# make install
$ make clean
</pre>

Compilation using cmake
<pre>
$ git clone https://github.com/3cky/mbusd.git mbusd.git
$ cd mbusd.git
$ mkdir -p output.dir && cd output.dir
$ cmake ../
$ make
</pre>

Usage:
------

       mbusd [-h] [-d] [-t] [-v level] [-L logfile] [-p device] [-s speed] [-m mode] \
       	       	[-P port] [-C maxconn] [-N retries] [-R pause] [-W wait] [-T timeout]

       -h     Usage help.
       -d     Instruct mbusd not to fork itself (non-daemonize).
       -t     Enable RTS RS-485 data direction control (if not disabled while compile).
       -y file
              Enable RS-485 direction data direction control by writing '1' to file
              for transmitter enable and '0' to file for transmitter disable
       -Y file
              Enable RS-485 direction data direction control by writing '0' to file
              for transmitter enable and '1' to file for transmitter disable
       -v level
              Specifies  log  verbosity level (0 for errors only, 1 for warnings
              and 2 for also information messages.) If mbusd was compiled in debug mode,
              valid log levels are up to 9, where log levels above 2 forces
              logging of information about additional internal events.
       -L logfile
              Specifies log file name ('-' for logging to STDOUT only, default is /var/log/mbusd.log).
       -p device
              Specifies serial port device name.
       -s speed
              Specifies serial port speed.
       -m mode
              Specifies serial port mode (like 8N1).
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

Reporting bugs:
---------------

Please file [issue](https://github.com/3cky/mbusd/issues) with attached debug log in verbose (`-v9`) mode, i.e.:

       # mbusd -L/tmp/mbusd.log -p /dev/ttyUSB0 -s 9600 -P 502 -d -v9

Unless you were prompted so or there is another pertinent reason (e.g. GitHub fails to accept the bug report), please do not send bug reports via personal email.

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

Credits:
--------

Andrew Denysenko (<nitr0@seti.kr.ua>):
 - RTS RS-485 data direction control
 - RTU response receiving by length

James Jarvis (<jj@aprsworld.com>):
 - file based RS-485 data direction control

License:
--------

This project is distributed under the BSD license. See the [LICENSE](LICENSE) file for the full license text.
