import smbus
import json
import time

i2c = smbus.SMBus(1)
addr = 0x10

step = 12

i2c.write_byte_data(addr, 1, 1)

time.sleep(.1)

i2c.write_byte_data(addr, 1, step)  # ccw

time.sleep(2)

i2c.write_byte_data(addr, 0, step)  # cw

time.sleep(.1)

i2c.write_byte_data(addr, 2, 0) # disable stepper

with open("status.json", "w") as f:
    f.write(json.dumps({"status": 1}))
