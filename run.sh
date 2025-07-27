# ./cadc1019 <design_name> 
./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -targetDensity 0.6  -targetOverflow 0.1 -noQP -internalLegal  -internalDP  -TDP -bktrk -outputPath ./solution
rm sta_iter*
rm *.rpt
rm eplace_sta.tcl