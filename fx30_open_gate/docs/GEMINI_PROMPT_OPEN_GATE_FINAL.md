# PROMPT PARA O GEMINI — OPEN GATE FX30 (PLANO FINAL, 23/11)

Você é um engenheiro reverso de firmware Sony BIONZ XR. Objetivo: liberar o open gate 3:2
(5952×3968 → 3840×2560, ou 6192×4128 → 3840×2560) na Sony FX30 (ILME-FX30, FW 6.01),
via injeção volátil em RAM (Jiritsu USB). NADA de flash. Cold boot (bateria 10s) restaura 100%.

REGRAS: (1) nunca escrever em flash; (2) só RAM; (3) cada helper tem markers de verificação
em /tmp_wbshd/; (4) não repita os 5 testes já feitos (listados abaixo).

=====================================================================
1. FATOS VERIFICADOS (não são hipótese — já conferidos contra fx30_cp_dump.bin)
=====================================================================

CP = ARM32 Thumb-2 (NÃO ARM64). CP RAM = file + 0x108000 no cpapp-b.bin.
Dump completo: /Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/fx30_dump_helper/fx30_cp_dump.bin
  (cobre CP 0x00C0000–0x2100000; CP = file_off + 0x108000)

TABELA MESTRE ImagerDrvParam: array 31 entries × 352 B (0x160) em CP 0x20d93a4.
  Objeto: C_ImagerCtrl 0x13bd364, ImagerDrvParam 0x13bd9fc (count=31, array=0x20d93a4).
  Campos por entry: +0x024 mode id, +0x028/+0x034 line timing, +0x054 LARGURA,
  +0x066 ALTURA, +0x0f6 cropV, +0x142 S3 (readout line periods), +0x12a VMAX.
  Entry 2  (0x20d9664, mode 0x0820): 5952×3348, crop 406, S3 1704
  Entry 6  (0x20d9be4, mode 0x08c0 4K24p): idem
  Entry 7  (0x20d9d44, mode 0x08d0 4K30p): idem
  Entry 16 (0x20db1e4, mode 0x05c0, DCI 17:9): 5952×3140, tipo 5
  Entry 25 (0x20db604, mode 0xa000, STILL 3:2): 6192×4128, crop 0, S3 0, tipo 2
  Entry 26 (0x20db764, mode 0xa100): 6192×4128

TABELA DE COLUNAS (0x01039938, u32 por mode index):
  mode2=1, mode3=2, mode4=1, mode5=1, mode6=1, mode7=1, mode8=3, mode9=4, mode10=4,
  mode11=4, mode12=8, mode13=8, mode14=8, mode15=8, mode16=9, mode17=9, mode18=5,
  mode19=6, mode25=11, mode26=12

MATRIZ DE 154 NÓS (0x01038d1c, stride 0x14 = 20 B por nó):
  cada nó = [u32 dest_ptr (aponta pro buffer P2S 0x01123xxx)] [14 bytes c0..c13] [2 bytes].
  Nó 0: ptr 0x01123dea, c1=0x0f (vídeo streaming contínuo), c11=0x01 (still single-shot).
  (0x01038d2b = c11 do nó 0.)

BUFFER P2S (registradores físicos do sensor, região 0x01123dea..0x01124208):
  N3 VBLANK lo 0x01123dfe=0xe7; N4 hi 0x01123e02=0x01;
  N5 active lo 0x01123e06=0x90; N6 hi 0x01123e0a=0x06 (0x0690=1680 = 3348/2+6);
  N7 0x01123e1e=0x09; N8 0x01123e26=0x22; N10 0x01123e4a=0x29 (reg sensor 0x19).

