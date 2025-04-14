import re
import numpy as np

def parse_gcode_line(line):
    """Parse a line of G-code and extract X, Y coordinates."""
    x_match = re.search(r'X([-\d.]+)', line)
    y_match = re.search(r'Y([-\d.]+)', line)
    x = float(x_match.group(1)) if x_match else None
    y = float(y_match.group(1)) if y_match else None
    return x, y

def calculate_distance(coord1, coord2):
    """Calculate the Euclidean distance between two points."""
    if None in coord1 or None in coord2:
        return 0
    return np.sqrt((coord2[0] - coord1[0])**2 + (coord2[1] - coord1[1])**2)

def sum_path_length(gcode_file_path):
    """Calculate the total path length from a G-code file."""
    with open(gcode_file_path, 'r') as file:
        gcode_lines = file.readlines()

    last_position = (None, None)
    total_path_length = 0

    for line in gcode_lines:
        if line.startswith('G0') or line.startswith('G1'):
            x, y = parse_gcode_line(line)
            
            if x is not None or y is not None:
                current_position = (x, y)
                total_path_length += calculate_distance(last_position, current_position)
                last_position = current_position

    return total_path_length

# Example usage
file_path = 'C:\\Users\\Jeremy\\Downloads\\V0_3DBenchy_Stripped.gcode'
print(f'Total Path Length: {sum_path_length(file_path)}')
