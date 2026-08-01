#ifndef _XCRYPT_H
#define _XCRYPT_H

// Console crypto service entry points (3DES CBC + HMAC) the dashboard links
// against for the online key blob.

#ifdef __cplusplus
extern "C" {
#endif

#define XC_SERVICE_DES3_CIPHER      1
#define XC_SERVICE_DES3_KEYSIZE     24
#define XC_SERVICE_DES3_TABLESIZE   384
#define XC_SERVICE_DES_BLOCKLEN     8
#define XC_SERVICE_DIGEST_SIZE      20
#define XC_SERVICE_ENCRYPT          1
#define XC_SERVICE_DECRYPT          0

void XcHMAC(
    PUCHAR pbKey, ULONG dwKeyLength,
    PUCHAR pbInput, ULONG dwInputLength,
    PUCHAR pbInput2, ULONG dwInputLength2,
    PUCHAR pbDigest);

void XcDESKeyParity(PUCHAR pbKey, ULONG dwKeyLength);

void XcKeyTable(ULONG dwCipher, PUCHAR pbKeyTable, PUCHAR pbKey);

void XcBlockCryptCBC(
    ULONG dwCipher, ULONG dwInputLength,
    PUCHAR pbOutput, PUCHAR pbInput,
    PUCHAR pbKeyTable, ULONG dwOp, PUCHAR pbFeedback);

#ifdef __cplusplus
}
#endif

#endif // _XCRYPT_H
