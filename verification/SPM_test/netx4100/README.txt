Testaufbau:
	Testboard:
		NXHX4000-JTAG+ '7813.100.4 - 20131'
		NXHX-FTDI '7703.050.2 - 20113'

Testablauf:		
	-Testboard flashen mit:
		-'common/netx4000_startup_SPM.nxi'
		-'common/fdl_cifx4000.fdl'
		-'../../../targets/verify/SPM_test/netx4100/HWC_NXHX4100-JTAG-r4_1GByteDDR3_600MHz_SPM_V4.hwc'
		
	-Konsole mode verlassen
			
	-Test SW (elf file) erzeugen:
		-'cd ...\com.hilscher.hw.util-setup_dpm'
		-'python mbs/mbs'

	-Testboard mit Lauterbach verbinden
		-'netx4100/Lauterbach_netx4100_SPM_test.do' ausführen (das script darf nicht in einem anderen Verzeichnis liegen, weil es relative Pfade enthält)
			->Ausführen im Lauterbach: (File->Run Script: {Pfad zum Script auswählen})		
			
	
		


Testerwartung:

	Memory.Dump Fenster zeigt 'netx4100 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4100_SPM.PNG')			
					
			
Testergebnis:

	TEST OK:		Nachricht Fenster im Lauterbach zeigt "TEST OK"
	
	TEST NICHT OK: 	Nachricht Fenster im Lauterbach zeigt eine der folgenden Texte:
					-"Failed to stop at first BP"			
					-"Failed to stop at second BP"
					-"Failed to stop at third BP"
					-"Failed to stop at end of HWC"
					 (Der test konnte nicht an einen der gesetzten Breakpoints anhalten)
					
					-"Error expected setup_dpm code NOT found"
					 (Der code im flash stimmt nicht mit dem .elf file überein)
					
					-"MESSAGE-TEST 1 FAILED! no correct message found for netx4000 SPM-Setup"
					 (Das DPM wurde nicht korrekt aufgesetzt)
					 
					-"MESSAGE-TEST 2 FAILED! no correct message found for netx4000 SPM-Setup"
					 (Das DPM wurde bereits aufgesetzt, ist jedoch danach überschrieben worden)