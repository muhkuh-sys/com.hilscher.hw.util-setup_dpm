Testaufbau:
	Testboard:
		NXHX4000-JTAG+ '7813.100.3 - 20086' (OTP fuse set to netx4100) 

Testablauf:		
	-Testboard flashen mit:
		-'common/netx4100_startup_SPM.nxi'
		-'common/fdl_cifx4000.fdl'
		-'netx4100/HWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.hwc'
		-'netx4100/MWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.mwc'
			
	-Testboard mit Lauterbach verbinden
		-'netx4100/Lauterbach_netx4100_SPM_test.do' ausführen (funktioniert nur wenn es sich noch im aktuellen Verzeichnis befindet)
		->Ausführen im Lauterbach: (File->Run Script: {Pfad zum Script auswählen})		
			
	
		


Testerwartung:

	Memory.Dump Fenster zeigt 'netx4100 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4100_SPM.PNG')			
					
			
Testergebnis:

	TEST OK:		Nachricht Fenster im Lauterbach zeigt "Found Message for netx4100 SPM-Setup! TEST OK"
	
	TEST NICHT OK: 	Nachricht Fenster im Lauterbach zeigt "TEST FAILED! no correct message found for netx4100 SPM-Setup"