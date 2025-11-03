/*
 * Digital Signature Tool using Windows CNG API
 * Compile: cl /O2 signtool.c /link advapi32.lib bcrypt.lib
 * Or: gcc signtool.c -o signtool.exe -lbcrypt -lncrypt
 */

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <bcrypt.h>
#include <ncrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ncrypt.lib")

#define MAX_PATH_LEN 260
#define BUFFER_SIZE 8192

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

// Algorithm mapping
typedef struct {
    const char* name;
    const wchar_t* algorithm;
    DWORD keySize;
} AlgorithmInfo;

AlgorithmInfo algorithms[] = {
    {"rsa2048", BCRYPT_RSA_ALGORITHM, 2048},
    {"rsa4096", BCRYPT_RSA_ALGORITHM, 4096},
    {"ecdsa256", BCRYPT_ECDSA_P256_ALGORITHM, 256},
    {"ecdsa384", BCRYPT_ECDSA_P384_ALGORITHM, 384},
    {NULL, NULL, 0}
};

// Function prototypes
int cmd_keygen(const char* algorithm);
int cmd_sign(const char* algorithm, const char* filename);
int cmd_verify(const char* algorithm, const char* filename);
void print_usage(const char* progname);
int save_to_file(const char* filename, BYTE* data, DWORD size);
int load_from_file(const char* filename, BYTE** data, DWORD* size);
void print_hex(BYTE* data, DWORD size);
AlgorithmInfo* get_algorithm(const char* name);

// Main function
int main(int argc, char* argv[]) {
    const char* command = NULL;
    const char* algorithm = "rsa2048";  // Default algorithm
    const char* filename = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            algorithm = argv[++i];
        } else if (command == NULL) {
            command = argv[i];
        } else if (filename == NULL) {
            filename = argv[i];
        }
    }
    
    if (command == NULL) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Verify algorithm
    if (get_algorithm(algorithm) == NULL) {
        fprintf(stderr, "Error: Unknown algorithm '%s'\n", algorithm);
        fprintf(stderr, "Available algorithms: rsa2048, rsa4096, ecdsa256, ecdsa384\n");
        return 1;
    }
    
    // Execute command
    if (strcmp(command, "keygen") == 0) {
        return cmd_keygen(algorithm);
    } else if (strcmp(command, "sign") == 0) {
        if (filename == NULL) {
            fprintf(stderr, "Error: sign command requires a filename\n");
            return 1;
        }
        return cmd_sign(algorithm, filename);
    } else if (strcmp(command, "verify") == 0) {
        if (filename == NULL) {
            fprintf(stderr, "Error: verify command requires a filename\n");
            return 1;
        }
        return cmd_verify(algorithm, filename);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
}

void print_usage(const char* progname) {
    printf("Digital Signature Tool\n\n");
    printf("Usage:\n");
    printf("  %s keygen [-a algorithm]\n", progname);
    printf("  %s sign [-a algorithm] <file>\n", progname);
    printf("  %s verify [-a algorithm] <file>\n", progname);
    printf("\n");
    printf("Algorithms:\n");
    printf("  rsa2048   - RSA 2048-bit (default)\n");
    printf("  rsa4096   - RSA 4096-bit\n");
    printf("  ecdsa256  - ECDSA P-256\n");
    printf("  ecdsa384  - ECDSA P-384\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s keygen\n", progname);
    printf("  %s keygen -a ecdsa256\n", progname);
    printf("  %s sign document.txt\n", progname);
    printf("  %s verify document.txt\n", progname);
}

AlgorithmInfo* get_algorithm(const char* name) {
    for (int i = 0; algorithms[i].name != NULL; i++) {
        if (strcmp(algorithms[i].name, name) == 0) {
            return &algorithms[i];
        }
    }
    return NULL;
}

