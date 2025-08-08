puts "reading lib.."
foreach libFile [glob "./ASAP7/LIB/*nldm*.lib"] {
    puts "lib: $libFile"
    read_liberty $libFile
}
puts "reading lef.."
read_lef ./ASAP7/techlef/asap7_tech_1x_201209.lef
foreach lef [glob "./ASAP7/LEF/*.lef"] {
    read_lef $lef
}
puts "reading def.."
#read_def ./solution/aes_cipher_top/aes_cipher_top-eDP.def

#read_def  ./after_buffered/aes_cipher_top_buf.def
#read_def ./eplace_solution/aes_cipher_top_buf/aes_cipher_top_buf-eDP.def



read_def ./eplace_solution/aes_cipher_top_buf/aes_cipher_top_buf-eLP.def
read_sdc ./after_buffered/aes_cipher_top/aes_cipher_top_buf.sdc

#read_def ./eplace_solution/aes_buf/aes_buf-eDP.def
#read_sdc ./after_buffered/aes/aes_buf.sdc

#read_def ./eplace_solution/ac97_top_buf/ac97_top_buf-eDP.def
#read_sdc ./after_buffered/ac97_top/ac97_top_buf.sdc

#read_def ./eplace_solution/pci_bridge32_buf/pci_bridge32_buf-eDP.def
#read_sdc ./after_buffered/pci_bridge32/pci_bridge32_buf.sdc

#read_def ./eplace_solution/des_buf/des_buf-eDP.def
#read_sdc ./after_buffered/des/des_buf.sdc

#read_def ./eplace_solution/des_buf/des_buf-eDP.def
#read_sdc ./after_buffered/des/des_buf.sdc

source ./ASAP7/setRC.tcl
estimate_parasitics -placement

# 5. 詳細配置
puts "Running detail placement..."

report_tns
report_wns
report_power
