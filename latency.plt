set term png
set output "img/local.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Local vs Networked Cache Latency"

set logscale x 10
plot "local_8k.dat" with lines, "networked_8k.dat" with lines
