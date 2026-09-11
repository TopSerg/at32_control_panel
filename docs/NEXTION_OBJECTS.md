# Nextion page 0 object names

Firmware writes these text objects:

- tStatus
- tRpm
- tTorque
- tIa, tIb, tIc
- tIdc1, tIdc2
- tUa, tUb, tUc
- tUdc1, tUdc2
- tVin12
- tTbrd
- tText1, tText2
- tRawTrq

The supplied bench archive did not contain the HMI/TFT source, therefore these names are the firmware-side interface for the first version. If the real HMI has other names, change them in src/nextion.c.
