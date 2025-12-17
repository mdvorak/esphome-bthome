# BTHome Scanner Flutter App

BLE scanner for BTHome v2 devices with AES-CCM decryption support.

## Commands

```bash
make help           # Show all commands
make deps           # Install dependencies
make analyze        # Check for errors
make icon           # Generate icons from icon.png
make build          # Build APK (auto-bumps version)
make release        # Build + GitHub release
make release-ios    # Build iOS + TestFlight
```

## Structure

```
lib/
├── main.dart                    # App entry, device list UI
├── models/bthome_device.dart    # BTHome parser, AES-CCM decryption
└── services/
    ├── ble_scanner.dart         # BLE scanning, key management
    └── key_storage.dart         # Persistent key storage
```

## Build Setup

- **Signing**: Create `android/key.properties` with keystore info
- **Icons**: Place `icon.png` (512x512+) in root, run `make icon`
- **Fastlane**: `cd android && bundle install` or `cd ios && bundle install`

## BTHome Protocol

- Service UUID: `0xFCD2` (in service data, not advertised services)
- Device info: `0x40` unencrypted, `0x41` encrypted
- Encryption: AES-128-CCM, 4-byte MIC, 13-byte nonce

## App IDs

- Android: `dev.dz0ny.bthome`
- iOS: `dev.dz0ny.bthome`
