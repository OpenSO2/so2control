import matplotlib.pyplot as plt
import subprocess
import signal
import sys

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)
plt.show()

first = True

def run():
	return subprocess.Popen(["./spectrometer-cli", "250"], stdout=subprocess.PIPE)

proc = run()

def signal_handler(signal, frame):
	proc.kill()
	proc.wait()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

while 1:
	retcode = proc.poll()
	if retcode is None:
		plt.pause(.01)
		continue

	spectro = proc.communicate()[0]
	y = []
	x = []
	for line in spectro.split("\n"):
		vals = line.split(" ")
		if vals[0] and vals[1]:
			x.append( float(vals[0]) )
			y.append( float(vals[1]) )

	if first:
		first = False
		p = ax.plot(x, y, 'r-')[0]

	p.set_ydata(y)
	plt.draw()
	fig.canvas.flush_events()

	proc.wait()

	proc = run()
