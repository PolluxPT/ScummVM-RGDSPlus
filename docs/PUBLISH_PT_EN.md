# Publicar ScummVM no GitHub — PT / EN

## Português — passo a passo, sem programar

### 1. Extrair o pacote de preparação

Extrai `ScummVM_GitHub_PT_EN.zip` no computador. Encontrarás:

| Pasta/ficheiro | Utilização |
| --- | --- |
| `REPOSITORIO` | Conteúdo para colocar no separador Code do GitHub |
| `RELEASE` | Três ficheiros já prontos para anexar à release |
| `PUBLICAR_PT_EN.md` | Este guia |

### 2. Criar o repositório

1. Abre https://github.com/new e entra na tua conta.
2. Nome sugerido: **ScummVM-RGDSPlus**.
3. Descrição sugerida: **Standalone ScummVM for Anbernic RG DS Plus Stock OS — PT/EN, stylus and dual-screen modes.**
4. Escolhe **Public**.
5. Como o pacote já inclui README, licença e gitignore, cria o repositório vazio, sem os gerar automaticamente.
6. Clica **Create repository**.

### 3. Carregar o código e documentação

1. No repositório vazio, clica **uploading an existing file**. Se já houver ficheiros, usa **Add file → Upload files**.
2. Abre **REPOSITORIO** no computador.
3. Arrasta **o conteúdo dessa pasta**, incluindo APPS, Sources, docs, tools, packaging e os ficheiros da raiz. Inclui `.gitignore` e `.gitattributes`.
4. Confirma que `README.md` e `LICENSE` aparecem diretamente na raiz do repositório, e o ícone em `APPS/Imgs/ScummVM.png`.
5. Mensagem de commit: **Initial ScummVM RG DS+ release**. Clica **Commit changes**.

Não carregues o ZIP de preparação, nem a pasta RELEASE, para o separador Code. O repositório fornecido tem menos de 100 ficheiros e todos ficam abaixo do limite de 25 MiB do upload pelo navegador.

### 4. Criar a release

1. Abre **Releases → Create a new release**.
2. Em **Choose a tag**, escreve **v1.0.0** e escolhe criar essa tag ao publicar; usa a branch **main**.
3. Título: **ScummVM for RG DS+ v1.0.0 — PT/EN**.
4. Abre `REPOSITORIO/RELEASE_NOTES.md`, copia todo o texto PT/EN e cola na descrição da release.
5. Em **Attach binaries**, arrasta estes **três ficheiros de RELEASE, mantendo os ZIPs fechados**:
   - `ScummVM_RGDSPlus_v1.0.0.zip`
   - `ScummVM_RGDSPlus_v1.0.0_Sources.zip`
   - `SHA256SUMS.txt`
6. Confirma os três anexos e clica **Publish release**.

O GitHub acrescenta automaticamente Source code (zip) e Source code (tar.gz). É normal: esses ficheiros são cópias do repositório. O download a indicar aos utilizadores é **ScummVM_RGDSPlus_v1.0.0.zip**.

### 5. Confirmar a publicação e instalar

Abre a página pública sem sessão, descarrega o ZIP instalável da release e verifica que dentro existe apenas APPS. Em APPS devem existir ScummVM.sh, Imgs/ScummVM.png e ScummVM_RGDSPlus.

Para a consola: extrai esse ZIP e copia **o conteúdo de APPS** para `Roms/APPS` do TF2. Numa atualização, une/substitui ficheiros e conserva a pasta atual para manter calibração, preferências, jogos e saves. O ZIP de fontes fica no computador.

### Atualizações futuras

Edita os ficheiros do repositório e a VERSION quando preparares uma nova versão. Os scripts em tools podem voltar a gerar a release a partir de um runtime validado; as instruções de desenvolvimento estão em `docs/DEVELOPMENT_PT_EN.md`. Para esta publicação, tudo já está gerado: não precisas de instalar Python, Git ou uma ferramenta de compilação.

## English — browser-only publication

1. Extract `ScummVM_GitHub_PT_EN.zip`. `REPOSITORIO` contains GitHub repository files; `RELEASE` contains the three release assets; this file is the publishing guide.
2. Open https://github.com/new. Suggested name: **ScummVM-RGDSPlus**. Choose **Public** and create an empty repository without generating a README, licence or gitignore; those are already supplied.
3. Click **uploading an existing file**, or **Add file → Upload files** in an existing repository. Upload the **contents of REPOSITORIO**, including `.gitignore` and `.gitattributes`. README.md and LICENSE must be at the repository root, with the icon at APPS/Imgs/ScummVM.png. Commit as **Initial ScummVM RG DS+ release**.
4. Open **Releases → Create a new release**. Create tag **v1.0.0** on **main**. Title: **ScummVM for RG DS+ v1.0.0 — PT/EN**. Paste the contents of `RELEASE_NOTES.md` into the description.
5. Under **Attach binaries**, upload the three files from RELEASE without unpacking the ZIPs: `ScummVM_RGDSPlus_v1.0.0.zip`, `ScummVM_RGDSPlus_v1.0.0_Sources.zip`, and `SHA256SUMS.txt`.
6. Click **Publish release**. GitHub's automatic Source code ZIP/tarball downloads are expected; users should install the separately attached `ScummVM_RGDSPlus_v1.0.0.zip`.
7. Check the public release while signed out. The installation ZIP must contain only APPS. Users copy the contents of APPS into TF2 `Roms/APPS`, merging updates and keeping their existing user data. Complete sources stay on the computer.

The supplied repository is below 100 files, each below 25 MiB for browser upload. Do not upload the preparation ZIP or RELEASE directory into the Code tab. Everything required for this publication is already generated; Python/Git/build tools are not needed to publish these files.

## Referências / References

- GitHub upload: https://docs.github.com/en/repositories/working-with-files/managing-files/adding-a-file-to-a-repository
- GitHub releases: https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository
- Large files / ficheiros grandes: https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github
