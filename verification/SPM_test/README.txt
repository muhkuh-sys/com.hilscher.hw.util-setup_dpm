Testablauf:		
	-Testboard flashen mit:
			-'common/netx4000_startup_SPM.nxi'
			-'common/fdl_cifx4000.fdl'
		netx4000:
			-'netx4000/HWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.hwc'
			-'netx4000/MWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.mwc'
		netx4100:
			-'netx4100/HWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.hwc'
			-'netx4100/MWC_NXHX4000-JTAG-r4_1GByteDDR3_600MHz_SPM_0.0.2.0.mwc'
		
		
	-Testboard mit Lauterbach verbinden
	-'common/Lauterbach_SPM_test.do' ausführen (funktioniert nur wenn es sich noch im aktuellen Verzeichnis befindet)
		->Ausführen im Lauterbach: (File->Run Script: {Pfad zum Script auswählen})

Testerwartung:
	netx4000:
		TEST OK:	Wenn Memory.Dump Fenster zeigt 'netx4000 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4000_SPM.PNG')
					ODER
					Nachricht Fenster im Lauterbach zeigt "Found Message for netx4000 SPM-Setup! TEST OK"
		
	netx4100:
		TEST OK:	Wenn Memory.Dump Fenster zeigt 'netx4000 serial DPM' als ASCII Wert (siehe 'Sollergebnis_netx4100_SPM.PNG')
					ODER
					Nachricht Fenster im Lauterbach zeigt "Found Message for netx4100 SPM-Setup! TEST OK"
					
					
					
Testergebnis:

	netx4000: TEST OK
	
	netx4100: TEST OK