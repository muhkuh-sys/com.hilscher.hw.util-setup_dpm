The PCIe DPM is not visible by default. 2 things prevent access to the DPM contents.

1) Firewall
From the PC view the complete DPM is filled with 0xdeafdeaf . The firewalls are still closed at this point to prevent an access before the firmware is running.
Open the firewall and the value changes from 0xdeafdeaf to 0x0bad0bad.

2) DPM locked
If the DPM is not configured yet, the memory shows 0x0bad0bad. Setting the "enable" bit in the idpm0_idpm_cfg0x0 register to 1 shows the DPM register in the first 256 bytes.
The rest of the 64K can be configured with the "idpm0_idpm_win*_end" and "idpm0_idpm_win*_map" registers.
