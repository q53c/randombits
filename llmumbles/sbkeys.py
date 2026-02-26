#!/usr/bin/env python3
"""
sbkeys.py - 查看当前系统安全启动(Secure Boot)的 PK, KEK, db, dbx 证书信息

功能：
  1. 检查 Secure Boot 是否启用
  2. 通过 PowerShell 导出 PK/KEK/db/dbx 的 EFI 签名列表二进制数据
  3. 解析 EFI Signature List 格式，提取 X.509 证书和 SHA-256 哈希
  4. 展示证书的 Subject、Issuer、序列号、有效期、SHA256 指纹等详细信息

依赖：
  - Windows 系统 + PowerShell
  - pip install cryptography

用法：
  python sbkeys.py
"""

import struct
import uuid
import os
import sys
import subprocess
import tempfile

# EFI Signature List 中的签名类型 GUID
EFI_CERT_X509_GUID = uuid.UUID("a5c059a1-94e4-4aa7-87b5-ab155c2bf072")
EFI_CERT_SHA256_GUID = uuid.UUID("c1c41626-504c-4092-aca9-41f936934328")

SECURE_BOOT_VARS = ["PK", "KEK", "db", "dbx"]


def check_secure_boot():
    """检查 Secure Boot 是否启用"""
    result = subprocess.run(
        ["powershell", "-Command", "Confirm-SecureBootUEFI"],
        capture_output=True, text=True
    )
    state = result.stdout.strip()
    if state == "True":
        print("[+] Secure Boot 已启用\n")
        return True
    elif state == "False":
        print("[-] Secure Boot 未启用\n")
        return True
    else:
        print(f"[!] 无法确定 Secure Boot 状态: {result.stderr.strip()}")
        return False


def export_secure_boot_vars(tmp_dir):
    """通过 PowerShell 导出安全启动变量到临时文件"""
    results = {}
    for name in SECURE_BOOT_VARS:
        ps_cmd = (
            f'$var = Get-SecureBootUEFI -Name {name}; '
            f'[System.IO.File]::WriteAllBytes("{tmp_dir}\\secureboot_{name}.bin", $var.Bytes); '
            f'Write-Output $var.Bytes.Length'
        )
        result = subprocess.run(
            ["powershell", "-Command", ps_cmd],
            capture_output=True, text=True
        )
        out = result.stdout.strip()
        err = result.stderr.strip()
        if out and not err:
            size = int(out)
            results[name] = size
            print(f"  导出 {name}: {size} bytes")
        else:
            print(f"  导出 {name} 失败: {err}")
    print()
    return results

def parse_efi_signature_list(data):
    """解析 EFI Signature List 二进制数据"""
    offset = 0
    entries = []
    while offset < len(data):
        if offset + 28 > len(data):
            break

        sig_type = uuid.UUID(bytes_le=data[offset:offset + 16])
        list_size = struct.unpack_from('<I', data, offset + 16)[0]
        header_size = struct.unpack_from('<I', data, offset + 20)[0]
        sig_size = struct.unpack_from('<I', data, offset + 24)[0]

        sig_data_offset = offset + 28 + header_size

        if sig_type == EFI_CERT_X509_GUID:
            while sig_data_offset + sig_size <= offset + list_size:
                owner = uuid.UUID(bytes_le=data[sig_data_offset:sig_data_offset + 16])
                cert_data = data[sig_data_offset + 16:sig_data_offset + sig_size]
                entries.append(("X509", owner, cert_data))
                sig_data_offset += sig_size
        elif sig_type == EFI_CERT_SHA256_GUID:
            count = 0
            while sig_data_offset + sig_size <= offset + list_size:
                count += 1
                sig_data_offset += sig_size
            entries.append(("SHA256_HASHES", None, count))
        else:
            entries.append(("UNKNOWN", sig_type, list_size))

        offset += list_size

    return entries


def format_cert_info(der_data):
    """解析 X.509 DER 证书并返回格式化信息"""
    try:
        from cryptography import x509
        from cryptography.hazmat.primitives import hashes

        cert = x509.load_der_x509_certificate(der_data)
        fingerprint = cert.fingerprint(hashes.SHA256()).hex(':')

        return {
            "Subject": cert.subject.rfc4514_string(),
            "Issuer": cert.issuer.rfc4514_string(),
            "序列号": hex(cert.serial_number),
            "有效期从": str(cert.not_valid_before_utc),
            "有效期至": str(cert.not_valid_after_utc),
            "SHA256指纹": fingerprint,
        }
    except ImportError:
        return None

def display_var(name, data):
    """显示单个安全启动变量的解析结果"""
    print("=" * 70)
    print(f"  {name} ({len(data)} bytes)")
    print("=" * 70)

    entries = parse_efi_signature_list(data)
    for i, entry in enumerate(entries):
        if entry[0] == "X509":
            print(f"\n  [{i + 1}] X.509 证书")
            print(f"      Owner GUID : {entry[1]}")
            info = format_cert_info(entry[2])
            if info:
                for k, v in info.items():
                    print(f"      {k:12s}: {v}")
            else:
                print(f"      (大小: {len(entry[2])} bytes, 请安装 cryptography 库查看详情)")
        elif entry[0] == "SHA256_HASHES":
            print(f"\n  [{i + 1}] SHA-256 吊销哈希")
            print(f"      数量: {entry[2]} 条")
        else:
            print(f"\n  [{i + 1}] 未知类型 GUID: {entry[1]}")
            print(f"      大小: {entry[2]} bytes")

    print()


def main():
    # 确保在 Windows 上运行
    if sys.platform != "win32":
        print("[!] 此脚本仅支持 Windows 系统")
        sys.exit(1)

    print("=" * 70)
    print("  Secure Boot 证书查看工具")
    print("=" * 70)
    print()

    # 1. 检查 Secure Boot 状态
    if not check_secure_boot():
        sys.exit(1)

    # 2. 导出变量到临时目录
    tmp_dir = tempfile.gettempdir()
    print("[*] 正在导出安全启动变量...")
    exported = export_secure_boot_vars(tmp_dir)

    if not exported:
        print("[!] 未能导出任何安全启动变量，请以管理员权限运行")
        sys.exit(1)

    # 3. 解析并展示每个变量
    for name in SECURE_BOOT_VARS:
        filepath = os.path.join(tmp_dir, f"secureboot_{name}.bin")
        if os.path.exists(filepath):
            data = open(filepath, "rb").read()
            display_var(name, data)
        else:
            print(f"[!] {name}: 未导出或不存在\n")

    # 4. 清理临时文件
    for name in SECURE_BOOT_VARS:
        filepath = os.path.join(tmp_dir, f"secureboot_{name}.bin")
        if os.path.exists(filepath):
            os.remove(filepath)

    print("[*] 完成")


if __name__ == "__main__":
    main()
