EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Sensor_Motion:LSM6DS3 U12
U 1 1 5EB1E92E
P 5275 2325
F 0 "U12" H 5919 2371 50  0000 L CNN
F 1 "LSM6DSO" H 5919 2280 50  0000 L CNN
F 2 "Package_LGA:LGA-14_3x2.5mm_P0.5mm_LayoutBorder3x4y" H 4875 1625 50  0001 L CNN
F 3 "www.st.com/resource/en/datasheet/lsm6ds3.pdf" H 5375 1675 50  0001 C CNN
	1    5275 2325
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Humidity:Si7020-A20 U11
U 1 1 5EB1FA67
P 3325 2375
F 0 "U11" H 3769 2421 50  0000 L CNN
F 1 "SHTC3" H 3769 2330 50  0000 L CNN
F 2 "Package_DFN_QFN:DFN-6-1EP_3x3mm_P1mm_EP1.5x2.4mm" H 3325 1975 50  0001 C CNN
F 3 "" H 3125 2675 50  0001 C CNN
F 4 "SHTC3" H 3325 2375 50  0001 C CNN "MPN"
	1    3325 2375
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Motion:LIS2DE12 U10
U 1 1 5EB2022A
P 1650 2275
F 0 "U10" H 1700 2956 50  0000 C CNN
F 1 "LIS2DW12" H 1700 2865 50  0000 C CNN
F 2 "Package_LGA:LGA-12_2x2mm_P0.5mm" H 1800 2825 50  0001 L CNN
F 3 "" H 1300 2275 50  0001 C CNN
F 4 "LIS2DW12" H 1650 2275 50  0001 C CNN "MPN"
	1    1650 2275
	1    0    0    -1  
$EndComp
Text Notes 1200 3450 0    79   ~ 16
Accel + Gyro \nISM330DHCXTR w/ built-in ML engine
$Comp
L Memory_Flash:AT45DB161B-TC-2.5 U?
U 1 1 5EA94115
P 8700 2325
F 0 "U?" H 8700 2906 50  0000 C CNN
F 1 "AT45DB161B-TC-2.5" H 8700 2815 50  0000 C CNN
F 2 "Package_SO:TSOP-28_11.8x8mm_P0.55mm" H 8700 1775 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/devicedoc/doc2224.pdf" H 8700 2325 50  0001 C CNN
	1    8700 2325
	1    0    0    -1  
$EndComp
$EndSCHEMATC
