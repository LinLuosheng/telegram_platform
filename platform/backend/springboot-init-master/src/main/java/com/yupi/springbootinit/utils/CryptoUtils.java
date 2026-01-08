package com.yupi.springbootinit.utils;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class CryptoUtils {

    private static final String PI_DIGITS = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679";

    public static String encode(String text, String hostname, String timestamp) {
        try {
            byte[] key = generateKey(hostname, timestamp);
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
            cipher.init(Cipher.ENCRYPT_MODE, secretKey);
            byte[] encryptedBytes = cipher.doFinal(text.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static String decode(String text, String hostname, String timestamp) {
        try {
            byte[] key = generateKey(hostname, timestamp);
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
            cipher.init(Cipher.DECRYPT_MODE, secretKey);
            byte[] decodedBytes = Base64.getDecoder().decode(text);
            byte[] decryptedBytes = cipher.doFinal(decodedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private static byte[] generateKey(String hostname, String timestamp) {
        String raw = hostname + timestamp;
        if (raw.length() > 32) {
            raw = raw.substring(0, 32);
        } else if (raw.length() < 32) {
            raw = raw + PI_DIGITS.substring(0, 32 - raw.length());
        }
        return raw.getBytes(StandardCharsets.UTF_8);
    }
}