SGC TABLE (records de 3000 B = 0xBB8, front-end do ISP, pares de 16-bit):
  Mode 2 L 0xc79440 / R 0xc79ff8 — O RECORD QUE O MODO 4K 0x08c0 CONSOME (w11=3348). USAR ESTE.
  Mode 3 L 0xc7abb0 / R 0xc7b768 — ERRADO (w11=3156, crop/S&Q). NÃO USAR.
  Campos: +0x006 bounding h, +0x00e guardband, +0x016 active h, +0x77c scaler in-h,
  +0x784 luma v-step, +0x786 chroma v-step, +0x788 phase (u32 na prática), +0x95e crop V.
  Mode 2 valores stock: w2=3320, w3=3382, w6=3304, w7=3356, w10=3292, w11=3348;
  +0x784=5625, +0x786=5623, +0x788=3963 (L) / 31591 (R), +0x95e=406.
  O record tem 1500 palavras de 16-bit, com sub-blocos de geometria em 4-tuplas [x,y,w,h];
  os campos de LARGURA (w0/w2/w4/... e os x de cada box) NUNCA foram mapeados/traduzidos.

SCALARS (S1/S3/S7/S9):
  S1 altura ativa modo 6: 0xe02888 = 3348 (0xe027f0 = a do modo 2, também 3348)
  S3 readout: 0xe03298 = 1704 (= (3348+60)/2). Tabela de rows de 14 B em 0xe0328c:
    row0 0xe0328c (24p), row1 0xe0329a (25p, S3=1609), row2 0xe032a8... — PATCH TODAS as rows
    ou travar a câmera em 24p NTSC.
  S9 reg 0x19: 0xe03290 = 41 (0x29) — bate com o Nó 10 da matriz.
  S7: 0xe030f0 = 502. S2 0xc9b308=502 e S4 0xc365d4=502 são IDs sequenciais, NÃO offsets (remover).

OUTSZ: 0x155d446 = 0x0870 → 0x0a00 (2160→2560). Tabela de pares [u32 id_modo, u32 w|h
  empacotado] em 0x155d410.. (ex.: 0x155d444 = bytes 00 0f 70 08 = w 0x0f00, h 0x0870).
DMMWB: 0x2080860 e 0x2080874 = 0x1788700 → 0x01bf0000 (29.29 MB).
LCD fx30lcd_v2 (7 sites): 0x1e2d3c (3→0), 0xde791c, 0xde7920, 0xde7994, 0xde7998,
  0xde7a0c, 0xde7a10 (página 0x00de7000).
ENCODER fx30og_v2 (level 19, helper separado): av-cam 19 sites + libmpr 4 sites.
CARD UNLOCK V60 (fx30menu_v1 + fx30rate_v1): 100% OK.

=====================================================================
2. OS 5 TESTES JÁ FEITOS (NÃO REPETIR — resultados confirmados)
=====================================================================
1. coluna 1 + S1/S3 + Mode 2 front-end (3968)  → vídeo PRETO (foto OK), UI/REC ok, tally não acende
2. coluna 1 + TG físico (0x01123xxx)          → PRETO em foto E vídeo (regressão)
3. coluna 1 + tabela mestre ImagerDrvParam     → PRETO
4. coluna 11 (S0=11) + nó0 0x0f + entry25      → imagem VIVA (rosa/branco saturado) + listras,
   gravou 8 s (REC0019) mas contador travou (encoder faltando)
5. coluna 11 + nó0 + Mode 2 escalado SÓ ALTURA → ruído amarelo, tally presa

CONCLUSÃO DOS 5: a coluna 1 NÃO entrega mais que 3348 linhas. A coluna 11 entrega o frame
completo (6192×4128) com o nó0 em streaming, mas o pipeline decodifica com a largura 5952
→ stride errado → lixo. A LARGURA NUNCA FOI TRADUZIDA EM NENHUM TESTE.

