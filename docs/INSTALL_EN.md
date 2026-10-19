# Installation — English

[Português](INSTALACAO_PT.md) · [Home](../README.en.md)

## Correct download

Under **Releases**, download `ScummVM_RGDSPlus_v1.0.0.zip`. This ZIP contains only the `APPS` directory, ready for the card. Complete sources are a separate asset: `ScummVM_RGDSPlus_v1.0.0_Sources.zip`.

## TF2 / Games card

1. Turn off the console and insert TF2 into your computer.
2. Extract the installation ZIP.
3. Open the extracted `APPS` directory and copy its contents into `Roms/APPS` on TF2.
4. Merge directories and replace supplied files. Keep the existing application directory when updating.
5. Safely eject the card, insert it into the console, then open APPS → ScummVM.

Check these paths on the card:

- `Roms/APPS/ScummVM.sh`
- `Roms/APPS/Imgs/ScummVM.png`
- `Roms/APPS/ScummVM_RGDSPlus/scummvm`

## First use

Use **Idioma / Language** at startup to change PT/EN. Your preference is saved. Choose **Lower screen only** or **Mirror both screens**. If no calibration exists, a nine-point assistant runs. Touch the centre of each target and lift the stylus between targets. A rejected calibration does not replace the previous one.

In the GUI, use **Add Game** and select the directory containing your game data. Games are not included. The ScummVM GUI has its own language selector in Global Options.

## Updates, saves and diagnostics

Keep `config`, `saves` and any games placed in `games`. Game directories outside the application can also be added through the GUI. The log is `ScummVM_RGDSPlus/ScummVM_Arranque.txt` and is overwritten on launch. Save it before launching again when asking for help.

If calibration fails, keep an already saved calibration when touch is accurate; to investigate rejection, save the failed session log. Do not delete your user data to update the app.

If audio is quiet, increase physical levels gradually and also check the game's internal volume settings. Low levels are deliberately gentle.

On exit, the launcher restores the previous backlight and touch settings. Confirmed compatibility is RG DS Plus with Stock OS V1.0 (20260915).
