#!/usr/bin/env python

import random
import unittest
import sys

from pymodbus.client.sync import ModbusTcpClient
from pymodbus.pdu import ExceptionResponse
from pymodbus.bit_read_message import ReadDiscreteInputsResponse, ReadCoilsResponse
from pymodbus.bit_write_message import WriteMultipleCoilsResponse, WriteSingleCoilResponse
from pymodbus.register_read_message import ReadInputRegistersResponse, ReadHoldingRegistersResponse
from pymodbus.register_write_message import WriteMultipleRegistersResponse, WriteSingleRegisterResponse

from environment.rtu_slave import ModbusSerialServer

MBUSD_PORT = 1025


class TestModbusRequests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.log = logging.getLogger("TestModbusRequests")

        cls.mbs = ModbusSerialServer()
        cls.mbs.start()

        cls.client = ModbusTcpClient('127.0.0.1', port=MBUSD_PORT)
        cls.client.connect()

    @classmethod
    def tearDownClass(cls):
        cls.log.info("test teardown")
        cls.client.close()
        try:
            cls.mbs.kill()
        except e:
            cls.log.info("Fetched exception during mbs.kill")


    def test_coils(self):
        bits = [random.randrange(2)>0 for i in range(8)]

        # 15 Write Multiple Coils
        result = self.client.write_coils(0, bits)
        self.assertIsInstance(result, WriteMultipleCoilsResponse, result)
        self.assertEqual(result.address, 0, result)
        self.assertEqual(result.count, 8, result)

        # 01 Read Coils
        result = self.client.read_coils(0, 8)
        self.assertIsInstance(result, ReadCoilsResponse, result)
        self.assertEqual(result.bits, bits, result)

        # 05 Write Single Coil
        bit1 = not bits[0]
        result = self.client.write_coil(0, bit1)
        self.assertIsInstance(result, WriteSingleCoilResponse, result)
        self.assertEqual(result.address, 0, result)
        self.assertEqual(result.value, bit1, result)
        result = self.client.read_coils(0, 1)
        self.assertIsInstance(result, ReadCoilsResponse, result)
        self.assertEqual(result.bits[0], bit1, result)

    def test_discreteInputs(self):
        # 02 Read Discrete Inputs
        result = self.client.read_discrete_inputs(0, 8)
        self.assertIsInstance(result, ReadDiscreteInputsResponse, result)
        self.assertEqual(result.bits, [True]*8, result)

    def test_inputRegisters(self):
        # 04 Read Input Registers
        result = self.client.read_input_registers(0, 8)
        self.assertIsInstance(result, ReadInputRegistersResponse, result)
        self.assertEqual(result.registers, list(range(8)), result)

    def test_holdingRegisters(self):
        registers = [random.randrange(8) for i in range(8)]

        # 16 Write Multiple Holding Registers
        result = self.client.write_registers(0, registers)
        self.assertIsInstance(result, WriteMultipleRegistersResponse, result)
        self.assertEqual(result.address, 0, result)
        self.assertEqual(result.count, 8, result)

        # 03 Read Multiple Holding Registers
        result = self.client.read_holding_registers(0, 8)
        self.assertIsInstance(result, ReadHoldingRegistersResponse, result)
        self.assertEqual(result.registers, registers, result)

        # 06 Write Single Holding Register
        register1 = (registers[0] + 1) % 65535
        result = self.client.write_register(0, register1)
        self.assertIsInstance(result, WriteSingleRegisterResponse, result)
        self.assertEqual(result.address, 0, result)
        self.assertEqual(result.value, register1, result)
        result = self.client.read_holding_registers(0, 1)
        self.assertIsInstance(result, ReadHoldingRegistersResponse, result)
        self.assertEqual(result.registers[0], register1, result)

    def test_exception(self):
        result = self.client.write_coil(1000, False) # invalid address 1000
        self.assertIsInstance(result, ExceptionResponse, result)
        self.assertEqual(result.original_code, 5, result) # fc05 Write Single Coil
        self.assertEqual(result.exception_code, 2, result) # Illegal Data Address


if __name__ == '__main__':
    import logging
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(level=logging.DEBUG,
                        format=u'[%(asctime)s] %(name)-26s-%(levelname)-5s %(funcName)-20s:%(lineno)-4d \033[35m%(message)s\033[0m',
                        datefmt='%d.%m. %H:%M:%S',
                        handlers=[stdout_handler])
    unittest.main(verbosity=2)