=====================================================================
3. A REFERÊNCIA A7 IV (repo clonado em /tmp/a7iv_og — LEIA os arquivos)
=====================================================================
/tmp/a7iv_og/windows/tools/modding/designs/ogsq_v3.json  ← 13 sites (11 scalars + 2 records)
  S0 0xfb4044: 6→5 (coluna da matriz; seleciona o scan 3:2 de VÍDEO 5016×3344 que JÁ EXISTE)
  S1 0xd95c42: 3942→4672; S3 0xd965fc: 2011→2382; S9 0xd965f4: 0x33→0x3b
  S2 0xc98476 / S4 0xc3dc92 / S7 0xd96428: 366→0; S5 0xd95c3e: 2→8; S6 0xd95c46: 2→8
  S8 0xd9642c: 0x193→0x13; S10 0xd960b6: 0x51→0x61
  Records: 3000 B cada, 180 campos; "first-stage window of 4688 rows squeezed by Lrc
  into the stock half picture"; summary: readout [7032,4688], crop [7008,4672],
  front_end [4336,2438], size_source [3840,2160], vertical sampling 27/32 (stock 1/1),
  scaler_vertical_step 7849 (stock 6623).
  FATOS-CHAVE: (a) o A7 IV NÃO muda a largura (o vídeo dele já lê 7008 = largura total);
  (b) ele usa um SCAN 3:2 DE VÍDEO que existe no sensor (modo 5) — não o still;
  (c) o record aplica um SQUEEZE VERTICAL (27/32) pra caber no pipeline 16:9;
  (d) o VMAX NÃO foi tocado (0→0 no record).
/tmp/a7iv_og/windows/tools/modding/designs/dmmwb4688_v1.json
/tmp/a7iv_og/windows/tools/modding/designs/outsz2560_v1.json  (0x143c7bc: 000f7008→000f000a)
/tmp/a7iv_og/windows/tools/modding/designs/lcdfill_v2.json, lcdcycle_v6.json
/tmp/a7iv_og/windows/tools/camera_tests/CT6_memory_read_check/open_gate_apply.py ← a ORDEM dos níveis

=====================================================================
4. A7S III (PASTA LOCAL — ANALISAR PRIMEIRO, é de graça)
=====================================================================
/Users/nauck/Documents/sony_research/a7siii-open-gate-test/
  Leia PORT_NOTES.md e windows/tools/camera_tests/CT6_memory_read_check/helper_s3/.
  Extraia: o S0 do A7S III troca pra qual coluna/modo? Ele também usa squeeze vertical?
  Compare com o A7 IV. (O A7S III tem o mesmo sensor do FX3/A7S III — 12MP, 4240×2832,
  largura total no vídeo — então deve ser o mesmo padrão do A7 IV.)

=====================================================================
5. AS 4 TAREFAS (nesta ordem)
=====================================================================
T1 (offline, sem câmera): diffar as 14 colunas da matriz de 154 nós (0x01038d1c, stride
  0x14; a coluna C de cada nó = o byte em nó+4+C). Imprimir, para cada coluna 0..13, a lista
  de nós onde ela difere da coluna 1. Procurar uma coluna que difere SÓ nos nós verticais
  (altura/VBLANK/prescaler) e MANTÉM os nós horizontais (5952) e o clock iguais à coluna 1.
  Essa coluna é o "modo 5" da FX30 → candidata ao S0 (0x01039950 = essa coluna).
  Também listar o que as colunas 2,3,4,5,6,8 mudam em relação à 1.

T2 (offline): mapear TODOS os campos de LARGURA do record Mode 2 (0xc79440/0xc79ff8):
  cada 4-tupla [x,y,w,h] — listar os offsets das palavras de largura/x e os valores stock
  (5952 → 6192 = +240 em cada; x → x, sem mudar). Produzir a lista completa dos campos
  horizontais (hoje só temos os verticais).

