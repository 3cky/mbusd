#!/usr/bin/env python
#https://pymodbus.readthedocs.io/en/latest/examples/synchronous-client.html

#---------------------------------------------------------------------------#
# import the various server implementations
#---------------------------------------------------------------------------#
from pymodbus.client.sync import ModbusTcpClient
from pymodbus.client.sync import ModbusUdpClient
from pymodbus.client.sync import ModbusSerialClient

#with ModbusSerialClient(method='rtu', port='/tmp/pts1', timeout=1, baudrate=19200) as client:
PORT_WITHOUT_ROOT_REQ = 1025
with ModbusTcpClient('127.0.0.1', port=PORT_WITHOUT_ROOT_REQ) as client:
    result = client.read_holding_registers(1, 8, unit=1)
    print result.registers