int cmd_keygen(const char* algorithm) {
    BCRYPT_ALG_HANDLE hAlgorithm = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    NTSTATUS status;
    DWORD cbBlob = 0;
    BYTE* pbPrivateBlob = NULL;
    BYTE* pbPublicBlob = NULL;
    char privFilename[MAX_PATH_LEN];
    char pubFilename[MAX_PATH_LEN];
    int result = 1;
    
    AlgorithmInfo* algInfo = get_algorithm(algorithm);
    if (algInfo == NULL) {
        fprintf(stderr, "Error: Invalid algorithm\n");
        return 1;
    }
    
    printf("Generating %s key pair...\n", algorithm);
    
    // Open algorithm provider
    status = BCryptOpenAlgorithmProvider(&hAlgorithm, algInfo->algorithm, NULL, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptOpenAlgorithmProvider failed (0x%x)\n", status);
        goto cleanup;
    }
    
    // Generate key pair
    status = BCryptGenerateKeyPair(hAlgorithm, &hKey, algInfo->keySize, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptGenerateKeyPair failed (0x%x)\n", status);
        goto cleanup;
    }
    
    status = BCryptFinalizeKeyPair(hKey, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptFinalizeKeyPair failed (0x%x)\n", status);
        goto cleanup;
    }
    
    // Export private key
    status = BCryptExportKey(hKey, NULL, BCRYPT_PRIVATE_KEY_BLOB, NULL, 0, &cbBlob, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptExportKey (size) failed (0x%x)\n", status);
        goto cleanup;
    }
    
    pbPrivateBlob = (BYTE*)malloc(cbBlob);
    status = BCryptExportKey(hKey, NULL, BCRYPT_PRIVATE_KEY_BLOB, pbPrivateBlob, cbBlob, &cbBlob, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptExportKey (private) failed (0x%x)\n", status);
        goto cleanup;
    }
    
    // Save private key
    snprintf(privFilename, sizeof(privFilename), "id_%s", algorithm);
    if (!save_to_file(privFilename, pbPrivateBlob, cbBlob)) {
        fprintf(stderr, "Error: Failed to save private key\n");
        goto cleanup;
    }
    printf("Private key saved to: %s\n", privFilename);
    
    // Export public key
    status = BCryptExportKey(hKey, NULL, BCRYPT_PUBLIC_KEY_BLOB, NULL, 0, &cbBlob, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptExportKey (size) failed (0x%x)\n", status);
        goto cleanup;
    }
    
    pbPublicBlob = (BYTE*)malloc(cbBlob);
    status = BCryptExportKey(hKey, NULL, BCRYPT_PUBLIC_KEY_BLOB, pbPublicBlob, cbBlob, &cbBlob, 0);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptExportKey (public) failed (0x%x)\n", status);
        goto cleanup;
    }
    
    // Save public key
    snprintf(pubFilename, sizeof(pubFilename), "id_%s.pub", algorithm);
    if (!save_to_file(pubFilename, pbPublicBlob, cbBlob)) {
        fprintf(stderr, "Error: Failed to save public key\n");
        goto cleanup;
    }
    printf("Public key saved to: %s\n", pubFilename);
    
    printf("Key generation successful!\n");
    result = 0;
    
cleanup:
    if (pbPrivateBlob) free(pbPrivateBlob);
    if (pbPublicBlob) free(pbPublicBlob);
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlgorithm) BCryptCloseAlgorithmProvider(hAlgorithm, 0);
    
    return result;
}

