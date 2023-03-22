setenv ipaddr 192.168.1.7; setenv serverip 192.168.1.6; tftp $loadaddr capdl-loader-image-arm-bcm2711; go $loadaddr
