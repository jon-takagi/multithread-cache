set term png enhanced giant size 1980,1080
set output "img/native.png"

set xlabel "Latency (ms)"
set ylabel "Percentage of latency at least x"
set title "Native Linux vs VM"


plot "jon_networked_8k.dat" with lines, "eli_networked_8k.dat" with lines
