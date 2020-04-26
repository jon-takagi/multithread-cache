set term png enhanced giant size 1980,1080
set output "img/multi.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Latency CDF by # of Threads"

plot "1_threads.dat" with lines, "2_threads.dat" with lines, "3_threads.dat" with lines, "4_threads.dat" with lines
