# assets/

Drop your app icon source image here as `icon.png`.

For best results, use a **square PNG, at least 1024x1024px**, with a
transparent background if you want rounded corners/etc. to show through
(macOS applies its own rounded-square mask on top).

`deploy.sh` (in the project root) will automatically resize this into all
the sizes macOS expects and package it as `AppIcon.icns` right here in this
folder, which the build then picks up automatically.
