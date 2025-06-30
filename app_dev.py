import os
import subprocess

import pyinotify

# Get the directory where the script is located
script_dir = os.path.dirname(os.path.abspath(__file__))
storage_path = os.path.join(script_dir, 'storage')
program_path = os.path.join(script_dir, '.pio/build/linux/program')

wm = pyinotify.WatchManager()  # Watch Manager
mask = pyinotify.IN_CLOSE_WRITE | pyinotify.IN_CREATE  # Watched events

class EventHandler(pyinotify.ProcessEvent):
    def process_IN_CLOSE_WRITE(self, event):
        self.handle_event(event)

    def process_IN_CREATE(self, event):
        self.handle_event(event)

    def handle_event(self, event):
        if event.pathname.startswith(storage_path):
            try:
                subprocess.run(['pkill', '-f', 'program'], check=True)
            except subprocess.CalledProcessError as e:
                print(f"Error stopping program: {e}")
            try:
                subprocess.Popen([program_path])
            except Exception as e:
                print(f"Error starting program: {e}")

handler = EventHandler()
notifier = pyinotify.Notifier(wm, handler)
wdd = wm.add_watch(storage_path, mask, rec=True)  # Monitor "storage" recursively

notifier.loop()
