set term png enhanced giant size 1980,1080
set output "img/3d_graph.png"

set key noautotitle

set title "Client & Server Threads vs Throughput"

set xlabel "Client Threads"
set ylabel "Server Threads"
set zlabel "Throughput"

set dgrid3d 32,32
set xrange [1:8]
set yrange [1:8]

splot "3d_data.dat" u 1:2:3 with lines