int cmd_sign(const char* algorithm, const char* filename) {
    AlgorithmInfo* algo = get_algorithm(algorithm);
    if (algo == NULL) {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        return 1;
    }
    
    printf("Signing file: %s\n", filename);
    
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    BCRYPT_ALG_HANDLE hHashAlg = NULL;
    NTSTATUS status;
    BYTE* privateKeyBlob = NULL;
    DWORD privateKeySize = 0;
    BYTE* fileData = NULL;
    DWORD fileSize = 0;
    BYTE hash[64] = {0};  // SHA-256 produces 32 bytes, SHA-384 produces 48 bytes
    DWORD hashSize = 32;  // Default to SHA-256
    BYTE* signature = NULL;
    DWORD signatureSize = 0;
    char privKeyFile[MAX_PATH_LEN];
    char sigFile[MAX_PATH_LEN];
    int ret = 1;
    
    // Determine hash algorithm and size
    const wchar_t* hashAlgorithm = BCRYPT_SHA256_ALGORITHM;
    if (wcsstr(algo->algorithm, L"384") != NULL) {
        hashAlgorithm = BCRYPT_SHA384_ALGORITHM;
        hashSize = 48;
    }
    
    // Load private key
    snprintf(privKeyFile, sizeof(privKeyFile), "id_%s", algorithm);
    if (load_from_file(privKeyFile, &privateKeyBlob, &privateKeySize) == 0) {
        fprintf(stderr, "Error: Failed to load private key from %s\n", privKeyFile);
        return 1;
    }
    
    // Load file to sign
    if (load_from_file(filename, &fileData, &fileSize) == 0) {
        fprintf(stderr, "Error: Failed to load file %s\n", filename);
        goto cleanup;
    }
    
    // Open algorithm provider
    status = BCryptOpenAlgorithmProvider(&hAlg, algo->algorithm, NULL, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptOpenAlgorithmProvider failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Import private key
    status = BCryptImportKeyPair(hAlg, NULL, BCRYPT_PRIVATE_KEY_BLOB, &hKey,
                                privateKeyBlob, privateKeySize, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptImportKeyPair failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Open hash algorithm provider
    status = BCryptOpenAlgorithmProvider(&hHashAlg, hashAlgorithm, NULL, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptOpenAlgorithmProvider for hash failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Hash the file data directly
    status = BCryptHash(hHashAlg, NULL, 0, fileData, fileSize, hash, hashSize);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptHash failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    printf("File hash: ");
    print_hex(hash, hashSize);
    printf("\n");
    
    // Get signature size
    status = BCryptSignHash(hKey, NULL, hash, hashSize, NULL, 0, &signatureSize, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptSignHash (size query) failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Allocate signature buffer
    signature = (BYTE*)malloc(signatureSize);
    if (signature == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        goto cleanup;
    }
    
    // Sign the hash
    status = BCryptSignHash(hKey, NULL, hash, hashSize, signature, signatureSize, &signatureSize, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptSignHash failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    printf("Signature (%u bytes): ", signatureSize);
    print_hex(signature, signatureSize);
    printf("\n");
    
    // Save signature
    snprintf(sigFile, sizeof(sigFile), "%s.sig", filename);
    if (!save_to_file(sigFile, signature, signatureSize)) {
        fprintf(stderr, "Error: Failed to save signature\n");
        goto cleanup;
    }
    
    printf("Signature saved to: %s\n", sigFile);
    ret = 0;
    
cleanup:
    if (signature) free(signature);
    if (fileData) free(fileData);
    if (privateKeyBlob) free(privateKeyBlob);
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    if (hHashAlg) BCryptCloseAlgorithmProvider(hHashAlg, 0);
    
    return ret;
}

int cmd_verify(const char* algorithm, const char* filename) {
    AlgorithmInfo* algo = get_algorithm(algorithm);
    if (algo == NULL) {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        return 1;
    }
    
    printf("Verifying file: %s\n", filename);
    
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    BCRYPT_ALG_HANDLE hHashAlg = NULL;
    NTSTATUS status;
    BYTE* publicKeyBlob = NULL;
    DWORD publicKeySize = 0;
    BYTE* fileData = NULL;
    DWORD fileSize = 0;
    BYTE hash[64] = {0};
    DWORD hashSize = 32;
    BYTE* signature = NULL;
    DWORD signatureSize = 0;
    char pubKeyFile[MAX_PATH_LEN];
    char sigFile[MAX_PATH_LEN];
    int ret = 1;
    
    // Determine hash algorithm and size
    const wchar_t* hashAlgorithm = BCRYPT_SHA256_ALGORITHM;
    if (wcsstr(algo->algorithm, L"384") != NULL) {
        hashAlgorithm = BCRYPT_SHA384_ALGORITHM;
        hashSize = 48;
    }
    
    // Load public key
    snprintf(pubKeyFile, sizeof(pubKeyFile), "id_%s.pub", algorithm);
    if (load_from_file(pubKeyFile, &publicKeyBlob, &publicKeySize) == 0) {
        fprintf(stderr, "Error: Failed to load public key from %s\n", pubKeyFile);
        return 1;
    }
    
    // Load file to verify
    if (load_from_file(filename, &fileData, &fileSize) == 0) {
        fprintf(stderr, "Error: Failed to load file %s\n", filename);
        goto cleanup;
    }
    
    // Load signature
    snprintf(sigFile, sizeof(sigFile), "%s.sig", filename);
    if (load_from_file(sigFile, &signature, &signatureSize) == 0) {
        fprintf(stderr, "Error: Failed to load signature from %s\n", sigFile);
        goto cleanup;
    }
    
    // Open algorithm provider
    status = BCryptOpenAlgorithmProvider(&hAlg, algo->algorithm, NULL, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptOpenAlgorithmProvider failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Import public key
    status = BCryptImportKeyPair(hAlg, NULL, BCRYPT_PUBLIC_KEY_BLOB, &hKey,
                                publicKeyBlob, publicKeySize, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptImportKeyPair failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Open hash algorithm provider
    status = BCryptOpenAlgorithmProvider(&hHashAlg, hashAlgorithm, NULL, 0);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptOpenAlgorithmProvider for hash failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    // Hash the file data
    status = BCryptHash(hHashAlg, NULL, 0, fileData, fileSize, hash, hashSize);
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptHash failed (0x%08x)\n", status);
        goto cleanup;
    }
    
    printf("File hash: ");
    print_hex(hash, hashSize);
    printf("\n");
    
    // Verify signature
    status = BCryptVerifySignature(hKey, NULL, hash, hashSize, signature, signatureSize, 0);
    if (status == STATUS_SUCCESS) {
        printf("✓ Signature verification SUCCESSFUL\n");
        ret = 0;
    } else if (status == STATUS_INVALID_SIGNATURE) {
        printf("✗ Signature verification FAILED\n");
    } else {
        fprintf(stderr, "Error: BCryptVerifySignature failed (0x%08x)\n", status);
    }
    
cleanup:
    if (signature) free(signature);
    if (fileData) free(fileData);
    if (publicKeyBlob) free(publicKeyBlob);
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    if (hHashAlg) BCryptCloseAlgorithmProvider(hHashAlg, 0);
    
    return ret;
}

int save_to_file(const char* filename, BYTE* data, DWORD size) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    
    size_t written = fwrite(data, 1, size, fp);
    fclose(fp);
    
    return written == size;
}

int load_from_file(const char* filename, BYTE** data, DWORD* size) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    *data = (BYTE*)malloc(filesize);
    if (!*data) {
        fclose(fp);
        return 0;
    }
    
    size_t read = fread(*data, 1, filesize, fp);
    fclose(fp);
    
    *size = (DWORD)read;
    return read == filesize;
}

void print_hex(BYTE* data, DWORD size) {
    for (DWORD i = 0; i < size; i++) {
        printf("%02x", data[i]);
        if ((i + 1) % 32 == 0) {
            printf("\n");
        } else if ((i + 1) % 4 == 0) {
            printf(" ");
        }
    }
    if (size % 32 != 0) {
        printf("\n");
    }
}
