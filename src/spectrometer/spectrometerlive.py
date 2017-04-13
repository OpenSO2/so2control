import matplotlib.pyplot as plt
import subprocess
import signal
import sys

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)
plt.show()

p = False

# intercept ctrl+c
def signal_handler(signal, frame):
	proc.kill()
	proc.wait()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

# plot indefinitely
while 1:
	proc = subprocess.Popen(["./spectrometer-cli", "250"], stdout=subprocess.PIPE)

	# check if subprocess has returned
	while proc.poll() is None:
		plt.pause(.01)

	# get and parse output from subprocess
	spectro = proc.communicate()[0]

	y = []
	x = []
	for line in spectro.split("\n"):
		vals = line.split(" ")
		if vals[0] and vals[1]:
			x.append( float(vals[0]) )
			y.append( float(vals[1]) )

	# init or update plot
	if not p:
		p = ax.plot(x, y, 'r-')[0]

	p.set_ydata(y)
	plt.draw()
	#~ fig.canvas.flush_events() # cargo-cult
