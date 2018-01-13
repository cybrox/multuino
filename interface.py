# Multuino HTTP <-> Serial interface attempt
# Used Packages: pyserial
#
# Maintainer: Sven Gehring <cbrxde@gmail.com>
import sys
import glob
import serial
from serial.tools import list_ports
from http.server import BaseHTTPRequestHandler, HTTPServer


# Find all available serial ports
available_ports = list(list_ports.comports())
multuino_port = None

for port in available_ports:
  if port[1].startswith("Arduino Micro"):
    multuino_port = port


# Abort if no connected device was found
if multuino_port is None:
  print("Failed to find connected Multuino!")
  sys.exit(1)


# Define custom HTTP handler for simple HTTP API
class multuinoHttpServer_RequestHandler(BaseHTTPRequestHandler):
  def do_GET(self):
    path_parts = self.path.split("/")
    resource = None
    command = None

    if len(path_parts) >= 2:
      resource = path_parts[1]
    
    if len(path_parts) >= 3:
      command = path_parts[2]

    if resource == "trigger":
      self.send_command(command)

    self.send_response(200)
    self.send_header('Content-type','text/html')
    self.wfile.write(bytes("OK", "utf8"))
    return
  
  def send_command(self, command):
    numeric_command = int(command)
    connection = serial.Serial(multuino_port[0], 9600)
    connection.write(bytes([numeric_command]))
    connection.close()
    return


# Start HTTP server
print("Starting Multuino HTTP server on port 8081")
http_address = ('192.168.1.11', 8081)
http_daemon = HTTPServer(http_address, multuinoHttpServer_RequestHandler)
http_daemon.serve_forever()

sys.exit(0)
