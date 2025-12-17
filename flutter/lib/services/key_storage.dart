import 'dart:typed_data';
import 'package:shared_preferences/shared_preferences.dart';

/// Service for storing and retrieving BTHome encryption keys
class KeyStorage {
  static const _keysPrefix = 'bthome_key_';

  /// Get encryption key for a device by MAC address
  static Future<Uint8List?> getKey(String macAddress) async {
    final prefs = await SharedPreferences.getInstance();
    final keyHex = prefs.getString('$_keysPrefix${_normalizeMac(macAddress)}');
    if (keyHex == null) return null;
    return _hexToBytes(keyHex);
  }

  /// Store encryption key for a device
  static Future<void> setKey(String macAddress, Uint8List key) async {
    if (key.length != 16) {
      throw ArgumentError('Key must be 16 bytes');
    }
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(
      '$_keysPrefix${_normalizeMac(macAddress)}',
      _bytesToHex(key),
    );
  }

  /// Store encryption key from hex string
  static Future<void> setKeyFromHex(String macAddress, String keyHex) async {
    final key = _hexToBytes(keyHex);
    if (key == null || key.length != 16) {
      throw ArgumentError('Key must be 32 hex characters (16 bytes)');
    }
    await setKey(macAddress, key);
  }

  /// Remove encryption key for a device
  static Future<void> removeKey(String macAddress) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.remove('$_keysPrefix${_normalizeMac(macAddress)}');
  }

  /// Get all stored device MAC addresses with keys
  static Future<List<String>> getStoredDevices() async {
    final prefs = await SharedPreferences.getInstance();
    final keys = prefs.getKeys();
    return keys
        .where((k) => k.startsWith(_keysPrefix))
        .map((k) => k.substring(_keysPrefix.length))
        .toList();
  }

  /// Normalize MAC address format
  static String _normalizeMac(String mac) {
    return mac.toUpperCase().replaceAll(':', '').replaceAll('-', '');
  }

  /// Convert hex string to bytes
  static Uint8List? _hexToBytes(String hex) {
    final cleanHex = hex.replaceAll(' ', '').replaceAll(':', '');
    if (cleanHex.length % 2 != 0) return null;

    try {
      final bytes = Uint8List(cleanHex.length ~/ 2);
      for (var i = 0; i < bytes.length; i++) {
        bytes[i] = int.parse(cleanHex.substring(i * 2, i * 2 + 2), radix: 16);
      }
      return bytes;
    } catch (e) {
      return null;
    }
  }

  /// Convert bytes to hex string
  static String _bytesToHex(Uint8List bytes) {
    return bytes.map((b) => b.toRadixString(16).padLeft(2, '0')).join();
  }
}
