set term png enhanced giant size 1980,1080
set output "img/single_thread_server.png"

set xlabel "Number of Client Threads"
set ylabel "Throughput (req/s)"
set title "Single Threaded Server Performance"

set boxwidth 0.9 relative
set style data histograms
set style fill solid 1.0 border -1

plot "throughput.dat" using 2:xticlabels(1)
