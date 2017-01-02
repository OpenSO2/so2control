import matplotlib.pyplot as plt
import subprocess

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)
plt.show()

first = True
while 1:
	spectro = subprocess.check_output(["./spectrometer-cli", "250"])
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
