import smbus
import json
import time

i2c = smbus.SMBus(1)
addr = 0x10

i2c.write_byte_data(addr, 0, 12)  # ccw, 12 steps

time.sleep(4)

i2c.write_byte_data(addr, 1, 12)  # cw, 12 steps

i2c.write_byte_data(addr, 2, 0) # disable stepper

with open("status.json", "w") as f:
    f.write(json.dumps({"status": 1}))
