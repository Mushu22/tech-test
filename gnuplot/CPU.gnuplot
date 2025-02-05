set datafile separator ';'

set title "CPU"
set xlabel "Time(s)"
set ylabel "CPU Load (Single Core %)"

set terminal qt size 960, 720

#5:idle  grey
#11:guest_nice
#10:guest
#9:steal
#8:softirq
#7:irq
#3:nice 
#6:iowait red
#4:system blue
#2:user green

plot \
  "CPU.csv" using 1:($2+$4+$6+$3+$7+$8+$9+$10+$11+$5) title 'idle' with filledcurves x1 fillcolor rgb "#DDDDDD", \
  "CPU.csv" using 1:($2+$4+$6+$3+$7+$8+$9+$10+$11) title 'guest_nide' with filledcurves x1 fillcolor rgb "#FF007F", \
  "CPU.csv" using 1:($2+$4+$6+$3+$7+$8+$9+$10) title 'guest' with filledcurves x1 fillcolor rgb "#FF00FF", \
  "CPU.csv" using 1:($2+$4+$6+$3+$7+$8+$9) title 'steal' with filledcurves x1 fillcolor rgb "#00FFFF", \
  "CPU.csv" using 1:($2+$4+$6+$3+$7+$8) title 'softirq' with filledcurves x1 fillcolor rgb "#FFFF00", \
  "CPU.csv" using 1:($2+$4+$6+$3+$7) title 'irq' with filledcurves x1 fillcolor rgb "#FF8000", \
  "CPU.csv" using 1:($2+$4+$6+$3) title 'nice' with filledcurves x1 fillcolor rgb "#7F00FF", \
  "CPU.csv" using 1:($2+$4+$6) title 'iowait' with filledcurves x1 fillcolor rgb "#FF0000", \
  "CPU.csv" using 1:($2+$4) title 'system' with filledcurves x1 fillcolor rgb "#0000FF", \
  "CPU.csv" using 1:($2) title 'user' with filledcurves x1 fillcolor rgb "#00FF00"

while(1) {
    pause 2
    replot
}
