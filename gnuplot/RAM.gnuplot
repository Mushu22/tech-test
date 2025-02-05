set datafile separator ';'

set title "RAM"
set ylabel "RAM usage(kB)" # label for the Y axis
set xlabel "Time(s)" # label for the X axis

set terminal qt size 960, 720

#MemTotal 2
#MemFree 3
#MemAvailable 4
#Buffers 5
#Cached 6
#SwapCached 7
#Active 8
#Inactive 9
#SwapTotal 10
#SwapFree 11

plot \
  "RAM.csv" using 1:2 title 'Free' with filledcurves x1 fillcolor rgb "#DDDDDD", \
  "RAM.csv" using 1:($2-$3) title 'Cached' with filledcurves x1 fillcolor rgb "#00BB00", \
  "RAM.csv" using 1:($2-$3-$6) title 'Buffer' with filledcurves x1 fillcolor rgb "#0000BB", \
  "RAM.csv" using 1:($2-$3-$6-$5) title 'Used' with filledcurves x1 fillcolor rgb "#BB0000" , \
  "RAM.csv" using 1:($2-$4) title 'Unavailable' with lines lw 2 lc rgb "#000000"

while(1) {
    pause 10
    replot
}
