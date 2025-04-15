import numpy as np

# Data provided
target_speeds = np.array([1500, 1350, 1200, 1050, 900, 750, 600, 450])
actual_speeds = np.array([1040, 982, 916, 835, 744, 643, 546, 432])

# Fit a 3rd order polynomial to the data
poly_coefficients = np.polyfit(target_speeds, actual_speeds, 3)

# Polynomial function
poly_fit = np.poly1d(poly_coefficients)

# Calculate target speeds for the given actual speeds
desired_actual_speeds = np.array([400, 500, 600, 700, 800])
target_speeds_needed = np.polyval(np.poly1d(np.polyfit(actual_speeds, target_speeds, 3)), desired_actual_speeds)

# Print results
print("Polynomial Coefficients:", poly_coefficients)
print("Target Speeds for Desired Actual Speeds:", target_speeds_needed)
