Testaufbau:
	Testboard:
		NXHX4000-JTAG+ '7813.100.4 - 20131'
		NXHX-FTDI '7703.050.2 - 20113'

Testablauf:		
	-Testboard flashen mit:
		-'common/netx4000_startup_SPM.nxi'
		-'common/fdl_cifx4000.fdl'
		-'netx4000/HWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_V4.hwc'
		-'netx4000/MWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_V4.mwc'
		
	-Konsole mode verlassen
			
	-Test SW (elf file) erzeugen:
		-'cd ...\com.hilscher.hw.util-setup_dpm'
		-'python mbs/mbs'

	-Testboard mit Lauterbach verbinden
		-'netx4000/Lauterbach_netx4000_SPM_test.do' ausführen (das script darf nicht in einem anderen Verzeichnis liegen, weil es relative Pfade enthält)
		->Ausführen im Lauterbach: (File->Run Script: {Pfad zum Script auswählen})		
			
	
		


Testerwartung:

	Memory.Dump Fenster zeigt 'netx4000 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4000_SPM.PNG')			
					
			
Testergebnis:

	TEST OK:		Nachricht Fenster im Lauterbach zeigt "Found Message for netx4000 SPM-Setup! TEST OK"
	
	TEST NICHT OK: 	Nachricht Fenster im Lauterbach zeigt "TEST FAILED! no correct message found for netx4000 SPM-Setup"