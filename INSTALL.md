# Installation

For running the app from a pre-built `.dmg`. If you're building from source instead, see `SETUP.md`.

## macOS

1. Open `EmailClusteringFrontend.dmg`.
2. Drag **EmailClusteringFrontend.app** into the **Applications** folder shortcut shown in the window.
3. Eject the mounted disk image (right-click it on the Desktop or in Finder's sidebar → Eject).
4. **First launch:** right-click the app in Applications → **Open** → click **Open** on the confirmation dialog.

   Do this instead of double-clicking the first time. The app is signed for local use but not notarized through Apple's paid developer program, so Gatekeeper flags it as being from an unidentified developer on the very first launch. Right-click → Open shows a dialog with an explicit "Open" button and macOS remembers your choice — every launch after that works normally with a regular double-click.

   If double-clicking gives a blocked/damaged-looking dialog with no Open option, go to **System Settings → Privacy & Security**, scroll down, and click **Open Anyway** next to the message about this app.

### Uninstalling

Drag `EmailClusteringFrontend.app` from Applications to the Trash. No other files are installed elsewhere on the system.

## Requirements

None beyond a reasonably recent macOS — the `.app` bundles its own copy of Qt, so you do **not** need Qt, Homebrew, or any other dependency installed to run it.
