# ./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
#
# ./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bktrk -outputPath ./eplace_solution
rm ePlace
ln ./build/main/ePlace ePlace
# ./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./testcase/hidden5/hidden5.aux  -fullPlot -targetDensity 0.2  -targetOverflow 0.1  -noQP -internalLegal  -internalDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./testcase/aes_cipher_top/aes_cipher_top.aux  -fullPlot -targetDensity 0.6  -targetOverflow 0 .1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/aes_cipher_top/aes_cipher_top_buf.aux  -fullPlot -targetOverflow 0.1 -noQP -internalLegal  -TDP -bktrk -outputPath ./eplace_solution -TDP
./ePlace -aux ./after_buffered/ac97_top/ac97_top_buf.aux  -fullPlot -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bktrk -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/ac97_top/ac97_top_buf.aux  -fullPlot -targetOverflow 0.05 -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/ariane/ariane_buf.aux  -fullPlot  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/des/des_buf.aux  -fullPlot  -targetOverflow 0.1  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution -TDP
# ./ePlace -aux ./after_buffered/ac97_top/ac97_top_buf.aux  -fullPlot -targetDensity 0.9  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/pci_bridge32/pci_bridge32_buf.aux  -fullPlot -targetOverflow 0.1  -noQP -internalLegal  -TDP -bktrk -outputPath ./eplace_solution
# ./ePlace -aux ./ariane/ariane.aux  -fullPlot  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# ./ePlace -aux ./after_buffered/test_aes/aes/aes_buf.aux -fullPlot -targetDensity 0.9  -targetOverflow 0.2  -noQP -internalLegal  -internalDP  -TDP -bb -outputPath ./eplace_solution
# /home/jerry/easyPlace/after_buffered/test_aes/aes/aes_buf.aux
# gdb -q -ex 'set pagination off' -ex 'run' -ex 'bt' -ex 'quit' --args ./ePlace -def testcase/aes_cipher_top/aes_cipher_top.def -fullPlot -targetOverflow 0.1 -noQP -internalLegal  -TDP -bktrk -outputPath ./eplace_solution
# ./ePlace -def ./after_buffered/aes_cipher_top/aes_cipher_top_buf.def  -fullPlot -targetOverflow 0.1 -noQP -internalLegal  -TDP -bktrk -outputPath ./eplace_solution
# rm sta_iter*
# rm *.rpt
# rm eplace_sta.tcl




