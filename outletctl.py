#!/usr/bin/env python3

import sys
import logging
import unittest

from usb.core import USBError
from usb.core import find
from usb.util import CTRL_IN
from usb.util import CTRL_RECIPIENT_DEVICE
from usb.util import CTRL_TYPE_VENDOR


LOG = logging.getLogger(__name__)
LOG.addHandler(logging.NullHandler())
OUTLET_USB_VENDOR_ID = 0x2306
OUTLET_USB_PRODUCT_ID = 0x1107
OUTLET_CMD_INVAL = 0x00
OUTLET_CMD_COUNT = 0x01
OUTLET_CMD_STATUS_GET = 0x02
OUTLET_CMD_STATUS_SET = 0x03
OUTLET_CMD_STATUS_TOGGLE = 0x04
OUTLET_CMD_GROUP_GET = 0x05
OUTLET_CMD_GROUP_SET = 0x06
OUTLET_CMD_MAX = 0x07
OUTLET_ERR_OK = 0x00
OUTLET_ERR_CMD = 0x01
OUTLET_ERR_ARG = 0x02


class OutletError(Exception):
    MESSAGES = {
        OUTLET_ERR_CMD: 'unknown command',
        OUTLET_ERR_ARG: 'invalid relay number',
    }
    @classmethod
    def from_code(cls, code):
        return cls(cls.MESSAGES.get(code, 'unknown error code: %r' % code))


class OutletControl:

    def __init__(self):
        LOG.debug('probing usb devices')
        self.device = find(
            idVendor=OUTLET_USB_VENDOR_ID, idProduct=OUTLET_USB_PRODUCT_ID)
        LOG.info('found device: %r', self.device)
        if not self.device:
            raise USBError('cannot find USB device')

    REQ_TYPE = CTRL_IN | CTRL_RECIPIENT_DEVICE | CTRL_TYPE_VENDOR

    def _send_command(self, cmd_code, outlet=0, param=0):
        LOG.debug(
            'ctrl request: bmRequestType=0x%02x bRequest=0x%02x wValue=%d wIndex=%d wLength=%d',
            self.REQ_TYPE, cmd_code, outlet, param, 4)
        cmd, err, arg, _ = self.device.ctrl_transfer(
            self.REQ_TYPE, cmd_code, outlet, param, 4)
        LOG.debug('ctrl response: cmd=0x%02x err=0x%02x arg=%d', cmd, err, arg)
        if cmd != cmd_code:
            raise OutletError(
                'invalid response cmd: %r (expected %r)' % (cmd, cmd_code))
        if err != OUTLET_ERR_OK:
            raise OutletError.from_code(err)
        return arg

    def get_count(self):
        LOG.info('OUTLET_CMD_COUNT')
        return self._send_command(OUTLET_CMD_COUNT)

    def is_on(self, outlet: int):
        LOG.info('OUTLET_CMD_STATUS_GET: outlet=%d', outlet)
        return bool(self._send_command(OUTLET_CMD_STATUS_GET, outlet))

    def turn_on(self, outlet: int):
        self._status_set(outlet, 1)

    def turn_off(self, outlet: int):
        self._status_set(outlet, 0)

    def _status_set(self, outlet: int, status: int):
        LOG.info('OUTLET_CMD_STATUS_SET: outlet=%d status=%d', outlet, status)
        self._send_command(OUTLET_CMD_STATUS_SET, outlet, status)

    def toggle(self, outlet: int):
        LOG.info('OUTLET_CMD_STATUS_TOGGLE: outlet=%d', outlet)
        return self._send_command(OUTLET_CMD_STATUS_TOGGLE, outlet)

    def get_group(self, outlet: int):
        LOG.info('OUTLET_CMD_GROUP_GET: outlet=%d', outlet)
        return self._send_command(OUTLET_CMD_GROUP_GET, outlet)

    def set_group(self, outlet: int, group: int):
        LOG.info('OUTLET_CMD_GROUP_SET: outlet=%d group=%d', outlet, group)
        return self._send_command(OUTLET_CMD_GROUP_SET, outlet, group)


class OutletControlTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.ctl = OutletControl()
        for r in range(cls.ctl.get_count()):
            cls.ctl.turn_off(r)
            cls.ctl.set_group(r, r)

    def test_count(self):
        self.assertGreater(self.ctl.get_count(), 0)

    def test_status_get(self):
        self.assertFalse(self.ctl.is_on(0))

    def test_status_set_get(self):
        for r in range(self.ctl.get_count()):
            self.ctl.turn_on(r)
            self.assertTrue(self.ctl.is_on(r))
            self.ctl.turn_off(r)
            self.assertFalse(self.ctl.is_on(r))

    def test_group_set_get(self):
        for r in range(self.ctl.get_count()):
            self.ctl.set_group(r, 123)
            self.assertEqual(self.ctl.get_group(r), 123)
            self.ctl.set_group(r, r)
            self.assertEqual(self.ctl.get_group(r), r)

    def test_status_toggle(self):
        on = self.ctl.is_on(0)
        self.ctl.toggle(0)
        self.assertTrue(not on)
        self.ctl.toggle(0)
        self.assertTrue(on == self.ctl.is_on(0))

    def test_group_status_set(self):
        on = self.ctl.is_on(0)
        for r in range(self.ctl.get_count()):
            self.ctl.set_group(r, 123)
        self.ctl.turn_on(0)
        for r in range(self.ctl.get_count()):
            self.assertTrue(self.ctl.is_on(r))
        self.ctl.turn_off(0)
        for r in range(self.ctl.get_count()):
            self.assertFalse(self.ctl.is_on(r))
        self.ctl.set_group(2, 0)
        self.ctl.turn_on(2)
        self.assertFalse(self.ctl.is_on(0))
        self.assertFalse(self.ctl.is_on(1))
        self.assertTrue(self.ctl.is_on(2))
        self.assertFalse(self.ctl.is_on(3))
        for r in range(self.ctl.get_count()):
            self.ctl.set_group(r, r)
            self.ctl.turn_off(r)


def main():
    import argparse

    parser = argparse.ArgumentParser(description="Manage outlets")
    parser.add_argument(
        '-v', '--verbose',
        action='count',
        default=0,
        help='Increase verbosity')
    g = parser.add_mutually_exclusive_group()
    g.add_argument(
        '-s', '--status',
        metavar='OUTLET',
        type=int,
        help='''
        Display the status of the specified outlet.
        When no option is specified, show the status of all outlets.
        ''')
    g.add_argument(
        '-c', '--count',
        action='store_true',
        help='Display the number of outlets.')
    g.add_argument(
        '-1', '--on',
        metavar='OUTLET',
        type=int,
        help='Turn the specified outlet ON.')
    g.add_argument(
        '-0', '--off',
        metavar='OUTLET',
        type=int,
        help='Turn the specified outlet OFF.')
    g.add_argument(
        '-t', '--toggle',
        metavar='OUTLET',
        type=int,
        help='Toggle the specified outlet.')
    g.add_argument(
        '-g', '--group',
        metavar=('OUTLET', 'GROUP'),
        type=int,
        nargs=2,
        help='Change the group of the specified outlet.')
    args = parser.parse_args()

    if args.verbose:
        level = {
            1: logging.INFO,
            2: logging.DEBUG,
        }.get(args.verbose, logging.WARNING)
        logging.basicConfig(level=level, format='%(levelname)s: %(message)s')

    try:
        ctl = OutletControl()
        if args.on is not None:
            ctl.turn_on(args.on)
        elif args.off is not None:
            ctl.turn_off(args.off)
        elif args.toggle is not None:
            ctl.toggle(args.toggle)
        elif args.group is not None:
            ctl.set_group(*args.group)
        elif args.count:
            print(ctl.get_count())
        else:
            if args.status is not None:
                print('on' if ctl.is_on(args.status) else 'off')
            else:
                for i in range(ctl.get_count()):
                    status = 'on' if ctl.is_on(i) else 'off'
                    group = ctl.get_group(i)
                    print(f'{i}: {status:<3} (group: {group})')
    except Exception as e:
        print('error: %s' % e, file=sys.stderr)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
