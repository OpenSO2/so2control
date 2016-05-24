#set terminal pngcairo
#set out "burolampe.png"

set arrow from 253.65,0 to 253.65,90000 nohead
set arrow from 296.73,0 to 296.73,90000 nohead
set arrow from 313.2,0 to 313.2,90000 nohead
set arrow from 334.1,0 to 334.1,90000 nohead

# set xrange [280:440]
# set yrange [0:90]
set xlabel "nm"
# set key off

# ilinie
set arrow from 365.16,0 to 365.16,90000 nohead

# hlinie
set arrow from 404.66,0 to 404.66,90000 nohead

set arrow from 407.78,0 to 407.78,90000 nohead
set arrow from 435.83,0 to 435.83,90000 nohead

plot "measurement.dat" u 1:($2/3) w l, "measurement-uncorrected.dat" u 1:($2/3) w l, "dark-current.dat" u 1:($2/60) w l, "electronic-offset.dat" u 1:2 w l

pause 10
reread
