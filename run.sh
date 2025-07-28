# ./cadc1019 <design_name> 
./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./solution
# ./ePlace -aux ./after_buffered/aes_cipher_top_buf.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./solution

rm sta_iter*
rm *.rpt
rm eplace_sta.tcl