The PCIe DPM is not visible by default. 2 things prevent access to the DPM contents.

1) Firewall
From the PC view the complete DPM is filled with 0xdeafdeaf . The firewalls are still closed at this point to prevent an access before the firmware is running.
Open the firewall and the value changes from 0xdeafdeaf to 0x0bad0bad.

2) DPM locked
If the DPM is not configured yet, the memory shows 0x0bad0bad. Setting the "enable" bit in the idpm0_idpm_cfg0x0 register to 1 shows the DPM register in the first 256 bytes.
The rest of the 64K can be configured with the "idpm0_idpm_win*_end" and "idpm0_idpm_win*_map" registers.



The parallel and serial DPM ("external" DPM) is also not visible by default. Here the comlete area is 0x0bad0bad. The external DPM does not have a firewall or an enable bit.
The 0x0bad0bad is produced by a complete unconfigured address range. Enable the DPM by writing a 0x00010000 to the dpm_win1_end register. Optionally enable the configuration area.
