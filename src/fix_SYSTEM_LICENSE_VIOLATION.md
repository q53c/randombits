## [BSOD 0x9A 1st param=3 SYSTEM_LICENSE_VIOLATION Windows 7](https://superuser.com/questions/1476444/bsod-0x9a-1st-param-3-system-license-violation-windows-7)

Asked 6 years, 5 months ago | Modified [6 years, 5 months ago](https://superuser.com/questions/1476444/bsod-0x9a-1st-param-3-system-license-violation-windows-7?lastactivity) | Viewed 272 times

> Solved the problem so this isn't a question but info for those who might care...
>
> A post elsewhere from a clearly informed person noted that this error does not mean the license is invalid, but specifically that windows could not read one or both registry keys at HKLM/SYSTEM/SETUP/SetupType and .../SystemSetupInProgress
>
> Booting from a PE CD, I could load the registry component from the system drive and clearly could read both values, and both were 0 as expected, so I spent some time following other suggestions.
>
> Ultimately I simply changed both values to 1 and saved the edits, then loaded the hive again and set them back to zero. Problem solved. Basking.
>
> Environment was an Intel-based RAID 10 system, fwiw.


works for [me](https://github.com/q53c), on a win11 23H2 box
