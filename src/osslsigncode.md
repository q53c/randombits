## osslsigncode sign driver

1. 把 `pfx` 拆成 `cert` 和 `key`：
   1. `openssl pkcs12 -in HT.pfx -nokeys -out HT.crt -legacy`
   2. `openssl pkcs12 -in HT.pfx -nocerts -nodes -out HT.key -legacy`
2. 打开 `HT.crt`, 把其中 subject 与 ac.subject 一样的证书删掉
3. `osslsigncode sign -spc HT.crt -key HT.key -ac mscv.cer -nolegacy -h sha2 -verbose -in ${in_sys} -out ${out_sys}`



---

来源：

https://github.com/mtrojnar/osslsigncode/issues/86#issuecomment-1651876095



