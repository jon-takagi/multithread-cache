set term png enhanced giant size 1980,1080
set output "img/maxmem.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Varying Maximum Memory on Latency"


plot "maxmem_4k.dat" with lines, "maxmem_8k.dat" with lines, "maxmem_16k.dat" with lines
