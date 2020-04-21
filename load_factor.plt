set term png enhanced giant size 1980,1080
set output "img/load_factor.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Varying Load Factor on Latency"


plot "load factor 50.dat" with lines, "load factor 75.dat" with lines, "load factor 95.dat" with lines
