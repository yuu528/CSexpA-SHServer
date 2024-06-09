#!/bin/python3

import smbus
import json

i2c = smbus.SMBus(1)
addr = 0x10

i2c.write_byte(addr, 0, 5)  # ccw, 12 steps

while (i2c.read_byte(addr, 0) == 1):
    pass

i2c.write_byte(addr, 1, 6)  # cw, 12 steps

with open("status.json", "w") as f:
    f.write(json.dumps({"status": 1})
