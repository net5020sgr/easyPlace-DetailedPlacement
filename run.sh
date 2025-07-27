# ./cadc1019 <design_name> 
./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -targetDensity 0.6  -targetOverflow 0.1 -noQP -internalLegal 1 -internalDP 1 -TDP -bb -outputPath ./solution
rm sta_iter*
rm *.rpt
rm eplace_sta.tcl