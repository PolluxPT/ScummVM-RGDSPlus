# ScummVM RG DS+ v1.0.1 — English by default

## English

This update makes first use easier for international users:

- The startup display menu defaults to **English (EN)** when no valid preference is saved.
- New ScummVM configurations use **English** for the GUI.
- Existing startup-menu and ScummVM GUI language preferences are preserved.
- Legacy GUI migration no longer forces Portuguese.
- The pre-launch Stock system volume is saved and restored on exit, including handled interruption/error exits.
- Physical volume buttons retain the existing curve: **0.35 × (level / 10)²**, for levels 0–10.
- The startup PT/EN button still saves your choice. ScummVM's GUI language is selected separately in Global Options.

**Install/update:** download `ScummVM_RGDSPlus_v1.0.1.zip`, extract it and copy the contents of **APPS** into **Roms/APPS** on TF2. Merge directories and replace supplied files; keep **config**, **saves** and **games**.

**Complete sources:** `ScummVM_RGDSPlus_v1.0.1_Sources.zip` (keep on your computer). Checksums: `SHA256SUMS.txt`. GitHub's automatic Source code archives are not the installation package.

Port version: **1.0.1**. Included ScummVM: **2026.4.0**. The startup menu was rebuilt for ARM64; the core, 113 plugins and display/audio libraries are unchanged from v1.0.0. Language, migration and volume checks pass on the host; this update still requires physical-console validation.

Target: Anbernic RG DS Plus, Linux Stock OS V1.0 (20260915). Games are not included. Game language depends on the game data.

## Português

- Menu inicial de ecrãs e interface ScummVM em **inglês na primeira utilização**.
- Preferências de idioma existentes preservadas, incluindo durante a migração de configurações antigas.
- Volume inicial do sistema memorizado antes de abrir e reposto ao sair.
- Botões físicos de volume com a curva **0,35 × (nível / 10)²**, nos níveis 0–10.
- O botão PT/EN controla o menu inicial; o idioma da interface ScummVM é escolhido separadamente nas Opções gerais.

Para instalar, copia o conteúdo de **APPS** do ZIP instalável para **Roms/APPS** no TF2. Une as pastas e substitui os ficheiros, preservando **config**, **saves** e **games**. Uma instalação que já tenha PT guardado continua em PT.

O menu foi recompilado para ARM64. O core ScummVM 2026.4.0, os 113 plugins e as bibliotecas mantêm-se. Testes de idioma, migração e volume aprovados no computador; falta validar esta atualização na consola física.
