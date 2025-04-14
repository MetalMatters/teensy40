import serial
import time
import re
import math

SERIAL_PORT = 'COM6'  # Replace with your serial port
BAUD_RATE = 250000
DEFAULT_FEEDRATE = 6000  # Default feedrate in mm/min if not specified in G-code

def parse_gcode_line(line):
    """Parse a line of G-code and return a dictionary of coordinates and feedrate."""
    gcode = {}
    params = re.findall(r'([XYZF])([-+]?[0-9]*\.?[0-9]+)', line)
    for param in params:
        gcode[param[0]] = float(param[1])
    return gcode

# def interpolate_move(start, end, steps):
#     """Generate intermediate steps for a G1 move."""
#     interpolated_moves = []
#     for axis in 'XYZ':
#         if axis in end and axis not in start:
#             start[axis] = 0  # Assume 0 for missing start coordinates

#     for step in range(1, steps):
#         fraction = step / float(steps)
#         interpolated_move = {}
#         for axis in 'XYZ':
#             if axis in start and axis in end:
#                 interpolated_move[axis] = start[axis] + (end[axis] - start[axis]) * fraction
#         interpolated_move['F'] = end.get('F', DEFAULT_FEEDRATE)
#         interpolated_moves.append(interpolated_move)
#     return interpolated_moves

def calculate_step_delay(start, end, feedrate):
    """Calculate the delay between each interpolation step."""
    distance = math.sqrt(sum((end.get(axis, 0) - start.get(axis, 0)) ** 2 for axis in 'XY'))
    feedrate_mmnS = feedrate / (60.0*1E9)  # Convert feedrate to mm/ns from mm/min
    time_to_move = distance / feedrate_mmnS  # Time in nanoseconds
    return time_to_move

def send_gcode(serial_conn, command):
    """Send a single G-code command over the serial connection."""
    while serial_conn.out_waiting > 0:
        print("Idle")
    serial_conn.write((command + '\n').encode('utf-8'))
    #print(f'Sent: {command}')
    
def wait_for_ok(ser, timeout=10):
    """
    Wait for an 'ok' response from the serial device.

    :param ser: The serial connection
    :param timeout: Timeout in seconds (default is 10 seconds)
    :return: True if 'ok' received, False if timeout reached
    """
    start_time = time.time()
    response = ""

    while (time.time() - start_time) < timeout:
        # Read from serial port
        if ser.in_waiting > 0:
            response += ser.read(ser.in_waiting).decode('utf-8')
            
            # Check if 'ok' is in the response
            if "ok" in response:
                #print(" OK Recv\n")
                return True
            else:
                print(response)

        #time.sleep(0.001)  # Small delay to prevent overwhelming the CPU

    # Timeout reached
    return False

def execute_gcode_with_timing(serial_port, gcode_file_path):
    with open(gcode_file_path, 'r') as file:
        lines = file.readlines()

    prev_gcode = None

    # Open the serial port
    with serial.Serial(serial_port, BAUD_RATE, timeout=1) as ser:
        for line in lines:
            # Ignore empty lines and comments
            line = line.strip()
            if not line or line.startswith(';'):
                continue

            current_gcode = parse_gcode_line(line)
            feedrate = current_gcode.get('F', DEFAULT_FEEDRATE)
            # send_gcode(ser, line)
            # wank = time.time_ns()
            # while time.time_ns() < (wank+1200000):
            #     pass
 
            
            if prev_gcode:
                # Calculate the number of interpolation steps
                # steps = 10  # For example, break the movement into 10 smaller steps
                # interpolated_moves = interpolate_move(prev_gcode, current_gcode, steps)
                # if not (current_gcode.get('X') or current_gcode.get('Y')):
                #     continue
                #step_delay = calculate_step_delay(prev_gcode, current_gcode, feedrate) #/ steps
                send_gcode(ser, line)
                wait_for_ok(ser,10)
                # _now = time.time_ns()
                # while time.time_ns() < (_now+step_delay):
                #     pass
                # for interpolated_move in interpolated_moves:
                #     interpolated_command = gcode_dict_to_command(interpolated_move)
                #     send_gcode(ser, interpolated_command)
                #     time.sleep(step_delay)  # Wait for the appropriate time
                    #time.sleep(1/250000)

            prev_gcode = current_gcode

            # Optionally, send the final point of the current move
            # final_command = gcode_dict_to_command(current_gcode)
            # send_gcode(ser, final_command)
            # Do not sleep after the last point, as the next move will include its own delay

def gcode_dict_to_command(gcode):
    """Convert a dictionary of G-code parameters to a G-code command string."""
    command = 'G1'  # Assuming linear interpolation
    for key, value in gcode.items():
        command += f' {key}{value:.3f}'
    return command

# Usage
gcode_path = "C:\\Users\\Jeremy\\Downloads\\high_def_circle.gcode"
execute_gcode_with_timing(SERIAL_PORT, gcode_path)
