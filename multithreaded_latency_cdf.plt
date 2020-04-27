set term png enhanced giant size 1980,1080
set output "4x4.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Latency CDF by # of Threads"

plot "throughput.dat" with lines
