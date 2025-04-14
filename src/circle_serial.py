import serial
import math
import time

# Serial port setup
serial_port = 'COM6'  # Change this to your serial port
baud_rate = 250000
ser = serial.Serial(serial_port, baud_rate, timeout=1)

# Circle parameters
radius = 50  # Radius of the circle
center_x, center_y = 50, 50 
num_points = 100  # Number of points on the circle
initial_feedrate = 6000  # Initial feedrate in mm/min
feedrate_increment = 100  # Amount to increment feedrate each loop

# Function to generate circle points
def generate_circle_points(cx, cy, r, num_points):
    points = []
    for i in range(num_points):
        angle = 2 * math.pi * i / num_points
        x = cx + r * math.cos(angle)
        y = cy + r * math.sin(angle)
        points.append((x, y))
    return points

# Function to send G-code command
def send_gcode(command):
    ser.write(f"{command}\n".encode())
    delay = 1.0/(((current_feedrate/60)/(2*radius*math.pi))*num_points)
    time.sleep(delay)  # Adjust based on your machine's capabilities
#diameter*PI/(current_feedrate/60)
# Continuous operation loop
current_feedrate = initial_feedrate
while True:
    # Generate circle points
    circle_points = generate_circle_points(center_x, center_y, radius, num_points)
    # Start G-code transmission
    #send_gcode("G21")  # Set units to millimeters
    #send_gcode("G90")  # Absolute positioning
    send_gcode(f"G1 F{current_feedrate}")  # Set feedrate

    # Send G-code commands for each point
    for (x, y) in circle_points:
        gcode_cmd = f"G1 X{x:.2f} Y{y:.2f} F{current_feedrate}"
        send_gcode(gcode_cmd)
        #print(gcode_cmd)
    # Update feedrate for next loop
    current_feedrate += 1000

# Close the serial connection
# Note: This line will never be reached in the continuous loop
# You'll need to implement a condition to break the loop and close the serial port.
# ser.close()
