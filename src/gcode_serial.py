# import serial
# import time
# import re

# SERIAL_PORT = 'COM3'  # Replace with your serial port
# BAUD_RATE = 250000
# DEFAULT_FEEDRATE = 6000  # Default feedrate in mm/min if not specified in G-code

# def parse_gcode_line(line):
#     """Parse a line of G-code and return a dictionary of coordinates and feedrate."""
#     gcode = {}
#     params = re.findall(r'([XYZFP])([-+]?[0-9]*\.?[0-9]+)', line)
#     for param in params:
#         gcode[param[0]] = float(param[1])
#     return gcode

# def send_gcode(serial_conn, command):
#     """Send a single G-code command over the serial connection."""
#     while serial_conn.out_waiting > 0:
#         print("Idle")
#     serial_conn.write((command + '\n').encode('utf-8'))

# def wait_for_ok(ser, timeout=10):
#     """
#     Wait for an 'ok' response from the serial device.

#     :param ser: The serial connection
#     :param timeout: Timeout in seconds (default is 10 seconds)
#     :return: True if 'ok' received, False if timeout reached
#     """
#     start_time = time.time()
#     response = ""

#     while (time.time() - start_time) < timeout:
#         # Read from serial port
#         if ser.in_waiting > 0:
#             response += ser.read(ser.in_waiting).decode('utf-8')
            
#             # Check if 'ok' is in the response
#             if "ok" in response:
#                 return True
#             else:
#                 print(response)

#     # Timeout reached
#     return False

# def execute_gcode_with_resume(serial_port, gcode_file_path, start_index=0):
#     with open(gcode_file_path, 'r') as file:
#         lines = file.readlines()


#     # Skip to the starting index
#     lines_to_process = lines[start_index:]

#     # Open the serial port
#     with serial.Serial(serial_port, BAUD_RATE, timeout=1) as ser:
#         for index, line in enumerate(lines_to_process, start=start_index):
#             # Ignore empty lines and comments
#             line = line.strip()
#             if not line or line.startswith(';'):
#                 continue

#             #Only reference layer changes
#             if "G97" in line:
#                 print(index)

#             send_gcode(ser, line)
#             if not wait_for_ok(ser, 60):
#                 print(f"Timeout or error occurred at line {index}.")
#                 break  # Stop execution if there's a timeout or error

# # Usage
# gcode_path = "C:\\Users\\User\\Documents\\OPAL-master\\gcode\\Circle_39.gcode"
# start_index = 0  # Change this to the desired starting index in case of resuming
# execute_gcode_with_resume(SERIAL_PORT, gcode_path, start_index)
import serial
import time
import re
from pywinauto import Application

SERIAL_PORT = 'COM7'  # Replace with your serial port
BAUD_RATE = 250000
DEFAULT_FEEDRATE = 6000  # Default feedrate in mm/min if not specified in G-code
LOG_FILE_PATH = "gcode_index_log.txt"  # Path to the log file
LASER_STATE = False


def parse_gcode_line(line):
    """Parse a line of G-code and return a dictionary of coordinates and feedrate."""
    gcode = {}
    params = re.findall(r'([XYZFP])([-+]?[0-9]*\.?[0-9]+)', line)
    for param in params:
        gcode[param[0]] = float(param[1])
    return gcode

def send_gcode(serial_conn, command):
    """Send a single G-code command over the serial connection."""
    while serial_conn.out_waiting > 0:
        print("Idle")
    serial_conn.write((command + '\n').encode('utf-8'))

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
                return True
            else:
                print(response)

    # Timeout reached
    return False

def execute_gcode_with_resume(serial_port, gcode_file_path, start_index, laser_gui, power_button):

    global LASER_STATE

    with open(gcode_file_path, 'r') as file:
        lines = file.readlines()

    # Skip to the starting index
    lines_to_process = lines[start_index:]

    # Open the serial port and log file
    with serial.Serial(serial_port, BAUD_RATE, timeout=1) as ser, open(LOG_FILE_PATH, 'a') as log_file:
        for index, line in enumerate(lines_to_process, start=start_index):
            # Ignore empty lines and comments
            line = line.strip()
            if not line or line.startswith(';'):
                continue

            if "G99" in line:
                print(index)
                if not LASER_STATE:
                    power_button.click_input()
                    LASER_STATE = True
                continue

            if "G98" in line:
                if LASER_STATE:
                    power_button.click_input()
                    LASER_STATE = False
                continue
                #log_file.write(f"{index}\n")

            send_gcode(ser, line)
            if not wait_for_ok(ser, 60):
                print(f"Timeout or error occurred at line {index}.")
                break  # Stop execution if there's a timeout or error
            
    #Ensure the laser is inactive upon file completion
    if LASER_STATE:
        power_button.click_input()


# Connect to the application by its window title
JPT_GUI = Application(backend="uia").connect(title="TypeE_20220104")

# Access the main window
laser_gui = JPT_GUI.window(title="TypeE_20220104")

power_button = laser_gui.child_window(title="Laser", auto_id="PA", control_type="Button")

# Usage
gcode_path = "C:\\Users\\User\\Documents\\OPAL-master\\gcode\\uniformScanCube.gcode"
start_index = 0  # Change this to the desired starting index in case of resuming
execute_gcode_with_resume(SERIAL_PORT, gcode_path, start_index, laser_gui, power_button)