T3 (na câmera — o teste decisivo): a TRADUÇÃO DE DOIS EIXOS da coluna 11:
  - S0: 0x01039950 = 11 (u32)
  - nó0 c11: 0x01038d2b = 0x0f (streaming)
  - ImagerDrvParam entry 2 (0x20d9664) e entry 6 (0x20d9be4):
      +0x054 largura 5952→6192, +0x066 altura 3348→4128, +0x0f6 crop 406→0,
      +0x142 S3 1704→2100 (= (4128+72)/2), +0x028/+0x034 line timing 340→1040 (valor do still)
  - Mode 2 L/R: aplicar a lista do T2 (larguras 5952→6192) + os verticais:
      bounding 3382→4162, guard 3356→4136, active 3348→4128, in-h 3356→4128,
      luma 5625→6912 (= round(4128/2442×4096)), chroma 5623→6911, crop 406→0,
      phase mantém stock (3963 / 31591)
  - S1: 0xe02888 = 4128; S3: TODAS as rows 0xe03298/0xe032a6/... = 2100
  - outsz 0x155d446 = 0x0a00; dmmwb 0x2080860/74 = 0x01bf0000
  - NÃO aplicar LCD (isolar); NÃO aplicar TG físico.
  - Verificação: markers; depois Filme→REC→contador? e o XML do clipe.
  Se der imagem LIMPA: o open gate está feito. Se der lixo de novo: o stride é outro (T4).

T4 (fallback): o SQUEEZE VERTICAL do A7 IV. No A7 IV o record muda "vertical sampling
  27/32" (campos +6/+7) e o scaler_vertical_step 6623→7849, pra ler 4688 linhas e comprimir
  no pipeline 16:9. Localizar os campos equivalentes no Mode 2 da FX30 (as razões de
  amostragem vertical — provavelmente perto do +0x784/+0x788 ou dos campos +6/+7) e aplicar
  a mesma ideia com a coluna 1 lendo 4128 e o front-end comprimindo pra 2442. Isso permitiria
  manter a coluna 1 (clock de vídeo puro) e só mexer na leitura.

T5 (se T3 der imagem mas contador travado): rodar o ENCODER fx30og_v2 (level 19, helper
  /Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/camera_tests/CT6_memory_read_check/helper/libJit_level19_fx30og_v2.so)
  SEM cold boot entre as etapas. E o fix pós: ffmpeg -i clip.mp4 -map 0:v:0 -map 0:a?
  -c copy -movflags +faststart out.mp4 (o A7 IV precisa disso — o clipe sai com aspect
  errado e o ffmpeg reescreve o metadata).

=====================================================================
6. PROTOCOLO DE BUILD/INJEÇÃO (macOS)
=====================================================================
BUILD:
  cd /Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/fx30_dump_helper
  SEN=/Users/nauck/Documents/sony_research/fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha15_unpacked/lib/libsencore.so
  python3 -m ziglang cc -target aarch64-linux-gnu -std=gnu11 -O1 -fPIC -shared -nostdlib \
    -ffreestanding -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables \
    -Wl,-soname,libJit.so -Wl,--no-undefined -Wl,-z,now -g0 -s -Wl,-z,max-page-size=4096 \
    <nome>.c "$SEN" -o <nome>.so
INJEÇÃO (câmera em Mass Storage 054C:0E0E):
  sudo -k /bin/bash /tmp/run_dump_root.sh
  (o send_dump.py aponta LIB = BASE / "<nome>.so" e lê os markers em EMS)
REGRAS DO HELPER: verificar CADA valor stock antes de escrever (aceitar old OU new), ler de
volta, markers distintos, unlink dos markers no início, cleanup do lib no fim (FILE_DELETE2).
DMA: um único cam_dmm_alloc(4096), mmap /dev/mem, sen_dma_transfer(0, CP_page, phys) lê,
sen_dma_transfer(1, phys, CP_page) escreve. Páginas: coletar por (addr & ~0xfff).

=====================================================================
7. ENTREGA
=====================================================================
- T1: tabela "coluna X difere da coluna 1 nos nós: [...]" (as 14 colunas)
- T2: lista completa dos offsets de LARGURA/x do record Mode 2
- T3: <nome>.c + <nome>.so + o resultado na câmera (imagem? contador? XML?)
- T4: a localização dos campos de squeeze (se T3 falhar)
- T5: o script end-to-end (coluna 11 dois eixos + encoder + LCD) + o ffmpeg fix
