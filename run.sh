# ./cadc1019 <design_name> 
# ./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./solution
#
./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bktrk -outputPath ./solution
# ./ePlace -aux ./after_buffered/aes_cipher_top_buf.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./solution

rm sta_iter*
rm *.rpt
rm eplace_sta.tcl



run.sh:
# -> testcase/<design_name>/...
./cadc1019 aes_cipher_top #only do buffering , NO legaize, DP, SA
./eplace_run.sh
(optional) openroad repair_timing.tcl #output design_name.def
(optional) ./make_changelist.sh 