import requests
import tkinter as tk
from tkinter import StringVar
import time

# Replace with the ESP32's IP address
ESP32_IP = "http://192.168.103.198/data"

def fetch_data():
    try:
        response = requests.get(ESP32_IP, timeout=5)
        response.raise_for_status()
        data = response.json()
        
        latitude.set(f"Latitude: {data.get('latitude', 'N/A')}")
        longitude.set(f"Longitude: {data.get('longitude', 'N/A')}")
        percentage_filled.set(f"Bin Filled: {data.get('percentage_filled', 'N/A')}%")
        status.set(f"Status: {data.get('status', 'N/A')}")
    except requests.RequestException as e:
        latitude.set("Error fetching data!")
        longitude.set("")
        percentage_filled.set("")
        status.set("")
        print(f"Error: {e}")

    # Schedule the next update
    root.after(1000, fetch_data)

# Create the GUI
root = tk.Tk()
root.title("Real-Time Bin Monitor")

latitude = StringVar()
longitude = StringVar()
percentage_filled = StringVar()
status = StringVar()

tk.Label(root, textvariable=latitude, font=("Arial", 14)).pack(pady=5)
tk.Label(root, textvariable=longitude, font=("Arial", 14)).pack(pady=5)
tk.Label(root, textvariable=percentage_filled, font=("Arial", 14)).pack(pady=5)
tk.Label(root, textvariable=status, font=("Arial", 14)).pack(pady=5)

# Start fetching data
fetch_data()

# Run the GUI loop
root.mainloop()
