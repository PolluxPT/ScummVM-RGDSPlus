# Publicar a v1.0.1 no GitHub

## 1. Testar na RG DS+

Extraia `ScummVM_RGDSPlus_v1.0.1.zip` e copie o conteúdo de APPS para Roms/APPS no TF2. Una as pastas e substitua os ficheiros fornecidos, sem apagar config/saves/games.

Uma instalação com PT guardado continua em PT. Para testar uma instalação nova sem mexer nos seus dados, use uma cópia de teste da aplicação num cartão de teste. Sem config, o menu de ecrãs e a GUI devem abrir em inglês; faça a calibração se for solicitada. Teste a mudança para PT no menu inicial, saia e abra novamente para confirmar a persistência. Teste também a escolha independente de idioma em Global Options e confirme que se mantém no arranque seguinte.

Teste também o volume: coloque a consola, por exemplo, no nível 4, abra ScummVM, altere o volume com os botões físicos e saia. O sistema deve regressar ao nível 4. Repita com volume inicial 0. Confirme o controlo durante um jogo e o registo `saved initial level` / `restored initial level` em ScummVM_Arranque.txt.

## 2. Atualizar o repositório existente

1. Extraia `ScummVM_RGDSPlus_v1.0.1_GitHub_Update.zip` no computador.
2. Abra https://github.com/PolluxPT/ScummVM-RGDSPlus na branch principal.
3. Escolha **Add file → Upload files**.
4. Arraste todo o conteúdo extraído: ficheiros e pastas, sem a pasta exterior nem o próprio ZIP. Preserve os caminhos, por exemplo APPS/ScummVM.sh e Sources/screens/menu.c.
5. O ficheiro APPS/ScummVM_RGDSPlus/tools/rgds-screen-menu é o menu recompilado e também deve ser enviado.
6. Mensagem do commit: `Release v1.0.1: English defaults and restored system volume`.
7. Confirme o commit na branch principal. O ZIP inclui apenas ficheiros novos ou alterados; os restantes mantêm-se.

## 3. Criar a release

1. Abra https://github.com/PolluxPT/ScummVM-RGDSPlus/releases/new.
2. Em **Choose a tag**, escreva **v1.0.1** e escolha criar a tag ao publicar.
3. Em **Target**, escolha a branch principal com o commit acabado de criar.
4. Título: **ScummVM RG DS+ v1.0.1 — English by default**.
5. Copie o conteúdo de **RELEASE_NOTES.md** para a descrição. Após o teste físico, pode atualizar a indicação de validação com o resultado real.
6. Anexe estes três ficheiros, sem extrair os ZIPs:
   - ScummVM_RGDSPlus_v1.0.1.zip
   - ScummVM_RGDSPlus_v1.0.1_Sources.zip
   - SHA256SUMS.txt
7. Depois de validar na consola, não marque como pre-release. Marque **Set as the latest release**, se disponível.
8. Clique em **Publish release**.

Não anexe GitHub_Update como pacote de instalação. Os ZIPs automáticos Source code do GitHub são normais; o instalável é o ficheiro anexado manualmente.

## 4. Confirmar

Abra https://github.com/PolluxPT/ScummVM-RGDSPlus/releases/latest e confirme a v1.0.1. Descarregue o instalável e confirme que abre sem erros.

No PowerShell, compare os valores seguintes com SHA256SUMS.txt:

```powershell
Get-FileHash .\ScummVM_RGDSPlus_v1.0.1.zip -Algorithm SHA256
Get-FileHash .\ScummVM_RGDSPlus_v1.0.1_Sources.zip -Algorithm SHA256
```

## English summary

Test the installer on the console. Extract GitHub_Update and upload its contents to the existing repository, preserving relative paths, including the rebuilt startup-menu binary. Commit the update, create tag v1.0.1 on that commit, paste RELEASE_NOTES.md and attach the installation ZIP, complete source ZIP and SHA256SUMS.txt. Publish as latest after hardware validation.
