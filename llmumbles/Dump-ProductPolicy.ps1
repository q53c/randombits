# 读取 ProductPolicy 数据
$regPath = "HKLM:\SYSTEM\ControlSet001\Control\ProductOptions"
$propName = "ProductPolicy"

# 确保键存在
if (-not (Test-Path $regPath)) {
    Write-Error "Registry path not found: $regPath"
    return
}

$regItem = Get-ItemProperty -Path $regPath -Name $propName -ErrorAction Stop
$policy  = $regItem.$propName

if (-not $policy -or $policy.Length -eq 0) {
    Write-Error "ProductPolicy value is empty or not found."
    return
}

# 建立内存流 & BinaryReader
$ms = New-Object System.IO.MemoryStream(,$policy)
$br = New-Object System.IO.BinaryReader($ms)

try {
    # 头部 5 个 UInt32
    $totalSize      = $br.ReadUInt32()
    $entriesSize    = $br.ReadUInt32()
    $endingSize     = $br.ReadUInt32()
    $unknownHeader  = $br.ReadUInt32()
    $version        = $br.ReadUInt32()

    Write-Host ("total license data size (including header): {0}" -f $totalSize)
    Write-Host ("size of all entries: {0}" -f $entriesSize)
    Write-Host ("size of of ending marker: {0}" -f $endingSize)
    Write-Host ("unknown: {0}" -f $unknownHeader)
    Write-Host ("license data version: {0}" -f $version)
    Write-Host ""

    while ($ms.Position -lt $ms.Length) {
        $entryPos  = $ms.Position
        $entrySize = $br.ReadUInt16()

        Write-Host ("entry offset: 0x{0:X8}" -f $ms.Position)
        Write-Host ("total size of entry: {0}" -f $entrySize)

        $sizeOfName = $br.ReadUInt16()

        # 结束标记（与原 C# 一致：entrySize == 69 且 name size == 0）
        if ($entrySize -eq 69 -and $sizeOfName -eq 0) {
            Write-Host ""
            Write-Host "No more entries."
            break
        }

        Write-Host ("size of name: {0}" -f $sizeOfName)

        $dataType   = $br.ReadUInt16()
        Write-Host ("type of data: {0}" -f $dataType)

        $sizeOfData = $br.ReadUInt16()
        Write-Host ("size of data: {0}" -f $sizeOfData)

        $flags      = $br.ReadUInt32()
        Write-Host ("flags: {0}" -f $flags)

        $unknown    = $br.ReadUInt32()
        Write-Host ("unknown: {0}" -f $unknown)

        Write-Host ("name offset: 0x{0:X8}" -f $ms.Position)

        # 读取 name（Unicode）
        $nameBytes = $br.ReadBytes($sizeOfName)
        $name      = [System.Text.Encoding]::Unicode.GetString($nameBytes, 0, $nameBytes.Length)
        Write-Host ("name: {0}" -f $name)

        Write-Host ("data offset: 0x{0:X8}" -f $ms.Position)

        # 读取 data
        $dataBytes = $br.ReadBytes($sizeOfData)

        if ($dataType -eq 4) {
            $value = [System.BitConverter]::ToUInt32($dataBytes, 0)
            Write-Host ("value: {0}" -f $value)
        }

        # 跳到下一条 entry 开头（按 entrySize 对齐）
        $ms.Seek($entryPos + $entrySize, [System.IO.SeekOrigin]::Begin) | Out-Null

        Write-Host ""
    }
}
finally {
    $br.Close()
    $ms.Close()
}