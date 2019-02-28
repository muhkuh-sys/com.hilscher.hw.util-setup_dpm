Testaufbau:
	Testboard:
		NXHX4000-JTAG+ '7813.100.4 - 20131'

Testablauf:		
	-Testboard flashen mit:
		-'common/netx4000_startup_SPM.nxi'
		-'common/fdl_cifx4000.fdl'
		-'netx4000/HWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.hwc'
		-'netx4000/MWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.mwc'
			
	-Testboard mit Lauterbach verbinden
		-'netx4000/Lauterbach_netx4000_SPM_test.do' ausführen (funktioniert nur wenn es sich noch im aktuellen Verzeichnis befindet)
		->Ausführen im Lauterbach: (File->Run Script: {Pfad zum Script auswählen})		
			
	
		


Testerwartung:

	Memory.Dump Fenster zeigt 'netx4000 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4000_SPM.PNG')			
					
			
Testergebnis:

	TEST OK:		Nachricht Fenster im Lauterbach zeigt "Found Message for netx4000 SPM-Setup! TEST OK"
	
	TEST NICHT OK: 	Nachricht Fenster im Lauterbach zeigt "TEST FAILED! no correct message found for netx4000 SPM-Setup"