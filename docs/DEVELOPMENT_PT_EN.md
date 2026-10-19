# Desenvolvimento / Development

## PT — organização e empacotamento

O repositório contém documentação, ícone, scripts, patches e código do port. A pasta APPS do repositório é um modelo de empacotamento: os motores e recursos compilados são distribuídos no ZIP de instalação das Releases. O arquivo upstream original de 223 MB fica no ZIP de fontes das Releases, para manter o repositório pequeno.

A versão pública desta adaptação é 1.0.0; o ScummVM é 2026.4.0. Os scripts de empacotamento não compilam nem descarregam executáveis: usam os ficheiros já validados fornecidos pelo responsável do projeto.

Para voltar a gerar os três assets a partir dos dois ZIPs publicados, com Python 3:

```sh
python tools/package_release.py --runtime /caminho/ScummVM_RGDSPlus_v1.0.0.zip --sources /caminho/ScummVM_RGDSPlus_v1.0.0_Sources.zip --output dist
python tools/validate_release.py dist
```

Também se aceita como `--runtime` o pacote anterior que contém APPS e Sources juntos; nesse caso `--sources` é dispensável. O ZIP instalável gerado contém **apenas APPS** e segue a lista explícita `packaging/runtime-files.txt`. Configurações, saves, jogos pessoais e logs não são incluídos. A pasta dist recebe apenas o instalável, o ZIP de fontes e SHA256SUMS.txt.

### Fontes e compilação

O ZIP de fontes inclui `Sources/scummvm-2026.4.0.tar.xz`, as alterações RG DS+, configurações usadas e os auxiliares. Para modificar o core: extrair a origem oficial, aplicar `Sources/rgds-2026-fix1.patch` e depois `Sources/volume-curve/volume-curve.patch`; as versões finais dos ficheiros do port também estão em `Sources/port`. Consultar os ficheiros de configuração e proveniência preservados em Sources para o cross-build ARM64 e as dependências.

`Sources/screens/BUILD.txt` descreve os auxiliares de ecrã e o menu PT/EN. `Sources/volume-curve/BUILD.txt` descreve a curva e a reprodução da alteração do executável validado; o script aceita apenas o hash original indicado. Não é necessário nem aconselhado correr os scripts de desenvolvimento na consola. O pacote de instalação já contém os executáveis.

Recompilar exige uma toolchain ARM64 e dependências de desenvolvimento compatíveis. Este repositório não promete um build completo automático do firmware/toolchain nem resultados byte a byte idênticos através de uma compilação integral. A documentação de volume inclui o procedimento reproduzível da alteração localizada usada no pacote distribuído.

## EN — layout and packaging

The repository contains documentation, icon, scripts, patches and port source. Its APPS directory is a packaging template: compiled engines and resources are distributed through the installation ZIP in Releases. The original 223 MB upstream archive is supplied in the separate source release asset to keep Git history small.

The adaptation release is 1.0.0; the included ScummVM version is 2026.4.0. Packaging scripts neither compile nor download executables: they consume the validated runtime supplied by the maintainer.

With Python 3, regenerate all three assets from the published ZIPs:

```sh
python tools/package_release.py --runtime /path/ScummVM_RGDSPlus_v1.0.0.zip --sources /path/ScummVM_RGDSPlus_v1.0.0_Sources.zip --output dist
python tools/validate_release.py dist
```

The older combined APPS/Sources package can also be used as `--runtime`; then `--sources` is optional. The generated installation ZIP contains **only APPS**, following `packaging/runtime-files.txt`. Configuration, saves, personal game data and logs are excluded. Output is limited to the installer ZIP, complete source ZIP and SHA256SUMS.txt.

### Sources and builds

The source ZIP includes `Sources/scummvm-2026.4.0.tar.xz`, RG DS+ changes, build configuration and helpers. To modify the core, extract upstream, apply `Sources/rgds-2026-fix1.patch` followed by `Sources/volume-curve/volume-curve.patch`. Final port files are also in `Sources/port`. Consult the preserved configuration/provenance files for the ARM64 cross-build and dependencies.

`Sources/screens/BUILD.txt` describes display helpers and the PT/EN menu. `Sources/volume-curve/BUILD.txt` documents the audio curve and reproduction of the validated executable change; its patch script accepts only the documented original hash. Development scripts are not required on the console: the installation ZIP already contains executables.

A complete rebuild requires an ARM64 toolchain and matching development dependencies. This repository does not promise a turnkey firmware/toolchain build or byte-identical results from a complete recompilation. The volume documentation does provide the reproducible localised change used in the distributed executable.
