set term pngcairo
set out 'runs.png'

set logscale x
set xtics 2,2,64
set xrange [1:128]
set yrange [0:300]
set xlabel "ilość rdzeni"
set ylabel "czas wykonania [s]"
plot 'res_64.data'  u 1:3 title '640MB', \
     'res_128.data' u 1:3 title '1280MB', \
     'res_256.data' u 1:3 title '2560MB', \
     'res_512.data' u 1:3 title '5120MB'

set out 'rate.png'
set yrange [0:5]
set ylabel "prędkość przetwarzania [GB/min]"
plot 'res_64.data'  u 1:(640*60/1000/$3) title '640MB', \
     'res_128.data' u 1:(1280*60/1000/$3) title '1280MB', \
     'res_256.data' u 1:(2560*60/1000/$3) title '2560MB', \
     'res_512.data' u 1:(5120*60/1000/$3) title '5120MB'

set out 'speedup.png'
set yrange [0:3]
set ylabel "przyspieszenie"
plot 'res_64.data'  u 1:(12.82/$3) title '640MB', \
     'res_128.data' u 1:(35.80/$3) title '1280MB', \
     'res_256.data' u 1:(102.44/$3) title '2560MB', \
     'res_512.data' u 1:(234.08/$3) title '5120MB'
