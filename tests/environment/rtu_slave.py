#!/usr/bin/env python

'''
Pymodbus Asynchronous Server Example
--------------------------------------------------------------------------

The asynchronous server is a high performance implementation using the
twisted library as its backend.  This allows it to scale to many thousands
of nodes which can be helpful for testing monitoring software.
'''
import logging
#---------------------------------------------------------------------------#
# import the various server implementations
#---------------------------------------------------------------------------#
#from pymodbus.server.async import StartTcpServer
#from pymodbus.server.async import StartUdpServer
from pymodbus.server import StartSerialServer

from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext
from pymodbus.transaction import ModbusRtuFramer, ModbusAsciiFramer

from os import readlink

class ModbusSerialServer:
    #---------------------------------------------------------------------------#
    # configure the service logging
    #---------------------------------------------------------------------------#
    log = logging.getLogger("ModbusServer")

    serialPort = readlink('/tmp/pts1')

    def _start_rtu_server(self, framer=ModbusRtuFramer):
        # @req an open and existing /tmp/pts0 is required
        #---------------------------------------------------------------------------#
        # initialize your data store
        #---------------------------------------------------------------------------#
        # The datastores only respond to the addresses that they are initialized to.
        # Therefore, if you initialize a DataBlock to addresses from 0x00 to 0xFF, a
        # request to 0x100 will respond with an invalid address exception. This is
        # because many devices exhibit this kind of behavior (but not all)::
        #
        #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
        #
        # Continuing, you can choose to use a sequential or a sparse DataBlock in
        # your data context.  The difference is that the sequential has no gaps in
        # the data while the sparse can. Once again, there are devices that exhibit
        # both forms of behavior::
        #
        #     block = ModbusSparseDataBlock({0x00: 0, 0x05: 1})
        #     block = ModbusSequentialDataBlock(0x00, [0]*5)
        #
        # Alternately, you can use the factory methods to initialize the DataBlocks
        # or simply do not pass them to have them initialized to 0x00 on the full
        # address range::
        #
        #     store = ModbusSlaveContext(di = ModbusSequentialDataBlock.create())
        #     store = ModbusSlaveContext()
        #
        # Finally, you are allowed to use the same DataBlock reference for every
        # table or you you may use a seperate DataBlock for each table. This depends
        # if you would like functions to be able to access and modify the same data
        # or not::
        #
        #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
        #     store = ModbusSlaveContext(di=block, co=block, hr=block, ir=block)
        #
        # The server then makes use of a server context that allows the server to
        # respond with different slave contexts for different unit ids. By default
        # it will return the same context for every unit id supplied (broadcast
        # mode). However, this can be overloaded by setting the single flag to False
        # and then supplying a dictionary of unit id to context mapping::
        #
        #     slaves  = {
        #         0x01: ModbusSlaveContext(...),
        #         0x02: ModbusSlaveContext(...),
        #         0x03: ModbusSlaveContext(...),
        #     }
        #     context = ModbusServerContext(slaves=slaves, single=False)
        #
        # The slave context can also be initialized in zero_mode which means that a
        # request to address(0-7) will map to the address (0-7). The default is
        # False which is based on section 4.4 of the specification, so address(0-7)
        # will map to (1-8)::
        #
        #     store = ModbusSlaveContext(..., zero_mode=True)
        #---------------------------------------------------------------------------#
        store = ModbusSlaveContext(
            di = ModbusSequentialDataBlock(0, [True]*8), # discrete inputs
            co = ModbusSequentialDataBlock(0, [False]*8), # coils
            hr = ModbusSequentialDataBlock(0, [0]*8), # holding regs
            ir = ModbusSequentialDataBlock(0, list(range(8))), # input regs
            zero_mode=True) # request(0-7) will map to the address (0-7)
        context = ModbusServerContext(slaves={1: store}, single=False)

        #---------------------------------------------------------------------------#
        # initialize the server information
        #---------------------------------------------------------------------------#
        # If you don't set this or any fields, they are defaulted to empty strings.
        #---------------------------------------------------------------------------#
        identity = ModbusDeviceIdentification()
        identity.VendorName  = 'Pymodbus'
        identity.ProductCode = 'PM'
        identity.VendorUrl   = 'http://github.com/bashwork/pymodbus/'
        identity.ProductName = 'Pymodbus Server'
        identity.ModelName   = 'Pymodbus Server'
        identity.MajorMinorRevision = '1.0'

        #---------------------------------------------------------------------------#
        # run the server you want
        #---------------------------------------------------------------------------#
        #StartTcpServer(context, identity=identity, address=("localhost", 5020))
        #StartUdpServer(context, identity=identity, address=("localhost", 502))
        StartSerialServer(context=context, identity=identity, port=self.serialPort, baudrate=19200, framer=framer, broadcast_enable=True)
        #StartSerialServer(context, identity=identity, port='/dev/pts/3', framer=ModbusAsciiFramer)

    p = None
    def start(self):
        from multiprocessing import Process

        self.p = Process(target=self._start_rtu_server) #args=('bob',)
        self.p.daemon = True
        self.p.start()
        print("p.start done")

    def kill(self):
        self.log.info("Going to terminate the process, this could throw exceptions")
        if self.p is not None:
            self.p.terminate()


if __name__ == '__main__':
    mbs = ModbusSerialServer()
    mbs._start_rtu_server()

    #mbs.start()

    #import time
    #time.sleep(3600)
    #mbs.kill()

