src: https://superuser.com/questions/1304215/mount-efi-partition-on-a-removable-media-in-windows-10

----

I found a way for it to work using `mountvol` by issuing `mountvol /?` first.

The output will have some hint at the end and in my case it was:

```
Possible values for VolumeName along with current mount points are:

\\?\Volume{38d5d60e-5760-43bc-85ac-25f3452b2e2f}\
    *** NO MOUNT POINTS ***

\\?\Volume{5f7cfdd0-dc43-0c6f-e24a-85ed1e43d782}\
    E:\

\\?\Volume{b08ddf00-9937-413c-82e9-c0e55525e46f}\
    *** NO MOUNT POINTS ***

\\?\Volume{07434f9f-e9d4-11e9-84c3-7085c2c582c4}\
    *** NO MOUNT POINTS ***

\\?\Volume{07434fa0-e9d4-11e9-84c3-7085c2c582c4}\
    D:\

\\?\Volume{45ce4d69-be26-4a9e-bc11-d850c69875ec}\
    C:\
```

After some tries (you can test one by one), the command that worked for me is:

```
mountvol h: \\?\Volume{07434f9f-e9d4-11e9-84c3-7085c2c582c4}\
```

Then find a way to access `H:`, which shows up: open Task Manager > File > Run New Task, and browse to it.

---

You can do it **easily** using **Windows PowerShell** without using any third-party apps.

Lets assume disk number is 4 and EFI partition number is 1.

1. Open **PowerShell** with **Admin privileges.**

2. Use the following syntax with a respectable drive letter and a partition number.

   Add-PartitionAccessPath -DiskNumber 4 -PartitionNumber 1 -AccessPath "Z:"

This will mount the EFI volume "**Z**". (**you can't access the volume using windows explorer without Admin privileges** , So use third-party apps like 7Zip File manager(run as Administrator) etc..)

---

