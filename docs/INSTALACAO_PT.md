# Instalação — Português

[English](INSTALL_EN.md) · [Início](../README.md)

## Download correto

Em **Releases**, descarrega `ScummVM_RGDSPlus_v1.0.0.zip`. Dentro desse ZIP existe apenas a pasta `APPS`, pronta para o cartão. As fontes são um download separado: `ScummVM_RGDSPlus_v1.0.0_Sources.zip`.

## Cartão TF2 / Games

1. Desliga a consola e coloca o TF2 no computador.
2. Extrai o ZIP de instalação.
3. Abre a pasta `APPS` extraída e copia o seu conteúdo para `Roms/APPS` do TF2.
4. Aceita unir as pastas e substituir os ficheiros. Conserva a pasta existente da aplicação numa atualização.
5. Ejeta o cartão, coloca-o na consola e abre APPS → ScummVM.

Confirma estes caminhos no cartão:

- `Roms/APPS/ScummVM.sh`
- `Roms/APPS/Imgs/ScummVM.png`
- `Roms/APPS/ScummVM_RGDSPlus/scummvm`

## Primeira utilização

No arranque podes mudar PT/EN através de **Idioma / Language**. A preferência fica guardada. Escolhe **Só ecrã inferior** ou **Duplicar nos dois ecrãs**. Sem calibração guardada, o assistente recolhe nove pontos. Toca no centro de cada alvo e levanta a stylus entre pontos. Uma calibração rejeitada não substitui a anterior.

No GUI, usa **Add Game / Adicionar jogo** e seleciona a pasta com os dados do teu jogo. Os jogos não vêm incluídos. O GUI ScummVM tem a sua própria seleção de idioma nas Opções gerais.

## Atualizações, saves e diagnóstico

Preserva `config`, `saves` e quaisquer jogos colocados em `games`. Jogos externos à pasta da aplicação também podem ser adicionados no GUI. O log é `ScummVM_RGDSPlus/ScummVM_Arranque.txt`: é substituído ao abrir a app. Guarda-o antes de repetir o arranque se precisares de pedir ajuda.

Se a calibração falhar, usa a que já ficou guardada quando o toque está preciso; para investigar rejeições, guarda o log da sessão que falhou. Não apagues os teus dados para instalar uma atualização.

Se o som estiver baixo, testa os níveis físicos progressivamente e verifica também os volumes internos do jogo. A curva é deliberadamente suave nos primeiros níveis.

Ao sair, o launcher repõe a iluminação e o controlo de toque anteriores. A compatibilidade confirmada é a RG DS Plus com Stock OS V1.0 (20260915).
